

```python
import sqlite3 as db
import numpy as np
from math import cos, sin, acos, asin, pi
from scipy.optimize import fsolve
# 长度单位 mm
# 角度单位 度
```


```python
fd = open("command.txt",'w')
```


```python
con = db.connect('craft.db')
```


```python
c = con.cursor()
```


```python
c.execute('select * from block')
rawdata = np.array(c.fetchall())
```


```python
rawdata_1 = np.delete(rawdata,[0,1],axis=1)
```


```python
rawdata_2 = rawdata_1[rawdata_1[:,3]>1,:]
```


```python
rawdata_3 = rawdata_2[rawdata_2[:,1]>1,:]
```


```python
print(rawdata_3)
```

    [[ 1  2 -1  2]
     [ 1  3 -1  2]
     [ 2  4 -1  3]
     [ 1  4  0  3]
     [ 0  4 -1  3]
     [ 1  4 -2  3]
     [ 1  4 -1  2]
     [ 1  5 -1  3]]
    


```python
rawdata_3_T = rawdata_3.T
```


```python
rawdata_3_T_min = rawdata_3_T.min(axis=1)
```


```python
delete_min = np.tile(rawdata_3_T_min,(len(rawdata_3_T.T),1))
delete_min
```




    array([[ 0,  2, -2,  2],
           [ 0,  2, -2,  2],
           [ 0,  2, -2,  2],
           [ 0,  2, -2,  2],
           [ 0,  2, -2,  2],
           [ 0,  2, -2,  2],
           [ 0,  2, -2,  2],
           [ 0,  2, -2,  2]])




```python
data = (rawdata_3_T -delete_min.T).T
data[:, [1, 2]] = data[:, [2, 1]]
```


```python
data = data[data[:,1].argsort()]
data = data[data[:,0].argsort()]
data = data[data[:,2].argsort()]
data = data[data[:,3].argsort()]
```


```python
data = data * [30,30,30,1]
data
```




    array([[30, 30,  0,  0],
           [30, 30, 30,  0],
           [30, 30, 60,  0],
           [ 0, 30, 60,  1],
           [30,  0, 60,  1],
           [30, 60, 60,  1],
           [60, 30, 60,  1],
           [30, 30, 90,  1]])




```python
def fun1(x,*args):
    x0 = float(x[0])
    x1 = float(x[1])
    return [
        - args[0] + args[2] * cos(x0) + args[3] * cos(x1) ,
        - args[1] + args[2] * sin(x0) + args[3] * sin(x1)
    ]
```


```python
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
```


```python
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
```


```python
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
```


```python
my = roboarm(0, 0, 0, 200, 200)
```


```python
my.readnowxyz(0,400,185)
```


```python
my.readcarxyz(0,267.5,88)
```


```python
my.readproductxyz(-150,150,0)
```


```python
car = CAR()
warehouse = WAREHOUSE()
```


```python
print("robo init")
print("car init")
print("warehouse init")
```

    robo init
    car init
    warehouse init
    


```python
warehouse.waretocar(0)
car.goblack()
warehouse.goblack()
warehouse.waretocar(1)
car.gorobo()
```


```python
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
```

    1
     block: -10 77
    -10 344.5 150
    89.99997943865745 90.00002056131427
    61.160840355041955 122.16454105279928
    -28.839139083615493 32.16452049148501 -35
    -10 344.5 88
    61.160840355041955 122.16454105279928
    61.160840355041955 122.16454105279928
    0.0 0.0 -62
    -10 344.5 150
    61.160840355041955 122.16454105279928
    61.160840355041955 122.16454105279928
    0.0 0.0 62
    -180 180 150
    61.160840355041955 122.16454105279928
    185.47639132353288 84.52360818424131
    124.31555096849092 -37.64093286855797 0
    -180 180 30
    185.47639132353288 84.52360818424131
    185.47639132353288 84.52360818424131
    0.0 0.0 -120
    -180 180 150
    185.47639132353288 84.52360818424131
    

    C:\Users\hukq1\Anaconda3\lib\site-packages\scipy\optimize\minpack.py:161: RuntimeWarning: The iteration is not making good progress, as measured by the 
      improvement from the last ten iterations.
      warnings.warn(msg, RuntimeWarning)
    

    185.47639132353288 84.52360818424131
    0.0 0.0 120
    1
     block: -38 43
    -38 310.5 150
    185.47639132353288 84.52360818424131
    135.52946295999755 58.425231234552484
    -49.946928363535335 -26.098376949688827 0
    -38 310.5 88
    135.52946295999755 58.425231234552484
    135.52946295999755 58.425231234552484
    0.0 0.0 -62
    -38 310.5 150
    135.52946295999755 58.425231234552484
    135.52946295999755 58.425231234552484
    0.0 0.0 62
    -180 180 150
    135.52946295999755 58.425231234552484
    185.47639132353288 84.52360818424131
    49.946928363535335 26.098376949688827 0
    -180 180 60
    185.47639132353288 84.52360818424131
    185.47639132353288 84.52360818424131
    0.0 0.0 -90
    -180 180 150
    185.47639132353288 84.52360818424131
    185.47639132353288 84.52360818424131
    0.0 0.0 90
    1
     block: 22 48
    22 315.5 150
    185.47639132353288 84.52360818424131
    123.76346316571231 48.25891344422975
    -61.71292815782057 -36.264694740011564 0
    22 315.5 88
    123.76346316571231 48.25891344422975
    123.76346316571231 48.25891344422975
    0.0 0.0 -62
    22 315.5 150
    123.76346316571231 48.25891344422975
    123.76346316571231 48.25891344422975
    0.0 0.0 62
    -180 180 150
    123.76346316571231 48.25891344422975
    185.47639132353288 84.52360818424131
    61.71292815782057 36.264694740011564 0
    -180 180 90
    185.47639132353288 84.52360818424131
    185.47639132353288 84.52360818424131
    0.0 0.0 -60
    -180 180 150
    185.47639132353288 84.52360818424131
    185.47639132353288 84.52360818424131
    0.0 0.0 60
    1
     block: -141 43
    -141 310.5 150
    185.47639132353288 84.52360818424131
    145.93434565467237 82.91184843739893
    -39.54204566886051 -1.611759746842381 0
    -141 310.5 88
    145.93434565467237 82.91184843739893
    145.93434565467237 82.91184843739893
    0.0 0.0 -62
    -141 310.5 150
    145.93434565467237 82.91184843739893
    145.93434565467237 82.91184843739893
    0.0 0.0 62
    -150 180 150
    145.93434565467237 82.91184843739893
    75.66288573829115 183.94825379239228
    -70.27145991638122 101.03640535499335 0
    -150 180 90
    75.66288573829115 183.94825379239228
    75.66288573829115 183.94825379239228
    0.0 0.0 -60
    -150 180 150
    75.66288573829115 183.94825379239228
    75.66288573829115 183.94825379239228
    0.0 0.0 60
    1
     block: -113 77
    -113 344.5 150
    75.66288573829115 183.94825379239228
    133.14782542796456 83.17235441331286
    57.48493968967341 -100.77589937907942 0
    -113 344.5 88
    133.14782542796456 83.17235441331286
    133.14782542796456 83.17235441331286
    0.0 0.0 -62
    -113 344.5 150
    133.14782542796456 83.17235441331286
    133.14782542796456 83.17235441331286
    0.0 0.0 62
    -180 150 150
    133.14782542796456 83.17235441331286
    86.05174444264412 194.33711301095173
    -47.09608098532044 111.16475859763887 0
    -180 150 90
    86.05174444264412 194.33711301095173
    86.05174444264412 194.33711301095173
    0.0 0.0 -60
    -180 150 150
    86.05174444264412 194.33711301095173
    86.05174444264412 194.33711301095173
    0.0 0.0 60
    1
     block: -109 16
    -109 283.5 150
    86.05174444264412 194.33711301095173
    70.43595158674907 151.6255258225683
    -15.615792855895052 -42.71158718838342 0
    -109 283.5 88
    70.43595158674907 151.6255258225683
    70.43595158674907 151.6255258225683
    0.0 0.0 -62
    -109 283.5 150
    70.43595158674907 151.6255258225683
    70.43595158674907 151.6255258225683
    0.0 0.0 62
    -180 210 150
    70.43595158674907 151.6255258225683
    84.34754949305352 176.85503964593954
    13.91159790630445 25.229513823371235 0
    -180 210 90
    84.34754949305352 176.85503964593954
    84.34754949305352 176.85503964593954
    0.0 0.0 -60
    -180 210 150
    84.34754949305352 176.85503964593954
    84.34754949305352 176.85503964593954
    0.0 0.0 60
    1
     block: -81 50
    -81 317.5 150
    84.34754949305352 176.85503964593954
    139.30975690477135 69.31405449586237
    54.96220741171783 -107.54098515007718 0
    -81 317.5 88
    139.30975690477135 69.31405449586237
    139.30975690477135 69.31405449586237
    0.0 0.0 -62
    -81 317.5 150
    139.30975690477135 69.31405449586237
    139.30975690477135 69.31405449586237
    0.0 0.0 62
    -210 180 150
    139.30975690477135 69.31405449586237
    185.65245001424773 93.14496014325414
    46.342693109476386 23.830905647391774 0
    -210 180 90
    185.65245001424773 93.14496014325414
    185.65245001424773 93.14496014325414
    0.0 0.0 -60
    -210 180 150
    185.65245001424773 93.14496014325414
    185.65245001424773 93.14496014325414
    0.0 0.0 60
    1
     block: -141 43
    -141 310.5 150
    185.65245001424773 93.14496014325414
    145.93434565467237 82.91184843739893
    -39.718104359575364 -10.233111705855208 0
    -141 310.5 88
    145.93434565467237 82.91184843739893
    145.93434565467237 82.91184843739893
    0.0 0.0 -62
    -141 310.5 150
    145.93434565467237 82.91184843739893
    145.93434565467237 82.91184843739893
    0.0 0.0 62
    -180 180 150
    145.93434565467237 82.91184843739893
    185.47639132353288 84.52360818424131
    39.54204566886051 1.611759746842381 0
    -180 180 120
    185.47639132353288 84.52360818424131
    185.47639132353288 84.52360818424131
    0.0 0.0 -30
    -180 180 150
    185.47639132353288 84.52360818424131
    185.47639132353288 84.52360818424131
    0.0 0.0 30
    


```python
car.gowhite()
warehouse.gotrush()
warehouse.cartoware()
car.goblack()
warehouse.cartoware()
```


```python
fd.close()
```
