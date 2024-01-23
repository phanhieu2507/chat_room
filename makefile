all: chatclient chatserver

chatclient: chatclient.c chatlinker.c login.c menu.c logger.c
	gcc -g -w chatclient.c  chatlinker.c login.c menu.c logger.c -o chatclient

chatserver: chatserver.c chatlinker.c login.c menu.c logger.c
	gcc -g chatserver.c  chatlinker.c login.c menu.c logger.c -o chatserver
.PHONY:clean
clean:
	rm -f chatclient chatserver activity_log.log
