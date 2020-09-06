# MyTinyWebServer 一个轻量级Web服务器

### 约定

1. 默认页面 index.html
2. /cgi-bin/ 下全部为CGI动态服务

### 问题

 	1. 请求Url 为 http://localhost:port 时, 浏览器一般会发起两次请求(一次请求**index.html**, 一次请求**favicon.ico**)，因此一般浏览器会建立两次TCP连接，而谷歌浏览器(Edge也是)会在两次TCP连接结束后，还会发起一次连接，但是此次连接没有数据传输，服务器端一直在等待数据到来，程序被阻塞在这里，不能服务别的浏览器的请求。