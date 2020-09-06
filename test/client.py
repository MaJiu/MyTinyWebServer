# 测试TinyWeb服务器

import socket


s = socket.socket()
(host, port) = input("输入: <host> <port>\n").split(" ")
s.connect((host, int(port)))
s.close()

