# MyTinyWebServer 一个轻量级Web服务器

### 简介

​	本项目参考CSAPP第十一章 TinyWebServer 搭建了一个轻量级Web服务器，支持`GET`请求，可提供CGI动态服务  ,一共有**迭代式，多进程，多线程，线程池**四个版本

`./src` 源代码  

​	`csapp.h csappc. sbuf.h sbuf.c` 为CSAPP提供的一些包

`./root` 测试服务器挂载的根目录

`./out` 编译后的可执行文件

`./test` Python编写的客户端脚本

### 测试以及开发环境

​	Windows WSL Ubuntu 20.04

​	gcc version  9.3.0

​	压测工具 : Apache Bench `ab -c 并发数 -n 总请求次数 url`

### 使用方法

1.编译

​	`./mygcc.sh filename`

​	filename可为 mytiny 迭代式、tiny_process 多进程， tiny_thread 多线程，tiny_pool 线程池

2.运行

​	`./run.sh <filename> <port> <ROOT_DIR> <DEFAULT_PAGE>`

​	在8000端口运行迭代式服务器，根目录为./root ，默认页面为 index.html

​	`eg. run.sh mytiny 8000 ./root index.html ` 

​	省略`<port> <ROOT_DIR> <DEFAULT_PAGE>` 时，默认执行 `run.sh <filename>8000 ./root index.html `

​	省略`<ROOT_DIR> <DEFAULT_PAGE>` 时，默认执行 `run.sh <filename><port> ./root index.html `

​	省略`<DEFAULT_PAGE>` 时，默认执行 `run.sh <filename><port> <ROOT_DIR> index.html `

3.打开浏览器访问

 静态页面 `http://localhost:8000/`

 动态页面 `http://localhost:8000/cgi-bin/adder?1&1`

### 约定

1. 默认页面 index.html
2. /cgi-bin/ 下全部为CGI动态服务

### 问题

  1. 请求Url 为 http://localhost:port 时, 浏览器一般会发起两次请求(一次请求**index.html**, 

     一次请求 **favicon.ico**)，因此一般浏览器会建立两次TCP连接，而谷歌浏览器(Edge也是)会在两

     次TCP连接结束后，还会发起一次连接，但是此次连接没有数据传输，mytiny版本的服务器(迭代式)一直在等待数据到来，程序被阻塞在这里，不能服务别的浏览器的请求。
