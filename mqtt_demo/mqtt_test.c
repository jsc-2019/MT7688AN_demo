#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "mqtt_client.h"

#define HOST "tcp://127.0.0.1:1883"
#define CLIENTID "clientID_33883"
#define TOPIC   "test_topic"
#define PAYLOAD     "Hello World!"
#define QOS     1
#define TIMEOUT     10000L
#define USERNAME    "admin"
#define PASSWORD    "public"


int main(int argc, char **argv)
{
    mqtt_client *p_mqtt_obj = NULL;  //mqtt client 对象指针
    int ret;
    char *host = HOST;  //测试服务器
    char *topic = TOPIC;    ////主题
    char *client_id = CLIENTID; //客户端ID；
    char *username = USERNAME;  //用户名，用于验证身份。
    char *password = PASSWORD;  //密码，用于验证身份。
    int Qos = QOS_EXACTLY_ONCE;    //Quality of Service
    char *mqtt_message = "hello from MTK7688AN";

    //carete new mqtt client object
    p_mqtt_obj = mqtt_new(host, MQTT_PORT, client_id);  //创建对象，MQTT_PORT = 1883
    if (p_mqtt_obj == NULL)
    {
        printf("mqtt client create failed error:%d\n", errno);
        return -1;
    }
    else
    {
        printf("mqtt client creat success!\n");
    }
    
    //connect to server
    ret = mqtt_connect(p_mqtt_obj, username, password); //连接服务器,这里不仅仅有硬件连接还有MQTT协议连接
    if (ret != MQTT_SUCCESS)
    {
        printf("mqtt client connect failed error:%d, ret=%d\n", errno, ret);
        return -1;
    }
    else
    {
        printf("mqtt client connect success!\n");
    }

    //publish message
    while (1)
    {
        ret = mqtt_publish(p_mqtt_obj, topic, mqtt_message, Qos); //发布消息
        printf("mqtt client publish,  return code = %d\n", ret);
        usleep(1000*1000);  //
    }
    
    //服务器断开连接,并且清除为了连接而建立的系列环境
    mqtt_disconnect(p_mqtt_obj);    //disconnect
    mqtt_delete(p_mqtt_obj);    //delete mqtt client object
    return 0;
}