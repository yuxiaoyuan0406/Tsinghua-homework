
#include "mbed.h"

class Esp8266 {                                                                 //声明一个类
    volatile bool network_start;
    volatile bool tcp_start;
    char esp_tokenBuf[32], esp_paramBuf[32];   // recv from esp8266
    char control_buf[32];
    bool esp_buf_ready;
    Serial ser2esp8266;
    
protected:
    void esp8266_rxCallback();
    void gotResponse(char *token, char *param);
public:
    Esp8266(PinName TX, PinName RX, const char *wifi_ssid, const char *wifi_passwd);
    
    // 通用
    bool reset();
    
    // 连接模式
    bool connect_wifi(const char *wifi_ssid, const char *wifi_passwd);
    bool connect_tcp_pc(char *ip,char *port);
    bool is_connected();
    
    bool tcp_send(const char *data);
    
    bool ifreceive;   //用于判断是否收到
    bool ifsent;      //判断是否发出
    char data[15000]; //存储数据

    // 热点模式
};