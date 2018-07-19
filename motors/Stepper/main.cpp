#include "mbed.h"

Ticker ticker_step;
DigitalOut step(PB_0), dir(PB_1), en(PC_13);
volatile int remain;

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
        remain = 3200;
        wait(1.5);
        dir = 1;
        remain = 3200;
        wait(1.5);
    }
}
