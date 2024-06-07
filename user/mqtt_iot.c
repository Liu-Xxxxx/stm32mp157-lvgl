#include "mqtt_iot.h"
#include "mqtt/MQTTClient.h" // 需要在系统中提前安装好MQTT
#include "squareLine/ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define ADDRESS "iot-06z00ifb87fntqv.mqtt.iothub.aliyuncs.com:1883"
#define CLIENTID "i6f2sHj3AkW.myDevice|securemode=2,signmethod=hmacsha256,timestamp=1717400711981|"
#define USERNAME "myDevice&i6f2sHj3AkW"
#define PASSWORD "32057d3bcf5197eb32834a1b01b465669dbef1e5e46a292921014ded560700ce"
#define QOS 0
#define TIMEOUT 10000L
#define SUB_TOPIC "/i6f2sHj3AkW/myDevice/user/get"

MQTTClient client; // 定义一个MQTT客户端client
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

void set_temp_humi_data(unsigned short value)
{
    uint8_t temp_value = (value >> 8) & 0xFF;
    uint8_t humi_value = value & 0xFF;

    lv_slider_set_value(ui_Slider1, temp_value, LV_ANIM_OFF);
    lv_slider_set_value(ui_Slider3, humi_value, LV_ANIM_OFF);

    // lv_chart_set_next_value(ui_chart1, temp, temp_value);
    // lv_chart_set_next_value(ui_chart1, humi, humi_value);
    // lv_chart_refresh(ui_chart1);
}

// 传递给MQTTClient_setCallbacks的回调函数 消息到达后，调用此回调函数
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived\n");
    printf(" topic: %s\n", topicName);
    printf(" message: %.*s\n", message->payloadlen, (char *)message->payload);

    unsigned short value = 0;
    unsigned short len = message->payloadlen;
    char *buf = (char *)message->payload;
    for (unsigned short i = 0; i < len; i++)
    {
        if (buf[i] == '\0')
            break;
        if (buf[i] <= '9' && buf[i] >= '0')
            value = value * 10 + buf[i] - '0';
    }

    set_temp_humi_data(value); // 调用我们封装的LVGL更新函数

    MQTTClient_freeMessage(&message); // 释放消息
    MQTTClient_free(topicName);       // 释放主题名
    return 1;
}

// 传递给MQTTClient_setCallbacks的回调函数 连接异常断开后调用此回调函数
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf(" cause: %s\n", cause);
}

// 封装主动断开连接服务器的函数
int mqtt_disconnect(void)
{
    int rc = EXIT_SUCCESS;

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS) // 断开和服务器的连接
    {
        printf("Failed to disconnect, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
    else
    {
        printf("MQTT disconnect success\n");
        MQTTClient_destroy(&client);
    }

    return rc;
}

// mqtt建立客户端、连接服务器、订阅主题的封装入口函数
int mqtt_iot(void)
{
    int rc = EXIT_SUCCESS;
    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
                                MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        goto exit;
    }
    // 设置回调函数，
    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd,
                                      NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        goto destroy_exit;
    }
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;
    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;
    // 连接服务器
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        goto destroy_exit;
    }

    // 订阅主题
    if ((rc = MQTTClient_subscribe(client, SUB_TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to subscribe, return code %d\n", rc);
        goto destroy_exit;
    }

    printf("MQTT connect success, press 'Q' or 'q' to disconnect mqtt server\n");
    return 0;

destroy_exit:
    MQTTClient_destroy(&client); // 释放客户端的资源
    return -1;
exit:
    return -1;
}