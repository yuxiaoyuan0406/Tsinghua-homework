/*-----------------------------------------------------
 File Name : main.cpp
 Purpose : For esp8266 mbed porting
 Creation Date : 22-06-2017
 Last Modified :
 Created By : Jeasine Ma [jeasinema[at]gmail[dot]com]
-----------------------------------------------------*/
#include <cstdarg>
#include <cstring>
#include "mbed.h"
#include "esp8266.h"

Serial ser2usb(PB_10, PB_11, 115200);  
DigitalIn ref(PA_12);
DigitalOut LED(PC_13);
DigitalOut IN1(PB_4);
DigitalOut IN2(PB_3);
DigitalOut IN3(PA_15);
DigitalOut IN4(PA_12);
PwmOut ENA(PB_1);
PwmOut ENB(PB_0);
InterruptIn switch1(PB_6);
int state = 0;
int actionok = 0;
void run_forwoard();
void run_backwoard();
void stop();
void init();
void after_irq();
int switch_trigger;
int main(void) {
    IN1 = 0;
    IN2 = 0;
    IN3 = 0;
    IN4 = 0;
    ENA.period_ms(10);
    ENB.period_ms(10);
    ENA.pulsewidth_ms(5);
    ENB.pulsewidth_ms(5);
	
	
    ser2usb.printf("starting\r\n");
    
    // 选定与 esp8266 相连接的串口，WiFi 名称和密码
    Esp8266 client(PA_9, PA_10, "iot_b827eb8fb527", "7c02b50b");// 参数分别为 TX pin / RX pin / SSID / Password

    //声明所有的传感器，每行一个，每个由名字、单位两部分组成，最后一行必须为空指针作为结尾
    const char* sensors[][2] = {
        "actionok", "",
        NULL, NULL //最后一行以空指针作为结束标记
    };

    //声明所有的执行器，每行一个，每个由名字、参数类型两部分组成，最后一行必须为空指针作为结尾
    const char* actuators[][2] = {
        "do", "int",
        NULL, NULL //最后一行以空指针作为结束标记
    };
    ser2usb.printf("connecting...\r\n");

    //连接到服务器
    client.connect_mqtt_broker("192.168.12.1", "car", sensors, actuators);

    ser2usb.printf("Initialization done.\r\n");
    char actuator_name[32], control_value[32];
    Timer t;// 定时器用于计量发送传感器数据的时间
    t.start();
		client.publish_value("actionok", "carinit");
    while(1) {
        //检查有没有收到新的执行器控制指令
        if(client.get_control_cmd(actuator_name, control_value)){
            ser2usb.printf("Received CMD %s %s\r\n", actuator_name, control_value);
            //判断哪个执行器收到命令
            state = atoi(control_value);
            switch (state)
            {
                case 1:  // white to black
                    run_forwoard();
                    wait(1);
                    stop();
										client.publish_value("actionok", "carfinish");
                    break;
                default:
                    break;
                
            }
        }
    }
}


void run_forwoard()
{     IN1 = 1;
      IN2 = 0;
      IN3 = 0;
      IN4 = 1;
}

void run_backwoard()
{
    IN1 = 0;
    IN2 = 1;
    IN3 = 1;
    IN4 = 0;
}

void stop()
{
    IN1 = 0;
    IN2 = 0;
    IN3 = 0;
    IN4 = 0;
}
