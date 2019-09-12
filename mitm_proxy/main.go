package main

import (
	"os"
	"flag"
	"fmt"
	"runtime"
	"log"
	"io/ioutil"
	"strings"
	"os/signal"
	"syscall"
	"time"
	"net/http"
	"sync"
	"bytes"
	"path/filepath"
	"crypto/tls"
)

var proxy_mapper = make(map[string]string, 100)    // 代理主机映射表
var save_path string                               // 数据包保存位置
var connCnt int                                    // 连接数量统计
var readTimeout = time.Duration(500) * time.Second // 数据包超时
var lock sync.Mutex                                // 文件创建锁

func init() {
	// 初始化协程
	runtime.GOMAXPROCS(runtime.NumCPU())
}

func parser_proxy_config_file(filename string) {
	file, err := os.Open(filename) // 检查文件访问权限
	if err != nil {
		if !os.IsNotExist(err) {
			fmt.Fprintf(os.Stderr, "error reading %s: %v\n", filename, err)
			os.Exit(1)
		}
	}
	defer file.Close()

	data, err := ioutil.ReadAll(file)
	if err != nil {
		return
	}
	if len(data) < 1 {
		return
	}
	reader := strings.Split(string(data), "\n")
	for _, line := range reader {
		line = strings.TrimSpace(line)
		if len(line) > 0 {
			r := strings.SplitN(line, "=", 2)
			if len(r) == 2 {
				proxy_mapper[r[0]] = r[1]
				fmt.Printf("[+++] 检查到代理转发主机 主机:%v -> %v\n", r[0], r[1])
			}
		}
	}
}

func get_save_file() (*os.File, *os.File) { // 获取用于保存请求和响应的两个文件
	var request, response *os.File
	current := time.Now().Format("2006_01_02_15_04_05")
	for i := 0; true; i++ {
		join_name := fmt.Sprintf("%v_%d_request.txt", current, i)
		join_name = filepath.Join(save_path, join_name)
		lock.Lock()
		if _, err := os.Stat(join_name); os.IsNotExist(err) {
			request, _ = os.Create(join_name)
			response, _ = os.Create(filepath.Join(save_path, fmt.Sprintf("%v_%d_response.txt", current, i)))
			lock.Unlock()
			break
		}
		lock.Unlock()
	}
	return request, response
}

func Analyzer(w http.ResponseWriter, r *http.Request) {
	var redirect_addr string      // 跳转主机名称
	var redirect_uri string       // 跳转主机地址
	var request_method = r.Method // 设置请求方法
	var new_url string            // 新的请求
	var request_body []byte       // 请求正文
	//r.ParseForm()                 //解析参数，默认是不会解析的

	redirect_uri = r.RequestURI

	// 设置跳转主机名称
	if addr, ok := proxy_mapper[r.Host]; ok {
		redirect_addr = addr
	}
	new_url = fmt.Sprintf("%s%s", redirect_addr, redirect_uri)
	current := time.Now().Format("2006-01-02 15:04:05")
	fmt.Printf("[***] %v 客户端:%v 发起请求:%v 请求方式:%v 转发到请求:%v\n", current, r.RemoteAddr, r.Host, request_method, new_url)
	request_body, _ = ioutil.ReadAll(r.Body)
	request, err := http.NewRequest(request_method, new_url, bytes.NewReader(request_body))
	if err != nil {
		fmt.Printf("请求编译失败 报错:%v\n", err.Error())
	}
	request.Header = r.Header
	transCfg := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: true}, // 禁用验证
	}
	httpClient := &http.Client{
		Timeout:   5 * time.Minute,
		Transport: transCfg,
	}
	response, err := httpClient.Do(request)
	if err != nil {
		fmt.Printf("请求失败 报错:%v\n", err.Error())
	}

	// 写回响应头
	for key, values := range response.Header {
		for _, value := range values {
			w.Header().Add(key, value)
		}
	}
	// 写回响应状态码
	w.WriteHeader(response.StatusCode)
	resp_body, err := ioutil.ReadAll(response.Body)
	if err != nil {
		fmt.Printf("响应正文读取失败 报错:%v\n", err.Error())
	}
	w.Write(resp_body)
	request_file, response_file := get_save_file()
	// 防止出现问题不能正常关闭文件
	defer request_file.Close()
	defer response_file.Close()
	// 写请求行到文件
	request_file.Write([]byte(fmt.Sprintf("%s %s %s\r\n", request_method, redirect_uri, r.Proto)))
	// 写请求头到文件
	for key, values := range r.Header {
		request_file.Write([]byte(fmt.Sprintf("%s: ", key)))
		for _, value := range values {
			request_file.Write([]byte(fmt.Sprintf("%v;", value)))
		}
		request_file.Write([]byte(fmt.Sprintf("\r\n")))
	}
	// 解释头
	request_file.Write([]byte(fmt.Sprintf("\r\n")))
	// 写入请求正文
	request_file.Write(request_body)

	// 写入响应到文件
	response_file.Write([]byte(fmt.Sprintf("%s %d %s\r\n", response.Proto, response.StatusCode, response.Status)))
	for key, values := range response.Header {
		response_file.Write([]byte(fmt.Sprintf("%s: ", key)))
		for _, value := range values {
			response_file.Write([]byte(fmt.Sprintf("%v;", value)))
		}
		response_file.Write([]byte(fmt.Sprintf("\r\n")))
	}
	response_file.Write([]byte(fmt.Sprintf("\r\n")))
	response_file.Write(resp_body)
}

func run(port string) {
	http.HandleFunc("/", Analyzer)                                                 //设置访问的路由
	err := http.ListenAndServeTLS(":"+port, "cert/cert.cer", "cert/cert.key", nil) //设置监听的端口
	if err != nil {
		log.Fatal("监听端口发生异常: ", err)
	}
}

func main() {
	log.SetOutput(os.Stdout)

	var proxy_config_file string // 代理配置文件
	var proxy_port string        // 代理端口

	flag.StringVar(&proxy_config_file, "f", "", "代理主机映射文件") // 设置代理映射文件
	flag.StringVar(&save_path, "save", "", "数据保存路径")        // 设置保存文件路径
	flag.StringVar(&proxy_port, "port", "443", "代理端口")      // 设置代理端口
	flag.Parse()                                            // 解析参数
	if save_path == "" || proxy_config_file == "" {
		flag.PrintDefaults()
		os.Exit(1)
	}

	fmt.Printf("[***] 代理端口:%v \n", proxy_port)
	fmt.Printf("[***] 代理主机映射文件:%v \n", proxy_config_file)
	fmt.Printf("[***] 代理数据保存位置:%v \n", save_path)

	parser_proxy_config_file(proxy_config_file)
	go run(proxy_port)
	// 等待退出信号
	var sigChan = make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGHUP)
	for sig := range sigChan {
		log.Printf("caught signal %v, exit", sig)
		os.Exit(0)
	}
	// bug修复
	// /usr/local/Cellar/go/1.11/libexec/src/net/http/client.go :168 数据发送 卡 send
	// /usr/local/Cellar/go/1.11/libexec/src/net/http/server.go :2314 请求接受被自动处理(傻逼一样)

}
