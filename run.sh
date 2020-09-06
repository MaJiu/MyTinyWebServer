#! /bin/bash

if [ $# -eq 1 ]
then
    ./out/$1 8000 ./root index.html
    exit 0
fi

if [ $# -ne 4 ]
then
    echo "参数不正确 格式: <服务器> <端口号> <服务器根目录> <默认页面>"
    exit 1
fi
./out/$1 $2 $3 $4
