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
//extern char sdata[];
//extern bool ifreceive;

//bool ifsent;

static int ser_baud = 9600;

//定义了一个调试的宏,C语言语法
#define ESP_CMD(format, ...) do{\
    char cmdbuf[128], *p;\
    ser2esp8266.printf("\r"); \
    sprintf(cmdbuf, format "\r", ##__VA_ARGS__);\
    for(p=cmdbuf;*p;p++){\
        ser2esp8266.putc(*p);\
        wait(0.02);\
    }\
    wait(0.3);\
}while(0)



void Esp8266::gotResponse(char *token, char *param)
{
    if(*token<'a' || *token>'z') return;
    ser2usb.printf("gotResponse %s %s\r\n", token, param);
    if(strcmp(token, "connection") == 0)
        tcp_start = true;
    else if(strcmp(token, "wifi") == 0){
        if(*param == '5')
            network_start = true;
    }
    else if(strcmp(token, "sent") == 0){
        //if(*param == '11')
            ifsent = true;
    }
    else if(strcmp(token, "receive") == 0){   //#receive+sdata
        //if(status==2)
        ser2usb.printf("debug8 \r\n");
        //strcpy(sdata,param);
        strcat(data,param);
        ifreceive = true;
        ser2usb.printf("debug9 |%i| \r\n",strlen(data));
            //sdata = param;
        /*
        if(*param == 'a'&&status==2)         //0: 初始化;  1:建立通信;  2:等候任务;  3:等待数据    4:正在执行一个任务
            status = 3;
        if(*param != 'a'&&status==3)
            wait(!canRdata);
            sdata = param;
        */
    }
}


// 接收 esp8266 侧数据的回调函数, 每次仅接收一个8位字符
// 数据格式约定: #token+data
void Esp8266::esp8266_rxCallback() {
    char in = ser2esp8266.getc();
    //ser2usb.putc(in);///
    enum{STATE_WAIT, STATE_TOKEN, STATE_PARAM};
    static uint8_t state = STATE_WAIT;
    static int tokenLen, paramLen;
    switch(state){
    case STATE_WAIT:
        if(in == '#'){
            tokenLen = 0;
            state = STATE_TOKEN;
        }
        break;
    case STATE_TOKEN:
        if(in == '+' || in == '\r' || in == '\n'){
            esp_tokenBuf[tokenLen] = '\0';
            if(in == '+'){
                paramLen = 0;
                state = STATE_PARAM;
            }else{
                gotResponse(esp_tokenBuf, NULL);
                //memcpy(esp_token, esp_tokenBuf, tokenLen);
                //esp_token[tokenLen] = '\0';
                esp_buf_ready = true;
                state = STATE_WAIT;
            }
        }else if(tokenLen+1 < sizeof(esp_tokenBuf)){
            esp_tokenBuf[tokenLen++] = in;
        }
        break;
    case STATE_PARAM:
        if(in == '\r' || in == '\n'){
            esp_paramBuf[paramLen] = '\0';
            gotResponse(esp_tokenBuf, esp_paramBuf);
            //memcpy(esp_token, esp_tokenBuf, tokenLen);
            //memcpy(esp_param, esp_paramBuf, paramLen);
            //esp_token[tokenLen] = '\0';
            //esp_param[paramLen] = '\0';
            //ser2usb.putc('?');
            esp_buf_ready = true;
            state = STATE_WAIT;
        }else if(paramLen+1 < sizeof(esp_paramBuf)){
            esp_paramBuf[paramLen++] = in;
        }
        break;
    }
}


Esp8266::Esp8266(PinName TX, PinName RX, const char *wifi_ssid, const char *wifi_passwd)       //定义类的函数
    : network_start(false), tcp_start(false), esp_buf_ready(false), ser2esp8266(TX, RX)
{
    strcpy(data,"");
    ser2esp8266.baud(ser_baud);
    ser2esp8266.attach(callback(this,&Esp8266::esp8266_rxCallback), Serial::RxIrq); 
        this->reset();
        this->connect_wifi(wifi_ssid, wifi_passwd);
        while(!is_connected()){
            wait(0.5);
        }
}

bool Esp8266::reset() {                                                         //定义类的函数
    ESP_CMD("node.restart()");
    wait(2);                                                                    // 延迟2s
    return true;
}   

bool Esp8266::connect_wifi(const char *wifi_ssid, const char *wifi_passwd) {                                                  //定义类的函数
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

    
bool Esp8266::connect_tcp_pc(char *ip,char *port) {  //定义类的函数
    ser2usb.printf("haha1");

    ESP_CMD("sk = net.createConnection(net.TCP, 0)");
    ser2usb.printf("haha2");
    ESP_CMD("sk:on(\"receive\", function(sck, c) print('\\035receive+'..c) end )");
    ser2usb.printf("haha3");
    ESP_CMD("sk:on(\"connection\", function(sck) print('\\035connection+1') end )");
    ESP_CMD("sk:on(\"sent\", function(sck) print('\\035sent+11') end )");
    ser2usb.printf("haha4");
    ESP_CMD("sk:connect(%s,\"%s\")", port, ip);
    ser2usb.printf("haha15");
    
    
    do{    
        wait(0.5);
        //ser2usb.printf("haha6");
    }while(!tcp_start);
    
    ser2usb.printf("haha6 %s \r\n",tcp_start==false?"FALSE":"TRUE"); 
    //wait(10);
    //ser2usb.printf("haha7 %s \r\n",tcp_start==false?"FALSE":"TRUE"); 
    
    
    //ser2usb.printf("haha8");
    
    wait(0.1);

    
    return true;
}
    
bool Esp8266::tcp_send(const char *data) {                      //定义类的函数
    //if (mqtt_start) {
        ifsent = false;
        wait(0.1);
        while(!ifsent){
            ESP_CMD("sk:send(\"%s\")" , data);
            wait(0.5);
        }
        //ESP_CMD("sk:send(\"%s\")" , data);
        //wait(0.2);
        //ser2usb.printf("sk:send(\"%s\")", data);
        //wait(0.1);
    //}
    return true;
}