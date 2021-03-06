/************************************************************************************************
File name:          main.cpp
Description:        项目激光部分客户端代码
Author:             秦智
Date:               June 4, 2018
Others:             本程序编写在mbed平台(https://os.mbed.com/)上
                    esp8266 相关部分参考助教提供的代码修改得到，见esp8266.h和esp8266.cpp，主要是将其中的MQTT协议改为了TCP协议，并对接受数据进行了处理
                    
*************************************************************************************************/

#include "mbed.h"
#include <math.h>
#include <cstring>
#include <stdlib.h>

#include "esp8266.h"


/////调试
Ticker ticker_ser,ticker_wifi;
Serial ser2usb(PB_10, PB_11, 115200);
//DigitalIn ref(PA_11);
DigitalOut LED(PC_13);
/////

//硬件接口
    //步进电机
DigitalOut step[2] = {DigitalOut(PB_0),DigitalOut(PA_7)};  //0--x P9,1--y P17
DigitalOut dir[2] = {DigitalOut(PB_7),DigitalOut(PB_4)};  //0--x,1--y
DigitalOut en[2] = {DigitalOut(PB_9),DigitalOut(PB_6)};  //0--x,1--y
    //电子开关,激光开关
DigitalOut switch_GS(PA_8);
    //行程开关
DigitalIn switch_pos1(PB_1);   //P25
DigitalIn switch_pos2(PA_15);  //P26
DigitalIn switch_pos3(PA_11);  //P27

//运行中的全局变量
bool Working,Getawork,Isend,Dataused,Getdata;
int status=0;  //0: 初始化;  1:建立通信;  2:等候任务;  3:等待数据    4:正在执行一个任务
int now_x,now_y;
int Endoffile = 0;

float thedata[50][3];
char sdata[1024];
int Nofdata;
bool ifreceive;

//与实际有关参数
float Lenoflattice = 1 ;//mm  取1mm为xy单元  unit_xy/Lenoflattice=100
int unit_xy = 100;  //单位长度(xy移动一格)对应unit_xy转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
int unit_x = 103;  //单位长度(xy移动一格)对应unit_x转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
int unit_y = 103;  //单位长度(xy移动一格)对应unit_y转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
///Ticker ticker_step;
float step_halfperiod = 0.002;//0.02;
int max_x,max_y;
int dir_x = 1;///调试时调整
int dir_y = 1;///调试时调整
int dot_max = 10;      //灰度最大值对应多少次激光点击
float dot_last = 0.5;    //每次激光点击持续多久
int H_max = 255;  //灰度值最大值





void report(){
    ser2usb.printf("-1- status: %d \r\n", status);
    ser2usb.printf("-2- now_x: %d now_y: %d \r\n", now_x,now_y);
    //ser2usb.printf("-3- switch_GS: %i \r\n", switch_GS.read());
    //ser2usb.printf("-4- switch_pos1: %i switch_pos2: %i switch_pos3: %i \r\n", switch_pos1.read(),switch_pos2.read(),switch_pos3.read());
    //ser2usb.printf("-5- en[0]: %i en[1]: %i \r\n", en[0].read(),en[1].read());
    //ser2usb.printf("-6- dir[0]: %i dir[1]: %i \r\n", dir[0].read(),dir[1].read());
    //ser2usb.printf("-7- step[0]: %i step[1]: %i \r\n", step[0].read(),step[1].read());
}
/*
void reportwifi(){
    char* topc;
    sprintf(topc,"-1- status: %d \r\n", status);
    client.tcp_send(topc);
    sprintf(topc,"-2- now_x: %d now_y: %d \r\n", now_x,now_y);
    client.tcp_send(topc);
}
*/

void init(){
    status=0;
    now_x=0;
    now_y=0;
    step[0]=0;
    step[1]=0;
    //ticker_ser.attach(&report, 2);
    //ticker_wifi.attach(&reportwifi, 10);
}
/*
void ALLconnect(){
    
}
*/






/*

void move_d(int xy, int d){  //xy= 0--x,1--y

    for(int i=0;i<d;i++){
        step[xy] = 1;
        wait(0.01);
    }
    
}

*/
void rotate(int id,int pix){ //id= 0--x,1--y  pix=3200为一圈
    if(pix>=0){
       dir[0]=dir_x; 
       dir[1]=dir_y; 
    }
    else{
       pix = -pix;
       dir[0]=1-dir_x;
       dir[1]=1-dir_y;
    }
    for(int i=0;i<pix;i++){
        step[id] = 1;
        wait(step_halfperiod);
        step[id]=0;
        wait(step_halfperiod);
    }
}

void moveto(float x,float y){
    rotate(0,(x-now_x)*unit_xy);
    rotate(1,(y-now_y)*unit_xy);
    now_x = x;
    now_y = y;
}
void fuwei(){
    moveto(0,0);
    status = 1;
}
void markdot(float value){
    //switch_GS=1;
    int Ndot = floor(value*dot_max/H_max);
    for(int i=0;i<Ndot;i++){
        switch_GS=1;
        wait(dot_last);
        switch_GS=0;
    }
}

/*
void step_pulse() { //产生STEP脉冲信号
    if(remain==0||aaa==0)
        return;
        
    switch(aaa){
        case 1:
            break;
        case 2:
            break;
    }
    if(step){
        step = 0; //STEP 1->0
        remain--;
    }
    else{
        step = 1; //STEP 0->1
    }
}
*/


void dealdata(){
    ser2usb.printf("getdata: %s \r\n",sdata);
    int data_size = 0;//strlen(pch);
    char * pch;
    pch = strtok (sdata,"\r\n ,.-)(");
    while (pch != NULL)
    {
        thedata[data_size/3][data_size%3]=atof(pch);
        pch = strtok (NULL, "\r\n ,.-)(");
        data_size++;
    }
    Nofdata = (data_size)/3;
}

void usbprintdata(){
    ser2usb.printf("dealdata:  \r\n   <<<<<<<<<<<<<<<<");
    for(int i=0;i<Nofdata;i++){
        ser2usb.printf("<<< i: %i  x: %f  y: %f  v: %f \r\n",i,thedata[i][0],thedata[i][1],thedata[i][2]);
    }
    ser2usb.printf("dealdata done!    \r\n    >>>>>>>>>>>>");
}

void draw(){
    for(int i=0;i<Nofdata;i++){
        moveto(thedata[i][0],thedata[i][1]);
        markdot(thedata[i][2]);
    }
}
/*
void waitthedata(){
    //ifreceive = false;
    //client.tcp_send("giveme!\r\n");
    while(!ifreceive){
        //ser2usb.printf("debug2 \r\n");
        wait(0.5);
        //ser2usb.printf("debug3 \r\n");
    }
}
*/


int main(){
    init();     //初始化
    //ALLconnect();   
    //建立通信
    
    status = 1;
    
    ser2usb.printf("starting\r\n");
    
    // 选定与 esp8266 相连接的串口，WiFi 名称和密码
    //Esp8266 client(PA_9, PA_10, "TP_qz_wifi", "abcd4321");// 参数分别为 TX pin / RX pin / SSID / Password
    Esp8266 client(PA_9, PA_10, "TSINGHUA.iCenter", "TS.icenter");// 参数分别为 TX pin / RX pin / SSID / Password

    ser2usb.printf("connecting...\r\n");

    //连接到服务器
    client.connect_tcp_pc("101.6.161.127","12345");
    client.tcp_send("Initialization done.\r\n");
    
    ser2usb.printf("Initialization done.\r\n");
    
    status = 2;
    
    wait(1);
    client.tcp_send("lalala.\r\n");
    client.tcp_send("ready.\r\n");      //0: 初始化;  1:建立通信;  2:等候任务;  3:等待数据    4:正在执行一个任务
    ser2usb.printf("laser printer send ready.\r\n");
    wait(1);
    //ifreceive = false;
    client.ifreceive = false;
    client.tcp_send("a");       //0表示服务器可以发新任务了
    Getawork = 0;
    
    
    while(true){
        //ser2usb.printf("debug1 \r\n");
        waitthedata();
        
        while(!client.ifreceive){
            //ser2usb.printf("debug2 \r\n");
            wait(1);
            ser2usb.printf("debug3 \r\n");
        }
        //ser2usb.printf("debug4 \r\n");
        dealdata();
        //ser2usb.printf("debug5 \r\n");
        usbprintdata();
        draw();
        //ifreceive = false;
        client.ifreceive = false;
        //ser2usb.printf("debug11 \r\n");
        client.tcp_send("done.\r\n");
        
        //client.tcp_send("done.\r\n");
        //client.tcp_send("done.\r\n");
    }
    
    /*    //这是一个的之前版本
    //进入工作状态
    while(true){
        client.tcp_send(sdata);
        wait(2);
        Getawork = 0;
        while(!Getawork){
            client.tcp_send(sdata);
            //ser2usb.printf("sdata: %s\n",sdata);
            ser2usb.printf(sdata);
            wait(2);
            if(sdata == "a"){
                Getawork = 1;
                status = 3;
            } 
        }
        Getawork = 0;
        client.tcp_send("1");  //1表示服务器可以发新任务的数据了
        Working = 1;
        while(Working){
            client.tcp_send(sdata);
            wait(2);
            Getdata = 0;
            while(!Getdata){
                client.tcp_send(sdata);
                wait(2);
                client.tcp_send("1");  //1表示服务器可以发新任务的数据了
                if(sdata != "a"){
                    Getdata = 1;
                    status = 4;
                }
            }
            Getdata = 0;
            if(sdata != "e"){
                dealdata();   //处理
                draw(); //画图
                client.tcp_send("2");       //2表示服务器可以发新一行的数据了
            }
            else{
                fuwei();
                Working = 0;
                client.tcp_send("0");       //0表示服务器可以发新任务了
            }
            status = 3;
        }
    }
    */
}

/////调试
/*
int main() {
    init();
    ticker_ser.attach(&report, 2);
    int x,y;
    x=10;
    y=10;
    switch_GS=1;
    while(1) {
        
        moveto(x,y);
        x=x+10;
        y=y+10;
        
        LED = 1; // LED is ON
        wait(2); // 200 ms
        LED = 0; // LED is OFF
        wait(2); // 1 sec
    }
}
*/
/////

/*
int main(){
    init();     //初始化
    ALLconnect();   //建立通信
    client.tcp_send("ready");      //0: 初始化;  1:建立通信;  2:等候任务;  3:等待数据    4:正在执行一个任务
    //进入工作状态
    while(true){
        switch(statues){
            case 2:
                while(!Getawork){}
                status = 3;
                break;
            case 3:
                getdata();
                if(Isend){
                    Getawork = 0;
                    status = 2;
                }
                else{
                    status = 4;
                }
                break;
            case 4:
                draw();
                Dataused = 1;
                status = 3;
                break;
        }
    }  
}
*/

/*
int main() {
    init();
    //ticker_step.attach(&step_pulse, step_period);
    while(1) {
        if(!Working){
            while(!Getawork){
                
            }
        }
        while(1);
        myled = 1; // LED is ON
        wait(0.2); // 200 ms
        myled = 0; // LED is OFF
        wait(1.0); // 1 sec
    }
}
*/



/*
int main(void) {
    ser2usb.printf("starting\r\n");
    
    // 选定与 esp8266 相连接的串口，WiFi 名称和密码
    Esp8266 client(PA_9, PA_10, "TP_qz_wifi", "abcd4321");// 参数分别为 TX pin / RX pin / SSID / Password
    


    ser2usb.printf("connecting...\r\n");

    //连接到服务器
    //client.connect_tcp_pc("192.168.12.1"); 
    //client.connect_tcp_pc("59.66.124.137","12345");
    
    client.connect_tcp_pc("192.168.1.101","12345");
    client.tcp_send("qinzhi2yiran");
    
    ser2usb.printf("Initialization done.\r\n");

    bool ref_last;
    float last_report=0;
    Timer t;// 定时器用于计量发送传感器数据的时间
    t.start();

    while(1) {
        
        client.tcp_send("qinzhi");
        
        bool reflection = ref;
        
        if(reflection != ref_last){ //仅在传感器发生变化时汇报数据
        
            ref_last = reflection;

            char val[4];
            sprintf(val, "%d", (int)reflection);
            
            //汇报传感器数据，两个参数分别是传感器名字和值
            //client.publish_value("reflection", val);
        }

        if(t.read() - last_report > 1){ // 每1s发送汇报一次w位置

            
            last_report = t.read();
        }
        wait(2);
    }
}

*/

