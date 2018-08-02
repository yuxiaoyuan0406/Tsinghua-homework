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



filename = "file1.jpg"
xsize = 50           #x的缩放像素长度


def waitfile(NumofOnce, minvalue):
    def reverse(val):
        return 255 - val
    im = Image.open(filename).convert('L')  # 打开图片，并处理成灰度图
    im.save('grayed-' + filename, 'jpeg')#保存处理后的图像

    im = Image.eval(im, reverse)

    im.save('reversed-' + filename, 'jpeg')#保存处理后的图像

    print("get a picture,the info about filename, im.format, im.size, im.mode:")

    print(filename, im.format, im.size, im.mode)

    x = im.size[0]  # 图片的长

    y = im.size[1]  # 图片的宽

    y = int(y*xsize/x)

    x = xsize

    im = im.resize((x, y))  # 缩放图片

    #filename = filename
    im.save('resized-' + filename, 'jpeg')#保存处理后的图像

    datalist = []  # 存放坐标点的数组

    for iy in range(0, y):

        for ix in range(0, x):

            value = im.getpixel((ix, iy))  # 获取像素亮度
            #im.getpixel((ix, iy)) = 255 - value
            if value >= minvalue:  # 判断阈值
                #print([ix, iy, value])
                datalist.append([ix, iy, value])
    '''
    for i in range(0, len(datalist)):
        for j in range(i+1, len(datalist)):
            if datalist[j] == datalist[i]:
                datalist.pop(j)
    '''
    strdatalist = []

    sdata = ""

    nn = 0

    dotfile = open("write.txt", "w")

    while len(datalist) > 0:

        data = datalist.pop(0)

        nn = nn+1

        # 将坐标转为字符串(x,y,v)

        sdata = sdata+"("+str(data[0])+","+str(data[1])+","+str(data[2])+")"

        #dotfile.write(sdata)

        if nn == NumofOnce or len(datalist) == 0:

            # 构造要发送的字符串数组

            strdatalist.append(sdata)
            dotfile.write(sdata)
            sdata = ""
            nn = 0

    dotfile.close()
    #strdatalist=["(1,1,255)"]  #for test
    strdatalist.append("(0,0,1)")

    return strdatalist


def dealy():
    im = Image.open(filename).convert('L')  # 打开图片，并处理成灰度图
    #print("get a picture,the info about filename, im.format, im.size, im.mode:")
    #print(filename, im.format, im.size, im.mode)
    x = im.size[0]  # 图片的长
    y = im.size[1]  # 图片的宽
    y = int(y*xsize/x)
    x = xsize
    return y


def NewWork(conn):

    datalist = waitfile(1, 40)
    countSent = 0
    for datastr in datalist:

        print("senddata:"+datastr)

        conn.publish('/control/laser/dat', datastr, qos=2)
        countSent = countSent + 1
        #time.sleep(0.002)
        print('sent %d times' %(countSent))







def on_message(conn, userdata, msg):
    widthy=dealy()
    widthx=xsize
    print(msg.topic+" "+str(msg.payload))
    if msg.payload == b'ready.':
        print('设备已启动')
        # 告诉设备，有新任务
        conn.publish('/control/laser/cmd', "1", qos=1)
        sleep(0.1)
        
    elif msg.payload == b'wait data.':
        print('等待接收数据')
        # 调用发送数据函数
        NewWork(conn)
        # 告诉设备，数据发送完
        #while msg.payload == b'wait data.':
        #    conn.publish('/control/laser/dat', "(0,0,1)", qos=2)
        print('all sent')
    elif msg.payload == b'all wrote.':
        print('数据写入完成')
        conn.publish('/control/laser/cmd', "2", qos=1)
        print('end command sent')
    elif msg.payload == b'received.':
        print('数据接收完成')
    elif msg.payload == b'done.':
        print('任务完成')
    elif msg.payload == b'input k:':
        conn.publish('/control/laser/cmd',3, qos=1)
        k=input('input k:')
        conn.publish('/control/laser/cmd',k, qos=1)
        print('完成')
    elif msg.payload == b'input x:':
        conn.publish('/control/laser/cmd',4, qos=1)
        conn.publish('/control/laser/cmd', widthx, qos=1)
    elif msg.payload == b'input y:':
        conn.publish('/control/laser/cmd',5, qos=1)
        conn.publish('/control/laser/cmd', widthy, qos=1)




'''

def on_message(conn):

    print('initiated')

    conn.publish('/control/laser/cmd', "1", qos=1)

    print('sending...')

    NewWork(conn)

    time.sleep(0.01)

    print('end')

    conn.publish('/control/laser/cmd', "2", qos=1)

'''



def on_connect(client, userdata, flags, rc):

    print("Connected with result code "+str(rc))





#waitfile("/home/dennis/Downloads/Tsinghua/laser_printer/final/image-2.jpg", 10, 60)  # 测试保存文件功能


def run():
    #widthy=dealy()
    #widthx=xsize

    client = mqtt.Client()

    client.on_connect = on_connect

    client.on_message = on_message

    client.connect("192.168.1.100", 1883, 30)

    client.subscribe('/values/laser/report', 1)

    client.loop_start()

if __name__ == "__main__":
    run()
