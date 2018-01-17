#include <Wire.h>

#define MCU2_ADDRESS 0x23

#define arm_pin 5
#define opener_pin 6
#define intake_pin 10
#define gripper_pin 11

#define arm_neutral 150
#define arm_up 200
#define arm_down 100

#define opener_neutral 150
#define opener_up 200
#define opener_down 100

#define intake_neutral 150
#define intake_forward 200

#define gripper_open 150
#define gripper_close 200

bool unfailsafed = true;
byte i;
byte dataRX[3];
byte temp = 0;
bool stepper_forward_curr = true;
bool stepper_forward_prev = true;
int steps = 0;
int step_cycle = 0;
byte step_config[4];
unsigned long read_time;
unsigned long stepper_time;

void setup(){
    Wire.begin(MCU2_ADDRESS);
    Wire.onReceive(receiveEvent);
    Serial.begin(9600);

    pinMode(arm_pin, OUTPUT);
    pinMode(opener_pin, OUTPUT);
    pinMode(intake_pin, OUTPUT);
    pinMode(gripper_pin, OUTPUT);

    memset(dataRX,0,sizeof(dataRX));
    /*
    * The sequence of control signals for 4 control wires is as follows:
    *
    * Step C0 C1 C2 C3
    *    1  1  0  1  0
    *    2  0  1  1  0
    *    3  0  1  0  1
    *    4  1  0  0  1
    */
    step_config[0] = 0x0A;
    step_config[1] = 0x06;
    step_config[2] = 0x05;
    step_config[3] = 0x09;

    DDRC = DDRC | 0x0F;

    failsafe();
    read_time = millis();
    stepper_time = millis();
}

void loop(){
    stepperControl();
    failsafe();
}

void receiveEvent(int x){
    dataRX[0] = Wire.read();
    dataRX[1] = Wire.read();
    dataRX[2] = Wire.read();
    while (Wire.available() > 0){
        Wire.read();
    }

    ///////////////////////////

    if(dataRX[0] & 0x01){
        analogWrite(arm_pin, arm_up); // arm up PWM
    } else if(dataRX[0] & 0x02){
        analogWrite(arm_pin, arm_down); // arm down PWM
    } else{
        analogWrite(arm_pin, arm_neutral); // arm neutral PWM
    }

    if(dataRX[0] & 0x04){
        analogWrite(opener_pin, opener_up); // opener up PWM
    } else if(dataRX[0] & 0x08){
        analogWrite(opener_pin, opener_down); // opener down PWM
    } else{
        analogWrite(opener_pin, opener_neutral); // opener neutral PWM
    }

    if(dataRX[0] & 0x10){
        analogWrite(intake_pin, intake_forward); // arm up PWM
    } else{
        analogWrite(intake_pin, intake_neutral); // arm neutral PWM
    }

    if(dataRX[0] & 0x20){
        analogWrite(gripper_pin, gripper_close); // arm up PWM
    } else{
        analogWrite(gripper_pin, gripper_open); // arm neutral PWM
    }

    /////////////////////////////

    if(dataRX[1] <= 10){
        Serial.write("#");
        Serial.write(dataRX[1]);
        Serial.write("\n");
    }else if(dataRX[1] == 11){
        Serial.write("q");
    }

    ////////////////////////////

    if(dataRX != 0){
        if(dataRX[2] & 0x80){
            stepper_forward_curr = false;
        }else{
            stepper_forward_prev = true;
        }
        if(stepper_forward_curr != stepper_forward_prev){
            steps = dataRX[2] & 0x7F;
        }else{
            steps += dataRX[2] & 0x7F;
        }
        stepper_forward_prev = stepper_forward_curr;
    }

    unfailsafed = true;
    read_time = millis();
}

void stepperControl(){
    if(steps > 0){
        if(millis() - stepper_time > 10){
            stepper_time = millis();
            temp = PORTC & 0x30;
            PORTC = temp | step_config[step_cycle];
            if(stepper_forward_curr){
                step_cycle++;
            }else{
                step_cycle--;
            }
            if(step_cycle < 0){
                step_cycle = 3;
            } else if(step_cycle > 3){
                step_cycle = 0;
            }
            steps--;
        }
    }
}

void failsafe(){
    if(unfailsafed && millis() - read_time >= 500){
        analogWrite(arm_pin, arm_neutral);
        analogWrite(opener_pin, opener_neutral);
        analogWrite(intake_pin, intake_neutral);
        analogWrite(gripper_pin, gripper_close);
        steps = 0;
        Serial.write("q");
        unfailsafed = false;
    }
}