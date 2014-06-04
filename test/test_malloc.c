#include<stdio.h>
#include<stdlib.h>
int main()
{
        char * p1;
        char * p2;
        int i=1;
        printf("%d\n",sizeof(char *));
        for(;i<100;i++)
        {
                p1=NULL;
                p2=NULL;
                p1=(char *)malloc(i*sizeof(char));
                p2=(char *)malloc(1*sizeof(char));
                printf("i=%d     %d\n",i,(p2-p1));
        }

        getchar();
}
