
# coding: utf-8

# In[1]:


import sqlite3 as db
import numpy as np
from math import cos, sin, acos, asin, pi
from scipy.optimize import fsolve
# 长度单位 mm
# 角度单位 度


# In[2]:


fd = open("command.txt",'w')


# In[3]:


con = db.connect('craft.db')


# In[4]:


c = con.cursor()


# In[5]:


c.execute('select * from block')
rawdata = np.array(c.fetchall())


# In[6]:


rawdata_1 = np.delete(rawdata,[0,1],axis=1)


# In[7]:


rawdata_2 = rawdata_1[rawdata_1[:,3]>1,:]


# In[8]:


rawdata_3 = rawdata_2[rawdata_2[:,1]>1,:]


# In[9]:


print(rawdata_3)


# In[10]:


rawdata_3_T = rawdata_3.T


# In[11]:


rawdata_3_T_min = rawdata_3_T.min(axis=1)


# In[12]:


delete_min = np.tile(rawdata_3_T_min,(len(rawdata_3_T.T),1))
delete_min


# In[13]:


data = (rawdata_3_T -delete_min.T).T
data[:, [1, 2]] = data[:, [2, 1]]


# In[14]:


data = data[data[:,1].argsort()]
data = data[data[:,0].argsort()]
data = data[data[:,2].argsort()]
data = data[data[:,3].argsort()]


# In[15]:


data = data * [30,30,30,1]
data


# In[16]:


def fun1(x,*args):
    x0 = float(x[0])
    x1 = float(x[1])
    return [
        - args[0] + args[2] * cos(x0) + args[3] * cos(x1) ,
        - args[1] + args[2] * sin(x0) + args[3] * sin(x1)
    ]


# In[17]:


class roboarm:
    def __init__(self, robox,roboy,roboz, r1, r2):
        self.robox = robox
        self.roboy = roboy
        self.roboz = roboz
        self.r1 = r1;
        self.r2 = r2;
        self.whiteblocknum = 4;
        self.whiteblockarray = [[-4,15],[22,48],[-38,43],[-10,77]]
        self.blackblocknum = 4;
        self.blackblockarray = [[-81,50],[-109,16],[-113,77],[-141,43]]
        self.highmax = 150
        
    def readoxyz(self, ox, oy, oz):
        self.ox = ox
        self.oy = oy
        self.oz = oz
        
    def readnowxyz(self,nowx, nowy, nowz):
        self.nowx = nowx
        self.nowy = nowy
        self.nowz = nowz
        
    def readcarxyz(self,carx,cary,carz):        
        self.carx = carx
        self.cary = cary
        self.carz = carz
        
    def readproductxyz(self, prodectx, prodecty, prodectz):
        self.prodectx = prodectx
        self.prodecty = prodecty
        self.prodectz = prodectz
        
    def xytotheta(self, mode):
        farr =  np.zeros(shape=(100,3))
        for i in range(-5, 5):
            for j in range(-5, 5):
                result = fsolve(fun1, [i,j],args=(self.ox - self.robox,self.oy - self.roboy,self.r1,self.r2))
                a = float(result[0]) / pi * 180.0
                b = float(result[1]) / pi * 180.0
                a = a % 360
                b = b % 360
                farr[(i+5)*10+(j+5)] = [a,b,a + b]
        farr=farr[farr[:,2].argsort()]
        print(farr[mode][0], farr[mode][1])
        return farr[mode][0], farr[mode][1]
        
    def cal(self, bx,by,bz):
        self.readoxyz(self.nowx, self.nowy,self.nowz)
        self.nowtheta1, self.nowtheta2 = self.xytotheta(0)
        self.nowthetah = self.oz - self.roboz
#         print(self.nowtheta1,self.nowtheta2,self.nowthetah)
        self.readoxyz(bx, by, bz)
        self.theta1, self.theta2 = self.xytotheta(0)
        self.thetah = self.oz - self.roboz
#         print(self.theta1,self.theta2,self.thetah)
        dtheta1 = self.theta1 - self.nowtheta1
        dtheta2 = self.theta2 - self.nowtheta2
        dthetah = self.thetah - self.nowthetah
        return dtheta1,dtheta2,dthetah
    
    def move(self,bx,by,bz):    
        move1, move2, moveh = self.cal(bx,by,bz)
        print(move1,move2,moveh)
        fd.write("/control/robo/x {}\n".format(move1))
        fd.write("/control/robo/y {}\n".format(move2))
        fd.write("/control/robo/z {}\n".format(moveh))
        fd.write("/control/robo/a {}\n".format(-(move1+move2)))
        fd.write("/control/robo/do 1\n")
        self.readnowxyz(bx,by,bz)
        
    def getblock(self, btype):
        if btype == 0:
            self.whiteblocknum = self.whiteblocknum - 1;
            print(" block:",self.whiteblockarray[self.whiteblocknum][0], self.whiteblockarray[self.whiteblocknum][1])
            return self.whiteblockarray[self.whiteblocknum][0] + self.carx, self.whiteblockarray[self.whiteblocknum][1] + self.cary
        if btype == 1:
            self.blackblocknum = self.blackblocknum - 1;
            print(" block:",self.blackblockarray[self.blackblocknum][0], self.blackblockarray[self.blackblocknum][1])
            return self.blackblockarray[self.blackblocknum][0] + self.carx, self.blackblockarray[self.blackblocknum][1] + self.cary            
    
    def getblockstate(self):
        if self.whiteblocknum == 0 or self.whiteblocknum == 0:
            self.whiteblocknum = 4 
            self.blackblocknum = 4
            return 0
        else:
            return 1
    
    def getprodectxyz(self, blockx, blocky, blockz):
        return self.prodectx + blockx, self.prodecty + blocky, self.prodectz + blockz
    
    def openair(self):
        fd.write("/control/robo/vacumn 1\n")
    
    def closeair(self):
        fd.write("/control/robo/vacumn 0\n")
    
    def getblock2(self):
        if self.whiteblocknum == 4:
            return 0
        elif self.blackblocknum == 4:
            return 1


# In[18]:


class WAREHOUSE:
    def __init__(self):
        self.placestate = 0
        self.airstate =0
        self.high= [60,60]
        self.trushhigh = 0
    def gowhite(self):
        if self.placestate == 0:
            self.placestate = 0
        elif self.placestate == 1:
            fd.write("/control/warehouse/do 2\n")
            self.placestate = 0
        elif self.placestate ==2:
            fd.write("/control/warehouse/do 4\n")
            self.placestate = 0
        
    def goblack(self):
        if self.placestate == 0:
            fd.write("/control/warehouse/do 1\n")
            self.placestate = 1
        elif self.placestate == 1:
            self.placestate = 1
        elif self.placestate ==2:
            fd.write("/control/warehouse/do 2\n")
            self.placestate = 1
            
    def gotrush(self):
        if self.placestate == 0:
            fd.write("/control/warehouse/do 3\n")
            self.placestate = 2
        elif self.placestate == 1:
            fd.write("/control/warehouse/do 1\n")
            self.placestate = 2
        elif self.placestate ==2:       
            self.placestate = 2  
    
    def waretocar(self,btype):
#        if self.airstate == 0:
#      ？   fd.write("/control/warehouse/high {}\n".format(self.high[btype]))
        fd.write("/control/warehouse/do 5\n")
        self.high[btype] = self.high[btype] - 30
#            self.airstate = 1
#        elif self.airstate ==1:
#            self.airstate = 1

    def cartoware(self):
 #       if self.airstate == 0:
  #          self.airstate = 0
   #     elif self.airstate ==1:
#         fd.write("/control/warehouse/high {}\n".format(self.trushhigh))
        fd.write("/control/warehouse/do 6\n")
        self.trushhigh = self.trushhigh + 30
         #   self.airstate = 0


# In[19]:


class CAR:
    def __init__(self):
        self.carstate = 0
        
    def gowhite(self):
        if self.carstate == 0:
            self.carstate = 0
        elif self.carstate == 1:
            fd.write("/control/car/do 6\n")
            self.carstate = 0
        elif self.carstate == 2:
            fd.write("/control/car/do 3\n")
            self.carstate = 0
    
    def goblack(self):
        if self.carstate == 0:
            fd.write("/control/car/do 1\n")
            self.carstate = 1
        elif self.carstate == 1:
            self.carstate = 1
        elif self.carstate == 2:
            fd.write("/control/car/do 4\n")
            self.carstate = 1
            
    def gorobo(self):
        if self.carstate == 0:
            fd.write("/control/car/do 5\n")
            self.carstate = 2
        elif self.carstate == 1:
            fd.write("/control/car/do 2\n")
            self.carstate = 2
        elif self.carstate == 2:
            self.carstate = 2


# In[20]:


my = roboarm(0, 0, 0, 200, 200)


# In[21]:


my.readnowxyz(0,400,185)


# In[22]:


my.readcarxyz(0,267.5,88)


# In[23]:


my.readproductxyz(-150,150,0)


# In[24]:


car = CAR()
warehouse = WAREHOUSE()


# In[25]:


print("robo init")
print("car init")
print("warehouse init")


# In[26]:


warehouse.waretocar(0)
car.goblack()
warehouse.goblack()
warehouse.waretocar(1)
car.gorobo()


# In[27]:


for i in data:
    if my.getblockstate() == 0:
        if my.getblock2() == 0:
            car.gowhite()
            warehouse.gotrush()
            warehouse.cartoware()
            warehouse.gowhite()
            warehouse.waretocar(0)
        elif my.getblock2 == 1:
            car.goblack()
            warehouse.gotrush()
            warehouse.cartoware()
            warehouse.goblack()
            warehouse.waretocar(1)
        car.gorobo()
    else:
        print(1)
        blockx, blocky = my.getblock(i[3])
        oblockx , oblocky, oblockz = my.getprodectxyz(-i[0],i[1],i[2]+30)
        print(blockx, blocky ,my.highmax)
        my.move(blockx, blocky ,my.highmax)
        print(blockx, blocky ,my.carz)
        my.move(blockx, blocky ,my.carz)
        my.openair()
        print(blockx, blocky ,my.highmax)
        my.move(blockx, blocky ,my.highmax)
        print(oblockx,oblocky,my.highmax)
        my.move(oblockx,oblocky,my.highmax)
        print(oblockx,oblocky,oblockz)
        my.move(oblockx,oblocky,oblockz)
        my.closeair()
        print(oblockx,oblocky,my.highmax)
        my.move(oblockx,oblocky,my.highmax)


# In[28]:


car.gowhite()
warehouse.gotrush()
warehouse.cartoware()
car.goblack()
warehouse.cartoware()


# In[29]:


fd.close()

