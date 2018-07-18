
#include "mbed.h"

class Esp8266
{ //声明一个类
    volatile bool network_start;
    volatile bool mqtt_start;
    volatile bool control_cmd;
    char esp_tokenBuf[32], esp_paramBuf[32]; // recv from esp8266
    char control_buf[32];
    bool esp_buf_ready;
    Serial ser2esp8266;

  protected:
    void esp8266_rxCallback();
    void gotResponse(char *token, char *param);
    void buildCapability(char *out, const char *infoList[][2]);

  public:
    Esp8266(PinName TX, PinName RX, const char *wifi_ssid, const char *wifi_passwd);

    // 通用
    bool reset();

    // 连接模式
    bool connect_wifi(const char *wifi_ssid, const char *wifi_passwd);
    bool weblogin();
    bool connect_mqtt_broker(char *ip, const char *node_name, const char *sensors[][2], const char *actuator[][2]);
    bool is_connected();

    bool publish_value(const char *topic, const char *data);
    bool subscribe_control(const char *topic, const char *data = NULL);
    bool is_control_available(void) { return control_cmd; }
    bool get_control_cmd(char *actuator, char *value);
    // 热点模式
};
