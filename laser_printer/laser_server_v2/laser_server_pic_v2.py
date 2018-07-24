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

class getlaserpic():

    def waitfile(self):
        filename = self.waitfilename()
        im = Image.open(filename)
        print("get a picture,the info about filename, im.format, im.size, im.mode:\n")
        print(filename, im.format, im.size, im.mode,"\n")
        imL = im.convert("1")    //转换为黑白图
        #imL = imL.resize((imL.size[0]//10,imL.size[1]//10))
        x = imL.size[0]
        y = imL.size[1]
        y=int(y*100/x)
        x=100
        imL=imL.resize((x,y))
        print((x,y))
        strdata = ""
        isfirst = True
        for iy in range(0,y):
            for ix in range(0,x):
                value = imL.getpixel((ix,iy))
                if value == 255:
                    str = "1"
                else:
                    if value == 0:
                        str = "0"
                    else:
                        print(value)
                strdata = strdata+str
        return strdata

    def waitfile_1(self,NumofOnce,minvalue):        ##加_0,_1或v0,v1的都是旧版本
        filename = self.waitfilename()
        im = Image.open(filename)
        print("get a picture,the info about filename, im.format, im.size, im.mode:\n")
        print(filename, im.format, im.size, im.mode,"\n")
        imL = im.convert("1")
        #imL = imL.resize((imL.size[0]//10,imL.size[1]//10))
        x = imL.size[0]
        y = imL.size[1]
        y=int(y*100/x)
        x=100
        imL=imL.resize((x,y))
        datalist = []
        for iy in range(0,y-1):
            for ix in range(0,x-1):
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



    def waitfilename(self):
        filename = "laser-6-4-2.JPG"
        exp=True   #####
        getname = True
        while not getname:
            if exp:
                getname = True
        return filename
            
if __name__ == '__main__':
    a=getlaserpic()
    strdata = a.waitfile()
    f = open("laser_pic.txt", "a")
    f.write( strdata )
    f.close()







'''
获取某个像素位置的值：
im.getpixel((4,4))

写某个像素位置的值：
img.putpixel((4,4),(255,0,0))
'''