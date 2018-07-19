//#include "stm32f10x.h"                  // Device header
#include "mbed.h"

Ticker ticker_step;
DigitalOut step(PB_0), dir(PB_1), en(PC_13);
volatile int remain;

double targetDistance = 30.0;										//目标距离（毫米）
double pitch = 3.6;															//螺距（毫米）
double circle = targetDistance / pitch;					//圈数=目标距离/螺距
double stepWidth = 1.8;													//步距角
double stepNumberPerCircle = 360.0 / stepWidth;	//单圈步数=一圈/步距角


void toggle_step() { //产生STEP脉冲信号
    if(remain == 0)
        return;
    if(step){
        step = 0; //STEP 1->0
        remain--;
    }else{
        step = 1; //STEP 0->1
    }
}

int main() {
    ticker_step.attach(&toggle_step, 0.0002);
    en = 0; //Enable stepper driver
    
    while (true) {
        dir = 0;
        remain = stepNumberPerCircle * circle;
        wait(1.5);
        dir = 1;
        remain = stepNumberPerCircle * circle;
        wait(1.5);
    }
}
