#include <stdio.h>
int main()
{
    int a[11],i,j,t;
    for(i=0;i<=10;i++)
         a[i]=0;//初始化为0
    for(i=1;i<=5;i++)//循环读入五个数
    {
        scanf("%d",&t);//把每一个读到导变量t中
        a[t]++;
    }
    for(i=0;i<=10;i++)//以此判断
       for(j=1;j<=a[i];j++)//出现几次就打印几次
           printf("%d  ",i);
    getchar();getchar();//用来暂停程序，以便查看程序中输出的内容
    return 0;
}