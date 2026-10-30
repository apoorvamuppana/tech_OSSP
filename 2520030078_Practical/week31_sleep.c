#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
int main(){
int pid;
	printf("Start of Parent\n");
pid = fork(); // create child process
if(pid == 0){ // body of child process
printf("Start of child\n");
sleep(15); //child process sleep for 15 sec
exit(0);// force termination of chid without completing child process
printf("End of Child\n");
}
else{ // body of  parent process
sleep(10); // parent process sleep for sec
wait(NULL); // parent process waits for child process complete
printf("End of Parent\n");
}
	return 0;
}
/*
output

Start of parent
start of child
end of child
end of parent
*/
