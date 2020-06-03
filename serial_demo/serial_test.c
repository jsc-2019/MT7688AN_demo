#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      //Unix 标准函数定义
#include <sys/types.h>   //
#include <sys/stat.h>
#include <fcntl.h>       //文件控制定义
#include <termios.h>     //PPSIX 终端控制定义
#include <errno.h>       //错误号定义
#include <string.h>

#include <pthread.h>
#include "serial_test.h"

#define TIMEOUT                 1
#define MIN_LEN                 1
#define DEV_NAME_LEN            31
#define MAX_UART_NUM            2

#define SERIAL_ATTR_BAUD        115200
#define SERIAL_ATTR_DATABITS    8
#define SERIAL_ATTR_STOPBITS    1
#define SERIAL_ATTR_PARITY      'N'
#define SERIAL_ATTR_FLOW_CTRL   0

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BUF_SIZE                    512

static char g_dev_serial[][DEV_NAME_LEN] = {
    "/dev/ttyS0",
    "/dev/ttyS1",
    "/dev/ttyS2"
};


static void print_usage(const char *filename)
{
    printf("Usage: %s [012]\n", filename);
    printf(" 0 --/dev/ttyS0  com0 test, used as rs232.\n");
    printf(" 0 --/dev/ttyS1  com1 test, used as rs232.\n");
    printf(" 0 --/dev/ttyS2  com2 test, used as rs232.\n");
}

/******************************************************************* 
* 名称：            uart_open 
* 功能：            打开串口并返回串口设备文件描述 
* 入口参数：        fd    :文件描述符     port :串口号(/dev/ttyS0,/dev/ttyS1,/dev/ttyS2) 
* 出口参数：        正确返回fd，错误返回为-1 
*******************************************************************/  
int uart_open(int uart_number)
{
    int fd = 0;
    
    fd = open(g_dev_serial[uart_number], O_RDWR);
    if (fd == -1)
    {
        printf("open serial device %s error\n", g_dev_serial[uart_number]);
        return -1;
    }

    //
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed !\n");
        return -1;
    }
    
    //
    if (isatty(STDIN_FILENO) == 0)
    {
        printf("standard input is not a terminal device\n");
        return -1;
    }
    printf("uart %d open success\n", uart_number);
    return fd;
}

/******************************************************************* 
* 名称：                uart_close 
* 功能：                关闭串口并返回串口设备文件描述 
* 入口参数：        fd    :文件描述符     port :串口号(ttyS0,ttyS1,ttyS2) 
* 出口参数：        void 
*******************************************************************/  
void uart_close(int fd)
{
    close(fd);
}

/******************************************************************* 
* 名称：                uart_attr_set 
* 功能：                设置串口数据位，停止位和效验位 
* 入口参数：        fd        串口文件描述符 
*                              speed     串口速度 
*                              flow_ctrl   数据流控制 
*                           databits   数据位   取值为 7 或者8 
*                           stopbits   停止位   取值为 1 或者2 
*                           parity     效验类型 取值为N,E,O,,S 
*出口参数：          正确返回为1，错误返回为0 
*******************************************************************/  
int uart_attr_set(int fd, SERIAL_ATTR_ST *serial_attr)
{
    int i = 0;
    int status;
    int speed_arr[] = {B115200, B57600, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200,  57600,  19200,  9600,  4800,  2400,  1200,  300};

    struct termios options;

    if (tcgetattr(fd, &options) != 0)
    {
        perror("Setupserial");
        return -1;
    }

    if (NULL == serial_attr)
	{
        printf("%s, invalid argument.\n", __FUNCTION__);
        return -1;  
	}

    //设置串口输入输出波特率
    for(i = 0; i< ARRAY_SIZE(speed_arr); i++)
    {
        if (serial_attr->baud == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }
    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    //设置数据流控制
    switch (serial_attr->flow_ctrl)
    {
        case 0: //不使用流控制
            options.c_cflag &= ~CRTSCTS;
            break;
        case 1: //使用硬件流控制
            options.c_cflag |= CRTSCTS;
            break;
        case 2:
            options.c_cflag |= IXON | IXOFF | IXANY;
            break;
        default:
            break;
    }

    //设置数据位
    options.c_cflag &= ~CSIZE; //屏蔽其它标志位
    switch (serial_attr->databits)
    {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
    
        default:
            fprintf(stderr, "Unsupported data size\n");
            return -1;
    }

    //设置校验位
    switch (serial_attr->parity)
    {
        case 'n':
        case 'N':   //无奇偶校验位
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':   //设置为奇校验
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
        case 'e':
        case 'E':   //设置为偶校验
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 's':
        case 'S':   //设置为空格
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return -1;
    }

    // 设置停止位  
    switch (serial_attr->stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported stop bits \n");
            return -1;
    }

    //-如果不是开发终端之类的,只是串口传输数据,而不需要串口来处理,那么使用原始模式(Raw Mode)方式来通讯     
    //修改输出模式，原始数据输出  
    options.c_oflag &= ~OPOST;  /*Output*/
    //-经典输入是以面向行设计的.在经典输入模式中输入字符会被放入一个缓冲之中,这样可以以与用户交互的方式编辑缓冲的内容,直到收到CR(carriage return)或者LF(line feed)字符.    
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的  /*Input*/选择原始输入
    //options.c_lflag &= ~(ISIG | ICANON);

    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = TIMEOUT;  /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = MIN_LEN;   /* 读取字符的最少个数为1 */

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd, TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("com set error \n");
        return -1;
    }
    return 0;
}

/******************************************************************* 
* 名称：                UART0_Init() 
* 功能：                串口初始化 
* 入口参数：        fd       :  文件描述符    
*               speed  :  串口速度 
*                              flow_ctrl  数据流控制 
*               databits   数据位   取值为 7 或者8 
*                           stopbits   停止位   取值为 1 或者2 
*                           parity     效验类型 取值为N,E,O,,S 
*                       
* 出口参数：        正确返回为0，错误返回为-1 
*******************************************************************/  
int uart_init(int fd)
{
    SERIAL_ATTR_ST serial_attr;

    memset(&serial_attr, 0, sizeof(serial_attr));
    serial_attr.baud = SERIAL_ATTR_BAUD;
    serial_attr.databits = SERIAL_ATTR_DATABITS;
    serial_attr.stopbits = SERIAL_ATTR_STOPBITS;
    serial_attr.parity = SERIAL_ATTR_PARITY;
    serial_attr.flow_ctrl = SERIAL_ATTR_FLOW_CTRL;
    //
    if (uart_attr_set(fd, &serial_attr) == 0)
    {
        return 0;
    }
    else
    {
        printf("uart init failed\n");
        return -1;
    }
}

/******************************************************************* 
* 名称：                  UART0_Recv 
* 功能：                接收串口数据 
* 入口参数：        fd                  :文件描述符     
*                              rcv_buf     :接收串口中数据存入rcv_buf缓冲区中 
*                              data_len    :一帧数据的长度 
* 出口参数：        正确返回为1，错误返回为0 
*******************************************************************/  
static void *uart_datas_read(void *arg)
{
    int ret;
    char read_buf[BUF_SIZE];
    int fd = *(int *)arg;
    int len = 0;

    while (1)
    {
        memset(read_buf, 0, sizeof(read_buf));
        ret = read(fd, read_buf, sizeof(read_buf));
        if(ret != 0)
        {
            printf("receive %d datas: %s\n", ret, read_buf);
        }
    }
    return NULL;
}

static int uart_transfer(int fd, char *write_buf, int buf_size)
{
    int ret = 0;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, uart_datas_read, (void *)&fd);
    if (ret != 0)
    {
        printf("can not create thread: %s\n", stderr);
    }
    while (1)
    {
        ret = write(fd, write_buf, buf_size);
        if (ret != buf_size)
        {
            printf("write uart failed, ret=%d\n", ret);
            break;
        }
        sleep(1);   //1s
    }
    pthread_join(pid, NULL);
    return ret;    
}

static int serial_rs232_test(int fd, char *write_buf, int buf_size)
{
    int ret = 0;

    ret = uart_init(fd);
    if (ret == 0)
    {
        ret = uart_transfer(fd, write_buf, buf_size);
    }
    return ret ;
}

static int rs232_test(int channel)
{
    int fd;
    int bufsize;
    char write_buf[BUF_SIZE];

    printf("OpenWRT MT7688AN rs232 com%d start Testing ...\n", channel);
    fd = uart_open(channel);
    if (fd < 0)
    {
        return -1;
    }
    memset(write_buf, 0, sizeof(write_buf));
    snprintf(write_buf, sizeof(write_buf), "Com%d RS232 test string!\n", channel);
    bufsize = strlen(write_buf);
    printf("write_buf: %s\n", write_buf);
    serial_rs232_test(fd, write_buf, bufsize);

    return 0;
}

int main(int argc, char *argv[])
{
    int index;

    if ((argc != 2) || ((index = (*argv[1] - '0')) < 0)||
        (index > 1))
    {
        printf("Invalid arguments!\n");
        print_usage(argv[0]);
        return -1;
    }
    if (index <=1)
    {
        if(rs232_test(index) != 0)
        {
            printf("rs232 test failed!\n");
        }
        else
        {
            printf("rs232 test success!\n");
        }  
    }
    return 0;
}