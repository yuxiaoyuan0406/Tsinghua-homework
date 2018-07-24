/************************************************************************************************
File name:          main.cpp
Description:        项目激光部分客户端代码
Author:             秦智
Date:               June 4, 2018
Others:             本程序编写在mbed平台(https://os.mbed.com/)上

*************************************************************************************************/

#include "mbed.h"
#include <math.h>
#include <cstring>
#include <stdlib.h>
#include "SDFileSystem.h"
#include "networking.h"


/////调试
Serial pc(PA_9, PA_10);
DigitalOut LED(PB_8);
/////

//硬件接口
// mosi, miso, sclk, cs, name
SDFileSystem sd(PB_15, PB_14, PB_13, PB_12, "sd");
//步进电机
DigitalOut step[2] = {DigitalOut(PC_5), DigitalOut(PA_5)}; //0--x P9,1--y P17
DigitalOut dir[2] = {DigitalOut(PC_4), DigitalOut(PA_4)}; //0--x,1--y
DigitalOut en[2] = {DigitalOut(PD_2), DigitalOut(PA_2)}; //0--x,1--y
//电子开关,激光开关
DigitalOut switch_GS(PC_15);
//行程开关
// DigitalIn switch_pos1(PB_1);   //P25
// DigitalIn switch_pos2(PA_15);  //P26
// DigitalIn switch_pos3(PA_11);  //P27

//运行中的全局变量
volatile bool Working, Getawork, Isend, Dataused, Getdata;
FILE *fp_drawing; // 存储图案的文件
int status = 0; //0: 初始化;  1:建立通信;  2:等候任务;  3:等待数据    4:正在执行一个任务
int now_x, now_y;
int Endoffile = 0;

// float thedata[50][3];
// char sdata[1024];
int Nofdata;
bool ifreceive;

//与实际有关参数
float Lenoflattice = 1 ;//mm  取1mm为xy单元  unit_xy/Lenoflattice=100
int unit_xy = 100;  //单位长度(xy移动一格)对应unit_xy转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
int unit_x = 103;  //单位长度(xy移动一格)对应unit_x转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
int unit_y = 103;  //单位长度(xy移动一格)对应unit_y转   大约10cm/3圈/9600step 约96mm/9600step=0.1mm/10step
///Ticker ticker_step;
float step_halfperiod = 0.002;//0.02;
int max_x, max_y;
int dir_x = 1;///调试时调整
int dir_y = 1;///调试时调整
int dot_max = 10;      //灰度最大值对应多少次激光点击
float dot_last = 0.5;    //每次激光点击持续多久
int H_max = 255;  //灰度值最大值





void report()
{
    // ser2usb.printf("-1- status: %d \r\n", status);
    // ser2usb.printf("-2- now_x: %d now_y: %d \r\n", now_x,now_y);
    //ser2usb.printf("-3- switch_GS: %i \r\n", switch_GS.read());
    //ser2usb.printf("-4- switch_pos1: %i switch_pos2: %i switch_pos3: %i \r\n", switch_pos1.read(),switch_pos2.read(),switch_pos3.read());
    //ser2usb.printf("-5- en[0]: %i en[1]: %i \r\n", en[0].read(),en[1].read());
    //ser2usb.printf("-6- dir[0]: %i dir[1]: %i \r\n", dir[0].read(),dir[1].read());
    //ser2usb.printf("-7- step[0]: %i step[1]: %i \r\n", step[0].read(),step[1].read());
}

void init()
{
    status = 0;
    now_x = 0;
    now_y = 0;
    step[0] = 0;
    step[1] = 0;
}
void rotate(int id, int pix) //id= 0--x,1--y  pix=3200为一圈
{
    if (pix >= 0) {
        dir[0] = dir_x;
        dir[1] = dir_y;
    } else {
        pix = -pix;
        dir[0] = 1 - dir_x;
        dir[1] = 1 - dir_y;
    }
    for (int i = 0; i < pix; i++) {
        step[id] = 1;
        wait(step_halfperiod);
        step[id] = 0;
        wait(step_halfperiod);
    }
}

void moveto(float x, float y)
{
    rotate(0, (x - now_x)*unit_xy);
    rotate(1, (y - now_y)*unit_xy);
    now_x = x;
    now_y = y;
}
void fuwei()
{
    moveto(0, 0);
    status = 1;
}
void markdot(float value)
{
    //switch_GS=1;
    int Ndot = floor(value * dot_max / H_max);
    for (int i = 0; i < Ndot; i++) {
        switch_GS = 1;
        wait(dot_last);
        switch_GS = 0;
    }
}
/*
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
*/

void draw()
{
    float x, y, v;
    fp_drawing = fopen("/sd/write.txt", "r");
    for (int i = 0; fscanf(fp_drawing, "(%f,%f,%f)", &x, &y, &v) == 3; i++) {
        pc.printf("(%f,%f,%f)\r\n", x, y, v);
        moveto(x, y);
        markdot(v);
        // moveto(thedata[i][0],thedata[i][1]);
        // markdot(thedata[i][2]);
    }
    fclose(fp_drawing);
}

void on_control_cmd(const char* actuator_name, const char* control_value)
{
    pc.printf("Received [%s] [%s]\r\n", actuator_name, control_value);
    if (strcmp(actuator_name, "dat") == 0) {
        //接受到的坐标数据写入存储卡
        fprintf(fp_drawing, "%s", control_value);
    } else if (strcmp(actuator_name, "cmd") == 0) {
        int the_command = atoi(control_value);
        if (the_command == 1) {
            Getawork = 1;
        } else if (the_command == 2) {
            Getdata = 1;
        }
    }
}

int main()
{
    init();     //初始化

    //建立通信
    status = 1;

    pc.printf("starting\r\n");

    MQTTSocket sock;
    MClient client(sock);

    //声明所有的传感器，每行一个，每个由名字、单位两部分组成，最后一行必须为空指针作为结尾
    const char* sensors[][2] = {
        "report", "",
        NULL, NULL //最后一行以空指针作为结束标记
    };

    //声明所有的执行器，每行一个，每个由名字、参数类型两部分组成，最后一行必须为空指针作为结尾
    const char* actuators[][2] = {
        "dat", "",
        "cmd", "",
        NULL, NULL //最后一行以空指针作为结束标记
    };
    pc.printf("connecting...\r\n");

    networking_init(sock, client, "tdxls-iot.xicp.net", sensors, actuators, on_control_cmd);

    pc.printf("Initialization done.\r\n");

    status = 2;

    publish_value(client, "report", "ready.");
    pc.printf("laser printer send ready.\r\n");

    Getawork = 0;
    Getdata = 0;

    while (true) {
        if (status == 2) {
            if (Getawork) { //收到新任务
                Getawork = 0;
                pc.printf("Begin receiving...\r\n");
                // 打开文件，准备接受坐标数据
                fp_drawing = fopen("/sd/write.txt", "w");
                if (fp_drawing) {
                    pc.printf("File opened\r\n");
                    publish_value(client, "report", "wait data.");
                    status = 3;
                }
            }
        } else if (status == 3) {
            if (Getdata) { //数据接收完
                Getdata = 0;
                pc.printf("End receiving...\r\n");
                fclose(fp_drawing);
                publish_value(client, "report", "received.");
                status = 4;
            }
        } else if (status == 4) {
            // pc.printf("debug4 \r\n");
            // dealdata();
            pc.printf("Drawing.. \r\n");
            // usbprintdata();
            draw();
            publish_value(client, "report", "done.");
            status = 2;
        }
        client.yield(1000);
    }

}
