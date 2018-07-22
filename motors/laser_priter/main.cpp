#include <mbed.h>

class stepperMotor //定义了一个步进电机类
{
  public:                                                          //公共对象
    DigitalOut step;                                               //驱动板step引脚对象
    DigitalOut dir;                                                //驱动板dir引脚对象
    DigitalOut en;                                                 //驱动板en引脚对象
    int remain;                                                    //步进电机对象剩余步数
    stepperMotor(PinName stepPin, PinName dirPin, PinName enPin) : //构造函数，在声明对象时调用，输入三个引脚名称
                                                                   step(stepPin), dir(dirPin), en(enPin), remain(0)
    {
        ;
    }               //初始化三个引脚对象，并设定剩余步数为0
    void init();    //初始化函数，函数定义见后
    Ticker stepper; //内部时钟中断对象
};
//定义了三个输入引脚对象
DigitalIn limitSwitchX(PA_11);
DigitalIn limitSwitchY(PA_15);
DigitalIn limitSwitchZ(PB_1);

//定义了三个步进电机对象
stepperMotor axisX(PA_7, PB_4, PB_6);
stepperMotor axisY(PB_0, PB_7, PB_9);
stepperMotor axisZ(PA_6, PA_12, PB_3);

void moveStepper()                       //步进电机移动函数，在内部时钟中断中被引用
{                                        //判断逻辑请自行理解
    if (axisX.remain != 0 && axisX.step) //
    {
        axisX.step = 0; //STEP 1->0
        axisX.remain--;
    }
    else if (axisX.remain != 0) //
    {
        axisX.step = 1; //STEP 0->1
    }

    if (axisY.remain != 0 && axisY.step) //
    {
        axisY.step = 0; //STEP 1->0
        axisY.remain--;
    }
    else if (axisY.remain != 0) //
    {
        axisY.step = 1; //STEP 0->1
    }

    if (axisZ.remain != 0 && axisZ.step) //
    {
        axisZ.step = 0; //STEP 1->0
        axisZ.remain--;
    }
    else if (axisZ.remain != 0) //
    {
        axisZ.step = 1; //STEP 0->1
    }
}

void stepperMotor::init() //步进电机初始化函数
{
    remain = 0;
    en = 0;                               //en脚低电平使能
    dir = 0;                              //初始化转动方向
    stepper.attach(&moveStepper, 0.0002); //声明引用的函数
}

void setup() //设置函数，用于整个程序的初始化
{
    //调用三个步进电机对象的初始化函数
    axisX.init();
    axisY.init();
    axisZ.init();
}

void initX() //x方向归零动作
{
    axisX.dir = 0;                   //设定转向
    axisX.remain = 300000;           //设定足够大的转动步数
    while (limitSwitchX.read() == 0) //当x方向行程开关未触发时等待
        ;
    axisX.remain = 0;   //触发后停转
    axisX.dir = 1;      //反向
    axisX.remain = 200; //转动一个小距离松开行程开关
}
//y，z方向归零动作类似
void initY()
{
    axisY.dir = 0;
    axisY.remain = 300000;
    while (limitSwitchY.read() == 0)
        ;
    axisY.remain = 0;
    axisY.dir = 1;
    axisY.remain = 300;
}

void initZ()
{
    axisZ.dir = 0;
    axisZ.remain = 300000;
    while (limitSwitchZ.read() == 0)
        ;
    axisZ.remain = 0;
    axisZ.dir = 1;
    axisZ.remain = 3000;
}

int main() //主函数
{
    setup(); //函数整体初始化
             //xyz方向归零
    initX();
    initY();
    initZ();
    while (1)
        ; //归零结束后进入死循环保证稳定
    return 0;
}
