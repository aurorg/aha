 /*
1) 主要构造函数
void error(const char *err_string,int line);//错误处理函数，打印错误所在行数和错误信息

void Demonstrate_attribute(struct stat buf,char *name);//获得文件属性并打印

void Demonstrate_single(char *name);//输出文件名，命令没有-l选择，输入文件名要保持上下对齐

void Demonstrate(int flag,char *pathname);//根据命令行参数和文件路径名来显示目标文件

void Demonstrate_dir(int flag_parameter,char *path);//为显示某个目录下的文件做准备


2）函数流程
  （1）获取该目录下文件的总数和最长文件名
  （2）若获取该目录下所有文件的文件名，存放于变量filenames中
  （3）使用冒泡法对文件名按字母顺序存储于filenames中
  （4）调用Demonstrate()函数来显示每个文件的信息


3)程序中主要的结构体：
参数struct stat *buf 是一个保存文件状态信息的结构体
A>  struct stat{
      dev st_ dev;//文件设备号
      ino_t st_ino;//文件的i-node
      mode_t st_mode;//文件类型和存储权限
      nlink_t st_nlike;//连接到该文件的硬连接数目
      uid_t st_uid;//文件所有者的用户id
      git_t st_gid;//文件所有者的组id
      dev_t st_rdev;//若此文件为设备文件，则为其设备编号
      off_t st_size;//文件大小
      blksize_t st_blksize;//文件系统的I/O缓冲区大小
      blkcnt_t st_blocks;//占用文件区块的个数
      time_t st_atime;//文件最近一次被访问的时间
      time_t st_ctime;//文件最近一次被更改的时间
  }

  对于st_mode包含的文件类型信息，POSIX标准定义了一系列的宏：
  S_ISLNK(st_mode)//判断是否为符号链接
  S_ISREG(st_mode)//判断是否为一般文件
  S_ISDIR(st_mode)//判断是否为目录文件
  S_ISCHR(st_mode)//判断是否为字符设备文件
  S_IBLK(st_mode)//判断是否为块设备文件
  S_ISFIFO(st_mode)//判断是否为先进先出FIFO
  S_ISFOCK(st_mode)//判断是否为socket

B>:struct passwd *psd;//从该结构体中获取文件所有者的用户名

c>:struct group *grp;//从该结构体重获取文件所有者所属组的组名

因为刚刚才有了思路，所以都大概列了出来
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>

#define PARAMETER_NONE 0 //无参数
#define PARAMETER_A//-a显示所有文件
#define PARAMETER_L//-l一行显示一个详细的文件信息
#define MAXROWLEN 80//一行显示最多的字符数

int g_lenve_len =MAXROWLEN;//一行剩余长度，用于输出对齐
int g_maxlen;//存放某目录下最长文件名的长度
void error(const char *err_string,int line);//错误处理函数，打印错误行数和错误信息
void Demonstrate_attribute(struct stat buf,char *name);//获取文件属性并打印
void Demonstrate_single(char *name);//输出文件名，命令没有-l选项，则输出文件名时要保持上下文对齐
void Demonstrate(int flag,char * pathname);//根据命令行参数和文件路径名显示目标文件
void Demonstrate_dir(int flag_parameter,char * path);//为显示某个目录下的文件做准备

int main(int argc,char** argv)
{
int i,j,k,num;
char path[PATH_MAX+1];//文件路径名
char parameter[32];//保存命令行参数，目标文件名和目录名不在此列
int flag_parameter=PARAMETER_NOTE;
struct stat buf;

/*
首先对命令行参数进行解析，获得命令行参数中的‘-’后面的内容{ls -l-a,ls -la}
用两层循环类来解析参数，
外层循环对argv[]数组中的元素一次进行内层循环的解析，
内层循环对以‘-’为首的字符串进行选项提取，
把每个选项存放于parameter[]数组里面，用num记录‘-’的数量，
命令行参数中的总选项数目用j计数
*/
j=0;
num=0;
for(i=1;i<argc;i++)
{
  if(argv[i][0]=='-')
  {
    for(k=1;k<strlen(argv[i]);k++)
    {
      parameter[j]=argv[i][k];//获取-后面的参数保存到数组parameter中
      j++;
    }
    num++;//保存‘-’的个数

  }
}
/*检查上面提取的选项是否合法，用或运算记录参数，以备后用，最后为选项数组的末尾元素赋‘/0’*/
for(i=0;i<j;i++)
{
  if(parameter[i]=='a')
  {
    flag_parameter=PARAMETER_A;

    continue;
  }
  else if(parameter[i]=='l')
  {
    flag_parameter=PARAMETER_L;

    continue;
  }
  else
  {
    printf("ls:invalid option -%c\n",parameter[i]);
    exit(1);
  }
}
parameter[j]='\0';

/*
因为上面的num是记录了‘-’的总数，所以num+1=argc的话，
说明用户输入的命令行参数不包括目录或者文件名，而是只是显示当前目录下的文件，
所以要自动将path赋值为当前目录
因为是字符串，所以必须末尾加‘/0’，之后进入Demonstrate_dir函数
*/

if((num+1)==argc)
{
  strcpy(path,"./");  //./是当前目录
  path[2]='\0';
  Demonstrate_dir(flag_parameter,path);

  return 0;
}
/*
如果命令行参数包括目录或者文件名，需要检查参数中的目录或者文件是否存在
可以利用stat族函数来获取文件的属性，完成上面的检查
stat族函数通常有两个参数：文件路径/文件描述符，struct stat *buf 类型的结构体
如果操作成功啦，那么buf就会保存文件的属性
如果可以的话，利用宏S_ISDIR(buf.st_mode),判断此文件是否是目录文件
如果是目录文件，就可进入Demonstrate_dir函数，要不然的话进去Demonstrate函数
Demonstrate_dir函数是获取path目录下所有文件的完整路径名，再使每个文件执行Demonstrate函数
所以的话，如果参数中指定的文件名，就可以跳过Demonstrate_dir函数，直接进入Demonstrate函数
*/

i=1;
do{
  //如果不是目标文件名或者目录，就解析下一个命令行参数
if(argv[i][0]=='-')
{
  i++;
  continue;
}
else
{
  strcpy(path,argv[i]);  //如果目标文件或者目录不存在，报错之后退出程序
if(stat(path,&buf)==-1)
{
  error("stat",__LINE__);
}
if (S_ISDIR(buf.st_mode)) //argv[i]是一个目录
{
if(path[strlen(argv[i]-1)]!='/') //如果目录的最后一个字符不是/就添加/
{
  path[strlen(argv[i])='/'];
  path[strlen(argv[i])+1]='\0';
}
else
{
  path[strlen(argv[i])]='\0';
}
Demonstrate_dir(flag_parameter,path);
i++;
}
else  //argv[i]是一个文件
{
  Demonstrate(flag_parameter,path);
  i++;
}
}
}
while(i<argc);

return 0;

}

void error(const char *err_string,int line)//错误处理函数，打印错误所在行的行数和错误信息
{
  fprintf(stderr,"line:%d ",line);
  perror(err_string);
  exit(1);
}

void Demonstrate_attribute(struct stat buf,char * name) //获取文件属性并且打印
{
 char buf_time[32];//存放时间的
 struct passwd *psd;//从这个结构体中获取文件所有者的用户名
 struct group *grp; //从这个结构体中获取文件所有者所属组的组名
//获取并且打印文件类型
//st_mode  文件内容和存储权限
if(S_ISLNK(BUF.st_mode))  //判断是否为符号链接
{
  printf('1');
}
else if (S_ISREG(buf.st_mode)) //判断是否是目录
{
  printf("-");
}
else if (S_ISCHR(buf.st_mode))  //判断是否是字符设备文件
{
  printf("c");
}
else if (S_ISBLK(buf.st_mode))//判断是否是块设备文件
{
  printf("b");
}
else if (S_ISFIFO(buf.st_mode)) //判断是否是先进先出的FIFO
{
  printf("f");
}
else if (S_ISSOCK(buf.st_mode))//判断是否是socket 
{
  printf("s");
}

//获取并且打印文件所有者的权限
if (buf.st_mode & S_IRUSR)
{
  printf("i");
}
else
{
 printf("-");
}
if (buf.st_mode & S_IMUSR)
{
  printf("w");
}
else 
{
  printf("-");
}
if(buf.st_mode & S_IXUSR)
{
  printf("x");
}
else
{
  printf("-");
}

//获取并且打印与文件所有者同组的用户对该文件的操作权限

if(buf.st_mode & S_IRGRP)
{
  printf("r");
}
else
{
  printf("-");
}
if (buf.st_mode & S_IWGRP)
{
  printf("w");
}
else
{
  printf("-");
}
if (buf.st_mode & S_IXGRP)
{
  printf("x");
}
else
{
  printf("-");
}

//获取并且打印其他用户的对该文件的操作权限

if (buf.st_mode & S_IROTH)
{
  printf("r");
}
else
{
  printf("-");
}
if (buf.st_mode & S_IWOTH)
{
  printf("r");
}
else
{
  printf("-");
}
if (buf.st_mode & S_IWOTH)
{
  printf("w");
}
else
{
  printf("-");
}
if (buf.st_mode & S_IXOTH)
{
  printf("x");
}
else
{
  printf("-");
}
printf(" ");

//根据uid和gid获取文件所有者的用户名与组名

psd = getpwuid(buf.st_uid);
grp = getgrgid(buf.st_gid);
printf("%4d ",buf.st_nlike); //打印文件的链接数 （该文件硬连接数目）
printf("%-9s",psd->pw_name); //打印文件拥有者
printf("%-8s",grp->gr_name); //打印文件的大小
strcpy(buf_time,ctime(&buf.st_size)); 
buf_time[strlen(buf_time)-1]='\0'; //去掉换行符
printf(" %s",buf_time); //打印文件的时间信息
}

//在没有使用-l选项的时候，打印一个文件名，打印时上下行之间进行对齐

void Demonstrate_single(char*name)
{
  int i,len;
  if (g_lenve_len<g_maxlen)  //如果本行打印不下一个文件名，就换行
  {
    printf("\n");
    g_leave_len = MAXROWLEN;
  }
  len=strlen(name);
  len=g_maxlen -len;
  printf("%-s",name);
  for(i=0; i<len; i++)
  {
    printf(" ");
  }
  printf(" ");

g_leave_len-=(g_maxlen+2); //这两个指示空格
}

//根据命令行参数和完整路径名显示目标文件
//参数flag:命令行参数
//参数pathname:包含了文件名的路径名
void Demonstrate(int flag,char * pathname)
{
  int i,j;
  struct stat buf;
  char name[NAME_MAX+1]
  //从路径中解析出文件名
  for(i=0; j=0; i<strlen(pathname); i++)
  {
  if (pathname[i]=='/')
  {
    j=0;
    continue;
  }
  name[j++]=pathname[i];
  }
  name[j]='/0';

//用lstat而不是stat以方便解析链接文件
if (linkat(pathname,&buf)==-1) //lstat返回的是符号链接文件，文件本身的状态信息
{
  error("stat",__LINE__);
}
switch(flag)
{
  case PARAMETER_NONE; //没有-l和-a选项
  if (name[0]!='.')
  {
    Demonstrate_single(name);
  }
  break;
  case PARAMETER_A: // -a显示包括隐藏文件在内的所有文件
  Demonstrate_single(name);
  break;
  case  PARAMETER_L: //每个文件单独占一行，显示文件的详细属性信息
  if (name[0]!='.')
  {
    Demonstrate_attribute (buf,name);
    printf(" %-s\n",name);
  }
  break;
  case PARAMETER_A + PARAMETER_L; //同时有-a和-l选项的情况
  Demonstrate_attribute(buf,name);
  printf(" %-s\n",name);
  break;
  default:
  break;
}
}

//为了显示目录下的文件做准备
void Demonstrate_dir(int flag_parameter,char *path)
{
  DIR*dir;
  struct dirent *ptr;
  int count=0;
  char filenames[256] [PATH_MAX+1]; 
  // 获取该目录下文件总数和最长的文件名
  dir =opendir(path);
  if(dir==NULL)
  {
    error("opendir",__LINE__);
  }
  while((ptr=readdir(dir)) !=NULL)
{
  if (g_maxlen<strlen(ptr->d_name))
  g_maxlen=strlen(ptr->d_name);
  count++;
} 
closedir(dir);
if(count>256)
error("太多了"，__LINE__);
int i,j.len=strlen(path);

// 获取这个目录下所有的文件名
dir =opendir(path);
for(i=0; i<count; i++)
{
  ptr=readdir(dir);
  if(ptr==NULL)
  {
    error("readdir",__LINE__);
  }
  strncpy(filenames[i],path,len);//存放目录下所有文件名
  filenames[i][len]='\0';
}

//使用冒泡排序的方法对文件名进行排序，排序后文件名按照字母顺序存在filenames
for(i=0; i<count-1; i++)
for(j=0; j<count-1-i; j++)
{
  if(strcmp(filename[j],filenames[j+1])>0)
{
  strcpy(temp,filenames[j+1]);
  temp[strlen(filenames[j+1])] = '\0';
  strcpy(filenames[j+1],filename[j]);
  filenames[j+1][strlen(filenames[j])] = '\0';
  strcpy(filenames[j],temp);
  filenames[j][strlen(temp)]='\0';

}
}
for(i=0; i<count; i++)
Demonstrate(flag_parameter,filenames[i]);
closedir(dir);
//如果命令行中没有-l选项，打印一个换行符
if((flag_parameter & PARAMETER_L)==0)
printf("\n");

} 


}

