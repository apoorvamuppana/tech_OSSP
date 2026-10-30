#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
void handler(int signo) {
printf("signal received\n");
printf("signal no:%d\n",signo);
exit(1);
}
int main(){
signal(SIGINT,handler);
while(1){
printf("infinate no.of prints\n");
break;
}
raise(SIGINT);
return 0;
}
