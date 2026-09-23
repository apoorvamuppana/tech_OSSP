#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
	int main(){
	mkfifo("/tmp/siri_fifo",0666);
	printf("sending message to reader\n");
	char msg[] = "hello receiver";
	int fd = open("/tmp/siri_fifo",O_WRONLY);
	write(fd, msg, strlen(msg));
	printf("write sent a message");
	close(fd);
	return 0;
}
