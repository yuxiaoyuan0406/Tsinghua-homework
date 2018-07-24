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
float Lenoflattice = 0.5 ;//mm  取1mm为xy单元  unit_xy/Lenoflattice=100
int unit_xy = 40;//100;  //单位长度(xy移动一格)对应unit_xy转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
int unit_x = 103;  //单位长度(xy移动一格)对应unit_x转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
int unit_y = 103;  //单位长度(xy移动一格)对应unit_y转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
///Ticker ticker_step;
float step_halfperiod = 0.0001;//0.02;
int max_x,max_y;
int dir_x = 1;///调试时调整
int dir_y = 1;///调试时调整
int dot_max = 1;      //灰度最大值对应多少次激光点击
float dot_last = 2;    //每次激光点击持续多久
int H_max = 255;  //灰度值最大值



/*
void reportwifi(){
    char* topc;
    sprintf(topc,"-1- status: %d \r\n", status);
    client.tcp_send(topc);
    sprintf(topc,"-2- now_x: %d now_y: %d \r\n", now_x,now_y);
    client.tcp_send(topc);
}
*/

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

void moveto(float x,float y){       //移动到坐标(x,y)
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


void dealdata(){            //按括号，空格等（"\r\n ,.-)("）分割发来的字符串数据
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

void draw(){    //画出收到的点
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

//char ssdata[] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111111111111111111111111111111111111111111111111111111111111111111111111111111000000000001111111011111111111101111111111110111111111111011111111111101111111111110111111111111011000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111111111110001111000101111010111111111111111111100111111110011111111111011111000000000001111111111011111111111111111111100101111111111111110111111111111110101111111011111101111000000000001111011111011111111111111111111111100111111111111111111111111111111111111111110111011011000000000001111111111111111111110111010111111111111111110111111111111011011111111110111111010111111000000000001101111111011111111111111111110110111111101111111111111111011011110111111111111111111111000000000001011111110110111111111111111111011111111111111111111111110111111111100111111111111111111000000000001111011111111111111111111111111101111101111111111111111110101111111111111111101111101111000000000001000000000000000000000000000000000000000000000000000000001000000000000010000000000000001000000000001000000000000001000000000000000000000000000000000000000001000000000000010000000000000001000000000001000000000000001000000000000000000000000000000000000000001000000000000010000000000000001000000000001000000000000000000000000000000000000000000000000000000001000000000000010000000000000001000000000001000000000000000000000000000000000000000000000000000000001000000000000010000000000000001000000000001000101100000000000000010000000000000000000000011011100000000000000000010000000010000001000000000000000101000000000000001111000000000000010000000000000100001000000000000010000001010000001000000000001000001101000000000100010010000001011000000000011101000001000000000000000010001010000001000000000001000111000000000000000111000000000001000000001100000010001000000000000010011000110000001000000000000000001010000000000011000000000000001000000000010011100001000111111100010001001010000001000000000001000111010000000000100111000000000001000000000111001000001000000000000010011001110000001000000000001000101100000000000100101000000000001000000000110011110001000000000000010000001010000001000000000001000101000000000000000111000000000111111100000010000000001000000000000000001100010000001000000000001000001000100000000111000000000000000000000000010001111001000000000000000010100111100001000000000001000000000000000000000000000000000000000000000010100000001000000000000000000000000000001000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000001000000000000000000000000000000000000000000000000000000001000000000000100000000000000001000000000001111111000001111011011111101101111111111111111111111111111111111111111111110111111111111000000000001111011110101111011011111111111111101111111110111111111111111111111111111011011111111111000000000001101111110110111111111111111111110111111010111111111111111101111111110011111111111111111000000000001100101111111111111111111111110111111111111111011111011111111001111111111111110111111111000000000001111111111111101111111111111101111111111111111001111011111111111111111010111101111111111000000000001101011111110111111111101111111111101111010111110111111111101111110111111111111111111111000000000001110101111111111111101110111111111111101111110011101111111111111101111110011111011111111000000000001111111111111111101111111111111111111111011111110111111111111111010001110111011111111111000000000001111101111111110101111111111111111101101110111111111111111011111111111111111111111111111000000000001010111111111110111111111011111111010111011110111110011011111111111111101111011111111101000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
/*
void new_draw(){
    for(int x=1;x<=100;x++){
        for(int y=1;y<=100;y++){
            int v = ssdata[(100*(x-1)+y)-1];
            if(v=='1'){
                moveto(y,x);
                markdot(255);
            }
        }
    };
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
    client.connect_tcp_pc("101.6.161.53","12345");
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
    
    wait(10);
    
    //new_draw:
    for(int x=1;x<=100;x++){
        for(int y=1;y<=100;y++){
            int v = client.data[(100*(x-1)+y)-1];
            if(v=='1'){
                moveto(y,x);
                markdot(255);
            }
        }
    }
    
    fuwei();
}

//
/*
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
    
    Nofdata = 0;
    
    while(true){
        //ser2usb.printf("debug1 \r\n");
        //waitthedata();
        
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
        while(!client.ifreceive){
            //ser2usb.printf("debug2 \r\n");
            wait(0.3);
            ser2usb.printf("debug3 \r\n");
        }
        Nofdata = 0;
        //client.tcp_send("done.\r\n");
        //client.tcp_send("done.\r\n");
    }
    
    
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
    
}
*/
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





