#include <Wire.h>

#define arm_pin 5
#define opener_pin 6
#define intake_pin 10
#define gripper_pin11

#define arm_neutral 150
#define arm_forward 200
#define arm_backward 100

#define opener_neutral 150
#define opener_forward 200
#define opener_backward 100

#define intake_neutral 150
#define intake_forward 200

#define gripper_open 150
#define gripper_close 200

typedef struct state{
    bool intake;
    bool gripper;
} state;

state current = {false, true};
state received = {false, true};

bool unfailsafed = true;
byte i;
byte temp0;
byte temp1;
bool stepper_direction = false;
byte step_cycle = 0;
unsigned long read_time;
unsigned long stepper_time;

void setup(){
    Wire.begin(8);           // join i2c bus with address #8
    Wire.onReceive(process); // register event
    Serial.begin(9600);

    pinMode(arm_pin, OUTPUT);
    pinMode(opener_pin, OUTPUT);
    pinMode(intake_pin, OUTPUT);
    pinMode(gripper_pin, OUTPUT);

    memset(dataTX,0,sizeof(dataTX));
    memset(dataRX,0,sizeof(dataRX));

    failsafe();
    read_time = millis();
    stepper_time = millis();
}

void loop(){
    if(steps > 0){
        if(millis() - stepper_time > 10){
            stepper_time = millis();

        }
    }
    

}

void process(){
    dataRX[0] = Wire.read();
    dataRX[1] = Wire.read();
    dataRX[2] = Wire.read();
    while (Wire.available() > 0){
        Wire.read();
    }

    current.intake = bitRead(dataRX[0],4);
    current.gripper = bitRead(dataRX[0],5);

    ///////////////////////////

   
    if(current.intake != received.intake){
        current.intake = received.intake;
        if(current.intake){
            
        }else{
            
        }
    }
    
    if(current.gripper != received.gripper){
        current.gripper = received.gripper;
        if(current.gripper){
            
        }else{
            
        }
    }

    if(bitRead(dataRX[0],0)){
        analogWrite(arm_pin, arm_forward); // arm up PWM
    } else if(bitRead(dataRX[0],1)){
        analogWrite(arm_pin, arm_backward); // arm down PWM
    } else
        analogWrite(arm_pin, arm_neutral); // arm neutral PWM
    }

    if(bitRead(dataRX[0],2)){
        analogWrite(opener_pin, opener_forward); // opener up PWM
    } else if(bitRead(dataRX[0],3)){
        analogWrite(opener_pin, opener_backward); // opener down PWM
    } else
        analogWrite(opener_pin, opener_neutral); // opener neutral PWM
    }

    if(dataRX[1] <= 10){
        Serial.write("#");
        Serial.write(dataRX[1]);
    }else if(dataRX[1] == 11){
        Serial.write("q");
    }

}

void transmit(){
    dataTX[11] = 0;
    for(i=1; i<11; i++){
        dataTX[11] += dataTX[i];
    }
    Serial.write(dataTX, 13);
    
}

void failsafe(){
    if(unfailsafed && millis() - read_time >= 500){
        analogWrite(arm_pin, arm_neutral);
        analogWrite(opener_pin, opener_neutral);
        analogWrite(intake_pin, intake_neutral);
        analogWrite(gripper_pin, gripper_close);
        unfailsafed = false;
    }
}