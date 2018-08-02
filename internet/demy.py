#!/usr/bin/env python
# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt_paho                                  #给paho.mqtt.client定义一个别名
from flask import Flask,request,send_from_directory,redirect          #从flask Web框架里导入flask,request...类
from werkzeug import secure_filename                                  #从werkzeug实用函数库导入secure_filname(上传文件)
import w                                                              #导入处理“mycraft”数据库文件的程序
import laser_wmx                                                          #导入处理激光扫描图片的程序


client = mqtt_paho.Client()                                             
flaskapp = Flask(__name__)
#app是Flask的实例，它接收包或者模块的名字作为参数，但一般都是传递__name__让函数通过传入这个名字确定程序的根目录，以便获得静态文件和模板文件的目录


@flaskapp.route('/static/<path:path>')                                 #/static/xxx的url处理（url是一个网页地址）
def send_static(path):
    return send_from_directory('static', path)                         #从static文件夹下返回结果文件

@flaskapp.route('/open/<string:door>')                                 #/open/xxx的url处理
def api_open(door):
    client.publish('/control/Warehouse/open', door, qos=1)             #给仓库发布命令开门
   

@flaskapp.route('/upload', methods=['POST'])                           #POST方式处理上传文件
def upload_file():                   
    if 'file1' in request.files:                                       #上传图片
        request.files['file1'].save('file1.jpg')                       #保存到文件file1.jpg
        try:
            laser_wmx.run()
        except e:
            print(e)
        return"图片文件上传成功，请返回"
    if 'file2' in request.files:                                       #上传数据
        request.files['file2'].save('file2.db')                        #保存到文件file2.db
        try:
            w.run()
        except e:
            print(e)
        return"数据库文件上传成功，请返回"                                               

def on_message(conn, userdata, msg):                                   #订阅消息处理
    print(msg.topic+" "+str(msg.payload))

client.on_message = on_message
client.connect("192.168.12.1", 1883, 60)                               #连接树莓派，mqtt端口1883

client.subscribe("/values/Warehouse/#", 1)                             #仓库订阅多层消息
client.loop_start()
flaskapp.run(host="0.0.0.0")                            
#执行app.run就可以启动服务了。默认Flask只监听虚拟机的本地127.0.0.1这个地址，端口为5000



'''
请在python代码旁边创建一个static文件夹
并将文件放到static文件夹下面，然后可以通过以下地址访问
http://127.0.0.1:5000/static/文件名

例如图片文件“081200.png”对应
http://127.0.0.1:5000/static/081200.png
网页文件“test.html”对应
http://127.0.0.1:5000/static/test.html

访问如下地址可以发送mqtt
http://127.0.0.1:5000/open/1
可以将链接指向该地址用于开门，例如：
<a href="/open/1">这是图片</a>

上传文件例子
http://127.0.0.1:5000/static/upload.html

下载文件例子
把minecraft.exe放到static下面
'''

