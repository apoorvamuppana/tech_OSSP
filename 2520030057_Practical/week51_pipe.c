#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
int main(){
     int pid;
     pid=fork();
     if(pid<0){
        printf("failure in process creation\n");
        exit(0);
    }
    else if(pid == 0) {
      printf("i am in child process\n");
    }
    else {
        printf("I am in parent process\n");
     }
     return 0;
}
