# MyTinyWebServer 一个轻量级Web服务器

### 简介

​	本项目参考CSAPP第十一章 TinyWebServer 搭建了一个轻量级Web服务器，支持`GET`请求，可提供CGI动态服务  

src 源代码  

root 测试服务器所用的根目录

out 编译后的可执行文件

test Python编写的客户端脚本

### 测试以及开发环境

​	Windows WSL Ubuntu 18.04

​	gcc version 7.4.0

### 使用方法

1.编译

​	`./mygcc.sh mytiny`

2.运行

​	`./run.sh mytiny <port> <ROOT_DIR> <DEFAULT_PAGE>`

​	`eg. run.sh mytiny 8000 ./root index.html `

3.打开浏览器访问

​	`http://localhost:8000/`

  `http://localhost:8000/cgi-bin/adder?1&1`

### 约定

1. 默认页面 index.html
2. /cgi-bin/ 下全部为CGI动态服务

### 问题

  1. 请求Url 为 http://localhost:port 时, 浏览器一般会发起两次请求(一次请求**index.html**, 

     一次请求 **favicon.ico**)，因此一般浏览器会建立两次TCP连接，而谷歌浏览器(Edge也是)会在两

     次TCP连接结束后，还会发起一次连接，但是此次连接没有数据传输，服务器端一直在等待数据

     到来，程序被阻塞在这里，不能服务别的浏览器的请求。