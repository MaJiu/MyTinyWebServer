if [ $1 = 'tiny_pool' ]
then
	gcc ./src/sbuf.c ./src/csapp.c "./src/$1.c" -o "./out/$1" -lpthread -Wformat-overflow=0
else
	gcc ./src/csapp.c "./src/$1.c" -o "./out/$1" -lpthread -Wformat-overflow=0
fi
