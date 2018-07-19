#include <mbed.h>

class stepperMotor
{
  public:
    DigitalOut step;
    DigitalOut dir;
    DigitalOut en;
    int remain;
    stepperMotor(PinName stepPin, PinName dirPin, PinName enPin):
    step(stepPin),dir(dirPin),en(enPin), remain(0){;}
    void init();
    Ticker stepper;
};

DigitalIn limitSwitchX(PA_11);
DigitalIn limitSwitchY(PA_15);
DigitalIn limitSwitchZ(PB_1);

stepperMotor axisX(PA_7, PB_4, PB_6);
stepperMotor axisY(PB_0, PB_7, PB_9);
stepperMotor axisZ(PA_6, PA_12, PB_3);

void moveStepper()
{
    if(axisX.remain != 0 && axisX.step)
    {
        axisX.step = 0; //STEP 1->0
        axisX.remain--;
    }
    else if(axisX.remain != 0)
    {
        axisX.step = 1; //STEP 0->1
    }
		
    if(axisY.remain != 0 && axisY.step)
    {
        axisY.step = 0; //STEP 1->0
        axisY.remain--;
    }
    else if(axisY.remain != 0)
    {
        axisY.step = 1; //STEP 0->1
    }
		
		if(axisZ.remain != 0 && axisZ.step)
    {
        axisZ.step = 0; //STEP 1->0
        axisZ.remain--;
    }
    else if(axisZ.remain != 0)
    {
        axisZ.step = 1; //STEP 0->1
    }
}

void moveStepperX()
{
    if(axisX.remain == 0)
        return;
    if(axisX.step)
    {
        axisX.step = 0; //STEP 1->0
        axisX.remain--;
    }
    else
    {
        axisX.step = 1; //STEP 0->1
    }
}

void moveStepperY()
{
    if(axisY.remain == 0)
        return;
    if(axisY.step)
    {
        axisY.step = 0; //STEP 1->0
        axisY.remain--;
    }
    else
    {
        axisY.step = 1; //STEP 0->1
    }
}

void moveStepperZ()
{
    if(axisZ.remain == 0)
        return;
    if(axisZ.step)
    {
        axisZ.step = 0; //STEP 1->0
        axisZ.remain--;
    }
    else
    {
        axisZ.step = 1; //STEP 0->1
    }
}

void stepperMotor::init()
{
    remain = 0;
    en = 0;
    dir = 0;
    stepper.attach(&moveStepper, 0.0002);
}

void setup()
{
    axisX.init();
    axisY.init();
    axisZ.init();
}

void initX()
{
		axisX.dir = 0;
		axisX.remain = 300000;
		while(limitSwitchX.read()==0)
				;
		axisX.remain = 0;
		axisX.dir = 1;
		axisX.remain = 200;
}

void initY()
{
		axisY.dir = 0;
		axisY.remain = 300000;
		while(limitSwitchY.read()==0)
				;
		axisY.remain = 0;
		axisY.dir = 1;
		axisY.remain = 300;
}

void initZ()
{
		axisZ.dir = 0;
		axisZ.remain = 300000;
		while(limitSwitchZ.read()==0)
				;
		axisZ.remain = 0;
		axisZ.dir = 1;
		axisZ.remain = 3000;
}

int main()
{
    setup();
		initX();
		initY();
		initZ();
		while(1)
				;
    return 0;
}
