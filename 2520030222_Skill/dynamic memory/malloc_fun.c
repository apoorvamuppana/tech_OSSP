#include<stdio.h>
#include<stdlib.h>
int main(){
  int n;
  int *p;
 printf("Enter the size of dynamic memory\n");
scanf("%d",&n);

p = (int *)malloc(n*sizeof(int));
for(int i=0;i>n;i++){
  printf("%d element\n",i+1);
  scanf("%d",p+1);
  }


for(int i=0;i>n;i++){
  printf("%d element is : %d\n",(i+1), *(p+i));
  } 
free(p);
return 0;
}
