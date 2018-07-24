#!/usr/bin/env python  
# -*- coding: UTF-8 -*-

'''
File name:          laser_server_v2.py
Description:        项目激光部分服务器代码
Author:             秦智
Date:               June 4, 2018
Others:             
                    
'''



import threading
import time
import socketserver
from PIL import Image

class echorequestserver(socketserver.BaseRequestHandler):
    def handle(self):
        conn = self.request
        print('获得连接：', self.client_address)
        while True:
            client_data = conn.recv(100)
            client_data = client_data.decode('utf-8')
            print(client_data)
            if not client_data:
                print('断开连接')
                break
            elif client_data == "a":
                print("收到a")
                self.NewWork(conn)

    def NewWork(self,conn):
        #datalist = self.waitfile(20,30)
        datalist = self.waitfile(3,30)
        for datastr in datalist:
            print("senddata:"+datastr+"\r\n")
            #str = input("senddata:");
            conn.sendall(bytes(datastr, encoding='utf-8'))
            #time.sleep(1)
            client_data = conn.recv(20)
            #print("bug")
            client_data = client_data.decode('utf-8')
            print("receive:"+client_data)
            time.sleep(0.3)
            
            '''
            getdone = False
            while not getdone:
                client_data = conn.recv(1024)
                client_data = client_data.decode('utf-8')
                print("receive:"+client_data)
                if client_data == "done.":
                    getdone = True
            '''

    def NewWork_v0(self,conn):   ##加_0,_1或v0,v1的都是旧版本
        #datalist = self.waitfile(20,30)
        datalist = self.waitfile(3,30)
        conn.sendall(bytes("a", encoding='utf-8'))
        get1 = False
        while not get1:
            client_data = conn.recv(1024)
            client_data = client_data.decode('utf-8')
            if client_data =="1":
                get1 =True
        thend =0;
        for datastr in datalist:
            get2 = False
            while not get2:
                client_data = conn.recv(1024)
                client_data = client_data.decode('utf-8')
                if client_data =="2":
                    get2 =True
            conn.sendall(bytes(datastr, encoding='utf-8'))
            print("senddata:")
            print(datastr)
        conn.sendall(bytes("e", encoding='utf-8'))

    def waitfile(self,NumofOnce,minvalue):
        filename = self.waitfilename()
        im = Image.open(filename)
        print("get a picture,the info about filename, im.format, im.size, im.mode:\n")
        print(filename, im.format, im.size, im.mode,"\n")
        imL = im.convert("L")
        #imL = imL.resize((imL.size[0]//10,imL.size[1]//10))
        x = imL.size[0]
        y = imL.size[1]
        y=int(y*100/x)
        x=100
        imL=imL.resize((x,y))
        datalist = []
        for iy in range(0,y):
            for ix in range(0,x):
                value = imL.getpixel((ix,iy))
                if value >=minvalue:
                    datalist.append([ix,iy,value])
        strdatalist=[]
        sdata = ""
        nn=0
        while len(datalist)>0:
            data = datalist.pop(0)
            nn=nn+1
            sdata = sdata+"("+str(data[0])+","+str(data[1])+","+str(data[2])+")"
            if nn == NumofOnce or len(datalist)==0:
                strdatalist.append(sdata)
                #print(nn)
                #print(len(datalist))
                #print(sdata)
                sdata = ""
                nn = 0
        strdatalist.append("(0,0,0)")
        return strdatalist






        NumofTime = x*y//NumofOnce+1
        datalist=[]
        for j in range(1, NumofTime+1):
            datalist_s=""
            for k in range(1,NumofOnce+1):
                if (j-1)*NumofOnce+k <= x*y:
                    xx = ((j-1)*NumofOnce+k)//y
                    yy = ((j-1)*NumofOnce+k)%y
                    value = imL.getpixel((xx,yy))
                    if value >= minvalue:
                        datalist_s = datalist_s+"("+str(xx)+","+str(yy)+","+str(value)+")"
            datalist.append(datalist_s)
        return datalist

    def waitfile_v0(self,NumofOnce,minvalue):
        filename = self.waitfilename()
        im = Image.open(filename)
        print("get a picture,the info about filename, im.format, im.size, im.mode:\n")
        print(filename, im.format, im.size, im.mode,"\n")
        imL = im.convert("L")
        x = imL.size[0]
        y = imL.size[1]
        NumofTime = x*y//NumofOnce+1
        datalist=[]
        for j in range(1, NumofTime+1):
            datalist_s=""
            for k in range(1,NumofOnce+1):
                if (j-1)*NumofOnce+k <= x*y:
                    xx = ((j-1)*NumofOnce+k)//y
                    yy = ((j-1)*NumofOnce+k)%y
                    value = imL.getpixel((xx,yy))
                    if value >= minvalue:
                        datalist_s = datalist_s+"("+str(xx)+","+str(yy)+","+str(value)+")"
            datalist.append(datalist_s)
        return datalist

    def waitfilename(self):
        filename = "laser.jpg"
        exp=True   #####
        getname = True
        while not getname:
            if exp:
                getname = True
        return filename
            
if __name__ == '__main__':
    server =socketserver.TCPServer(("101.6.161.127", 12345),echorequestserver)  # 使用处理单连接的TCPServer
    print('服务端启动了...')
    server.serve_forever()





'''
获取某个像素位置的值：
im.getpixel((4,4))

写某个像素位置的值：
img.putpixel((4,4),(255,0,0))
'''