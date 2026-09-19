#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
	void myhandler(int signo) {
		printf("Signal Received\n");
	if(signo == SIGSEGV) {
		printf("handling segmentation fault \n");
		printf("Signal Number=%d",signo);
	}
	if(signo == SIGINT) {
		print("handling interupts \n");
		printf("Signal Number = %d",signo);
	}
	exit(1);
    }
	int main(){
	signal(SIGSEGV,myhandler);
	signal(SIGINT,myhandler);
	int x, *p = NULL;
		printf("Enter the number");
		scanf("%d",&x);
//break;
	p = &x;
	raise(SIGINT);
	p = (int *)100;
		printf("The entered number is: %d \n",x);
		printf("address of x: %u \n",&x);
		printf("The entered number is:%d \n",*p);
		printf("value of p: %u \n",p);
		printf("address of p:%u \n",&p);
//}
//raise(SIGINT);
	return 0;
}
