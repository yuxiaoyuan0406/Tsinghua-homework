/***************************************************************
功能 : ESP8266物联车接口函数
作者 : 马晓健
邮箱 : jeasinema[at]gmail[dot]com
声明 : 
本程序仅供学习与交流使用,如需他用,须联系作者
本程序可以随意更改,但须保留本信息页
All rights reserved
2017.6.16
***************************************************************/

#include "esp8266.h"

#include <cstdarg>
#include <cstring>
#include <stdint.h>
#include "mbed.h"

extern Serial ser2usb;

static int ser_baud = 9600;

//定义了一个调试的宏,C语言语法
#define ESP_CMD(format, ...)                         \
    do                                               \
    {                                                \
        char cmdbuf[128], *p;                        \
        ser2esp8266.printf("\r");                    \
        sprintf(cmdbuf, format "\r", ##__VA_ARGS__); \
        for (p = cmdbuf; *p; p++)                    \
        {                                            \
            ser2esp8266.putc(*p);                    \
            wait(0.02);                              \
        }                                            \
        wait(0.3);                                   \
    } while (0)

void Esp8266::gotResponse(char *token, char *param)
{
    if (*token < 'a' || *token > 'z')
        return;
    ser2usb.printf("gotResponse %s %s\r\n", token, param);
    if (strcmp(token, "connected") == 0)
        mqtt_start = true;
    else if (strcmp(token, "control") == 0)
    {
        if (!control_cmd)
        {
            strncpy(control_buf, param, sizeof(control_buf));
            control_cmd = true;
        }
    }
    else if (strcmp(token, "wifi") == 0)
    {
        if (*param == '5')
            network_start = true;
    }
}

bool Esp8266::get_control_cmd(char *actuator, char *value)
{
    if (!control_cmd)
        return false;

    char *plus = strchr(control_buf, '+');
    if (!plus)
    {
        control_cmd = false;
        return false;
    }
    *plus = '\0';
    strcpy(actuator, control_buf);
    strcpy(value, plus + 1);
    control_cmd = false;
    return true;
}

// 接收 esp8266 侧数据的回调函数, 每次仅接收一个8位字符
// 数据格式约定: #token+data
void Esp8266::esp8266_rxCallback()
{
    char in = ser2esp8266.getc();
    //    ser2usb.putc(in);
    enum
    {
        STATE_WAIT,
        STATE_TOKEN,
        STATE_PARAM
    };
    static uint8_t state = STATE_WAIT;
    static int tokenLen, paramLen;
    switch (state)
    {
    case STATE_WAIT:
        if (in == '#')
        {
            tokenLen = 0;
            state = STATE_TOKEN;
        }
        break;
    case STATE_TOKEN:
        if (in == '+' || in == '\r' || in == '\n')
        {
            esp_tokenBuf[tokenLen] = '\0';
            if (in == '+')
            {
                paramLen = 0;
                state = STATE_PARAM;
            }
            else
            {
                gotResponse(esp_tokenBuf, NULL);
                //memcpy(esp_token, esp_tokenBuf, tokenLen);
                //esp_token[tokenLen] = '\0';
                esp_buf_ready = true;
                state = STATE_WAIT;
            }
        }
        else if (tokenLen + 1 < sizeof(esp_tokenBuf))
        {
            esp_tokenBuf[tokenLen++] = in;
        }
        break;
    case STATE_PARAM:
        if (in == '\r' || in == '\n')
        {
            esp_paramBuf[paramLen] = '\0';
            gotResponse(esp_tokenBuf, esp_paramBuf);
            //memcpy(esp_token, esp_tokenBuf, tokenLen);
            //memcpy(esp_param, esp_paramBuf, paramLen);
            //esp_token[tokenLen] = '\0';
            //esp_param[paramLen] = '\0';
            //ser2usb.putc('?');
            esp_buf_ready = true;
            state = STATE_WAIT;
        }
        else if (paramLen + 1 < sizeof(esp_paramBuf))
        {
            esp_paramBuf[paramLen++] = in;
        }
        break;
    }
}

Esp8266::Esp8266(PinName TX, PinName RX, const char *wifi_ssid, const char *wifi_passwd) //定义类的函数
    : network_start(false), mqtt_start(false), control_cmd(false), esp_buf_ready(false), ser2esp8266(TX, RX)
{
    // serial to esp8266 init
    ser2esp8266.baud(ser_baud);
    ser2esp8266.attach(callback(this, &Esp8266::esp8266_rxCallback), Serial::RxIrq);
    //if (mode == 0) {                                                            // client mode
    this->reset();
    this->connect_wifi(wifi_ssid, wifi_passwd);
    while (!is_connected())
    {
        wait(0.5);
    }
    this->weblogin();
    //} else {
    //
    //}
}

bool Esp8266::reset()
{ //定义类的函数
    ESP_CMD("node.restart()");
    wait(2); // 延迟2s
    return true;
}

bool Esp8266::connect_wifi(const char *wifi_ssid, const char *wifi_passwd)
{ //定义类的函数
    ESP_CMD("wifi.setmode(wifi.STATION)");
    ESP_CMD("wifi.sta.config([[%s]],[[%s]])", wifi_ssid, wifi_passwd);
    wait(2);
    // set auto autoconnect
    ESP_CMD("wifi.sta.autoconnect(1)");
    return true;
}

bool Esp8266::is_connected()
{
    ESP_CMD("print('\\035wifi+'..wifi.sta.status())");
    wait(0.4);
    return network_start;
}

bool Esp8266::weblogin()
{ //定义类的函数
    // not implemented yet
    return true;
}

void Esp8266::buildCapability(char *out, const char *infoList[][2])
{
    out[0] = '\0';
    for (int i = 0; infoList[i][0]; ++i)
    {
        strcat(out, infoList[i][0]);
        strcat(out, ",");
        strcat(out, infoList[i][1]);
        strcat(out, "\\n");
    }
}

bool Esp8266::connect_mqtt_broker(char *ip, const char *node_name, const char *sensors[][2], const char *actuator[][2])
{ //定义类的函数

    ESP_CMD("node_name = '%s'", node_name);
    ESP_CMD("m = mqtt.Client('i_' .. node.chipid(), 120)");
    ESP_CMD("m:connect(\"%s\",1883,0,function(conn)print (\"\\035connected\"); end)", ip);

    do
    {
        wait(0.5);
    } while (!mqtt_start);

    ESP_CMD("m:on(\"message\", function(conn, topic, data)");
    ESP_CMD("if topic:find(\"^/control/\") then");
    ESP_CMD("local tok = topic:match(\"^/control/.+/(.+)\")");
    ESP_CMD("if tok then print(\"\\035control+\"..tok..\"+\"..data) end");
    ESP_CMD("end");
    ESP_CMD("end)");

    ESP_CMD("m:publish('/events/'..node_name..'/online','',1,0)");
    wait(0.1);

    char *capabilities = new char[512];

    if (sensors)
    {
        buildCapability(capabilities, sensors);
        ESP_CMD("m:publish('/capability/'..node_name..'/values','%s',1,1)", capabilities);
        wait(0.1);
    }
    if (actuator)
    {
        buildCapability(capabilities, actuator);
        ESP_CMD("m:publish('/capability/'..node_name..'/control','%s',1,1)", capabilities);
        wait(0.1);
        for (int i = 0; actuator[i][0]; ++i)
            subscribe_control(actuator[i][0]);
    }

    delete[] capabilities;

    return true;
}

bool Esp8266::publish_value(const char *topic, const char *data)
{   //定义类的函数
    //if (mqtt_start) {
    ESP_CMD("m:publish('/values/'..node_name..'/%s',\"%s\",0,1)", topic, data);
    wait(0.1);
    //}
    return true;
}

bool Esp8266::subscribe_control(const char *topic, const char *data)
{   //定义类的函数
    //if (mqtt_start) {
    ESP_CMD("m:subscribe('/control/'..node_name..'/%s', 0)", topic);
    wait(0.1);
    //}

    // ESP_CMD("m:unsubscribe(\"%s\")", topic);
    return true;
}
