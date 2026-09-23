#include<stdio.h>
	int main(){
	int x, *p = NULL;
		printf("Enter the nunber");
		scanf("%d", &x);
	p = &x;
	printf("The entered number is: %d \n", x);
	printf("address of x: %u \n", &x);
	printf("The entered number is: %d \n", *p);
	printf("value of p: %u \n",p);
	printf("address of p: %u \n",& *p);
	return 0;
}
