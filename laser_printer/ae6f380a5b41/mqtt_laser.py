#!/usr/bin/env python  
# -*- coding: UTF-8 -*-

'''
File name:          laser_server_v2.py
Description:        项目激光部分服务器代码
Author:             秦智
Date:               June 4, 2018
Others:             
Dependency:         pip install Pillow paho-mqtt
                    
'''

from time import sleep
import paho.mqtt.client as mqtt
import threading
import time
from PIL import Image


def waitfile(NumofOnce,minvalue):
    filename = "laser-6-4.JPG"
    im = Image.open(filename) #打开图片
    print("get a picture,the info about filename, im.format, im.size, im.mode:")
    print(filename, im.format, im.size, im.mode)
    imL = im.convert("L")
    x = imL.size[0] #图片的长
    y = imL.size[1] #图片的宽
    y=int(y*100/x)
    x=100
    imL=imL.resize((x,y)) #缩放图片
    datalist = [] #存放坐标点的数组
    for iy in range(0,y):
        for ix in range(0,x):
            value = imL.getpixel((ix,iy)) #获取像素亮度
            if value >=minvalue: #判断阈值
                datalist.append([ix,iy,value])
    strdatalist=[]
    sdata = ""
    nn=0
    dotfile = open("write.txt","w")
    while len(datalist)>0:
        data = datalist.pop(0)
        nn=nn+1
        # 将坐标转为字符串(x,y,v)
        sdata = sdata+"("+str(data[0])+","+str(data[1])+","+str(data[2])+")"
        dotfile.write(sdata)
        if nn == NumofOnce or len(datalist)==0:
            # 构造要发送的字符串数组
            strdatalist.append(sdata)
            sdata = ""
            nn = 0
    dotfile.close()
    strdatalist.append("(0,0,0)")
    return strdatalist

def NewWork(conn):
    datalist = waitfile(5,30)
    for datastr in datalist:
        print("senddata:"+datastr)
        conn.publish('/control/laser/dat', datastr, qos=1)
        time.sleep(0.01)

def on_message(conn, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    if msg.payload == 'ready.':
        print('设备已启动')
        # 告诉设备，有新任务
        conn.publish('/control/laser/cmd', "1", qos=1)
    elif msg.payload == 'wait data.':
        print('等待接收数据')
        # 调用发送数据函数
        NewWork(conn)
        # 告诉设备，数据发送完
        conn.publish('/control/laser/cmd', "2", qos=1)
    elif msg.payload == 'received.':
        print('数据接收完成')
    elif msg.payload == 'done.':
        print('任务完成')

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

waitfile(10,30); #测试保存文件功能
'''
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("tdxls-iot.xicp.net", 1883, 60)
client.subscribe('/values/laser/report', 1)
client.loop_start()
while True:
    sleep(0.1)
'''

