from tkinter.filedialog import *
from tkinter import *
#from mqtt_laser import *

from time import sleep
import paho.mqtt.client as mqtt
import threading
import time
from PIL import Image

filename = None


def waitfile(NumofOnce, minvalue):
    im = Image.open(filename).convert('L')  # 打开图片，并处理成灰度图
    print("get a picture,the info about filename, im.format, im.size, im.mode:")
    print(filename, im.format, im.size, im.mode)
    imL = im.convert("L")
    x = imL.size[0]  # 图片的长
    y = imL.size[1]  # 图片的宽
    y = int(y*100/x)
    x = 100
    imL = imL.resize((x, y))  # 缩放图片
    datalist = []  # 存放坐标点的数组
    for iy in range(0, y):
        for ix in range(0, x):
            value = imL.getpixel((ix, iy))  # 获取像素亮度
            if value >= minvalue:  # 判断阈值
                datalist.append([ix, iy, value])
    strdatalist = []
    sdata = ""
    nn = 0
    dotfile = open("write.txt", "w")
    while len(datalist) > 0:
        data = datalist.pop(0)
        nn = nn+1
        # 将坐标转为字符串(x,y,v)
        sdata = sdata+"("+str(data[0])+","+str(data[1])+","+str(data[2])+")"
        dotfile.write(sdata)
        if nn == NumofOnce or len(datalist) == 0:
            # 构造要发送的字符串数组
            strdatalist.append(sdata)
            sdata = ""
            nn = 0
    dotfile.close()
    strdatalist.append("(0,0,0)")
    return strdatalist


def NewWork(conn):
    datalist = waitfile(5, 60)
    for datastr in datalist:
        print("senddata:"+datastr)
        conn.publish('/control/laser/dat', datastr, qos=2)
        # time.sleep(0.2)


def on_message(conn, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    if msg.payload == b'ready.':
        print('设备已启动')
        cmd.insert(END, '设备已启动\n')

        # 告诉设备，有新任务
        conn.publish('/control/laser/cmd', "1", qos=1)
    elif msg.payload == b'wait data.':
        print('等待接收数据')
        cmd.insert(END, '等待接收数据\n')

        # 调用发送数据函数
        NewWork(conn)
        # 告诉设备，数据发送完
        print('all sent')
        cmd.insert(END, 'all sent\n')

        time.sleep(1.0)
        conn.publish('/control/laser/cmd', "2", qos=1)
        print('end command sent')
        cmd.insert(END, 'end command sent\n')
    elif msg.payload == b'received.':
        print('数据接收完成')
        cmd.insert(END, '数据接收完成\n')

    elif msg.payload == b'done.':
        print('任务完成')
        cmd.insert(END, '任务完成\n')


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    cmd.insert(END, "Connected with result code "+str(rc)+'\n')

#


def openFile():
    filename = None
    filename = askopenfilename(initialdir='.')
    print(filename)
    # print(fileAddress)
    # ipAddressLable.get()
    #client.connect(str(ipAddress), 1883, 60)
    #client.subscribe('/values/laser/report', 1)
    client.loop_start()


# 初始化窗口对象
root = Tk()
# 设置标题
root.title("Laser Printer Server")
# 设置窗口大小
root.geometry('500x500')
# 初始化一个按钮，单击以打开文件
Button(root, text="Open", command=openFile).pack()

'''
#初始化一个文本输入框
ipAddress = StringVar()
ipAddressLable = Entry(root, textvariable = ipAddress)
ipAddress.set("input ip here")
ipAddressLable.pack()
'''

cmd = Text(root)
cmd.pack()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect('192.168.12.1', 1883, 60)
client.subscribe('/values/laser/report', 1)
client.loop()

root.mainloop()

# client.loop_start()
print('1')
while True:
    sleep(0.1)
