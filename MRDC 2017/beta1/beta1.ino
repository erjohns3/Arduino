#include <SoftwareSerial.h>
#include <Wire.h>

#define relay0 8
#define jump1 10
#define compressor 11
#define motor_pin0 5
#define motor_pin1 6
#define motor_pin2 10
#define motor_pin3 11
#define max_pressure 100
#define min_pressure 80
#define adc 0
#define deadzone 20
#define neutral 361
#define SFX_TX 5
#define SFX_RX 6
#define reset_pin 11

boolean pressed[14];
boolean toggled[14];
int motor_config[4];
float full_speed[4];
int motor_pwm[4];
int count[4];
byte mapping[16];
byte controller[8];
byte past_controller[2];
byte dataRX[10];        // the incoming is 11 including the 255
byte dataTX[13];        // includes the 255

int LSX;
int LSY;
int RSX;
float trigger_diff;
float power;
boolean connection = true;
boolean check;
boolean resetTimer;
int pressure;
int pressAvg;
int pressNum;
byte i;
byte checkSumTX;
byte checkSumRX;
unsigned long read_time;
unsigned long reset_time;


void setup(){
    Serial.begin(9600);

    pinMode(reset_pin, INPUT);
    pinMode(12, OUTPUT);
    pinMode(2, OUTPUT);

    memset(controller,0,sizeof(controller));
    memset(past_controller,0,sizeof(past_controller));
    memset(pressed,0,sizeof(pressed));
    memset(toggled,0,sizeof(toggled));
    memset(dataTX,0,sizeof(dataTX));
    memset(full_speed,0,sizeof(full_speed));

    dataTX[0] = 255;
    dataTX[11] = 240;

    pressAvg = 0;
    pressNum = 0;
    connection = false;
    checkSumRX = 0;
    checkSumTX = 0;
    read_time = millis();
}

void loop(){
    encoders();
    pressure();
    failsafe();
}

void serialEvent(){
    while(Serial.available() >= 22){
        Serial.read();
    }
    
    while(Serial.available() >= 11){
        if((byte)Serial.read() == 255){
            check = true;
            Serial.readBytes(dataRX, 10);
            checkSumRX = 0;
            for(i=0; i<8; i++){
                checkSumRX += dataRX[i];
            }
            if(dataRX[8] != checkSumRX){
                check = false;
            }
            if(dataRX[9] != 240){
                check = false;
            }
            if(check){
                for(i=0; i<8; i++){
                    controller[i] = dataRX[i];
                }
                connection = true;
                process();
                transmit();
                read_time = millis();
            }
        }
    }
}

void process(){
    for(i=0; i<7; i++){
        pressed[i] = false;
        if(bitRead(controller[0],i) && !bitRead(past_controller[0],i)){
            toggled[i] = !toggled[i];
            pressed[i] = true;
        }
        pressed[i+7] = false;
        if(bitRead(controller[1],i) && !bitRead(past_controller[1],i)){
            toggled[i+7] = !toggled[i+7];
            pressed[i+7] = true;
        }
    }
    past_controller[0] = controller[0];
    past_controller[1] = controller[1];

    ///////////////////////////

    if(pressed[0]){
        if(toggled[0]){
            //analogWrite(, 550);
            //analogWrite(14, 550);
        }else{
            //analogWrite(12, 250);
            //analogWrite(14, 250);
        }
        
        bitWrite(dataTX[3], 5, toggled[0]);
    }
    if(pressed[1]){
        if(toggled[1]){
            //analogWrite(8, neutral+150);
            //analogWrite(10, neutral+150);
        }else{
            //analogWrite(8, neutral);
            //analogWrite(10, neutral);
        }
        bitWrite(dataTX[1], 5, toggled[1]);
    }
    if(pressed[3]){
        if(toggled[1]){
            //analogWrite(15, 300);
        }else{
            //analogWrite(15, 600);
        }
        bitWrite(dataTX[1], 6, toggled[3]);
    }
    

    //analogWrite(0, motor_pwm[0]);
    //analogWrite(1, motor_pwm[1]);
    //analogWrite(2, motor_pwm[2]);
    //analogWrite(3, motor_pwm[3]);

    /////////////////////////////////////////////////////////////////////////

    if(toggled[7]){
        if(pressure > max_pressure){
            //digitalWrite(compress, HIGH);
            bitClear(dataTX[1], 4);
        }else if(pressure < min_pressure){
            //digitalWrite(compress, LOW);
            bitSet(dataTX[1], 4);
        }
        bitSet(dataTX[1], 3);
    }else if(pressed[7]){
        //digitalWrite(compress, HIGH);
        bitClear(dataTX[1], 4);
        bitClear(dataTX[1], 3);
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
    if(connection && millis() - read_time >= 500){
        //analogWrite(0, neutral);
        //analogWrite(1, neutral);
        //analogWrite(2, neutral);
        //analogWrite(3, neutral);

        //analogWrite(4, neutral);
        //analogWrite(5, neutral);
        //analogWrite(6, neutral);

        //digitalWrite(7, LOW);
        connection = false;
    }
}

void encoders(int* count, byte* mapping){
    
    asm volatile(

    "mov r2, r26 \n\t"
    "mov r3, r27 \n\t"
    "mov r4, r18 \n\t"
    "mov r5, r19 \n\t"

    "sbr r26, 64 \n\t"
    "sbr r27, 0 \n\t"
    "clr r7 \n\t"
    "clr r8 \n\t"
    "clr r9 \n\t"
    "clr r10 \n\t"
    "clr r16 \n\t"
    "clr r17 \n\t"
    "clr r19 \n\t"

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r16, 0 \n\t"   // register for encoder 1

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r16, 2 \n\t"   // register for encoder 1

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 2

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder 2

    "LOOP:     \n\t"    // start loop

    "lsl r16 \n\t"
    "lsl r17 \n\t"

    "in r18, 0x05 \n\t"
    "bst r18, 2 \n\t"
    "bld r16, 0 \n\t"

    "in r18, 0x0B \n\t"
    "bst r18, 0 \n\t"
    "bld r16, 2 \n\t"

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 2

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder 2

    "andi r16, 0x0F \n\t"
    "andi r17, 0x0F \n\t"

    "mov r28, r22 \n\t"
    "mov r29, r23 \n\t"
    "add r28, r16 \n\t"
    "adc r29, r19 \n\t"
    
    "ldd r6, Y+0 \n\t"
    "add r7, r6 \n\t"

    "mov r28, r22 \n\t"
    "mov r29, r23 \n\t"
    "add r28, r17 \n\t"
    "adc r29, r19 \n\t"
    
    "ldd r6, Y+1 \n\t"
    "add r8, r6 \n\t"

    "subi r26, 1 \n\t"
    "sbc r27, r19 \n\t"
    
    "brne LOOP \n\t"    // end loop

    "mov r26, r2 \n\t"
    "mov r27, r3 \n\t"
    "mov r18, r4 \n\t"
    "mov r19, r5 \n\t"

    "mov r28, r24 \n\t"
    "mov r29, r25 \n\t"
    "std Y+0, r7 \n\t"
    "std Y+0, r8 \n\t"
    
    );
}

void pressure(){

    pressAvg += (0.966*analogRead(adc) - 52.076);
    pressNum++;
    if(pressNum >= 10){
        pressure = pressAvg / pressNum;
        dataTX[6] = pressure;
        pressAvg = 0;
        pressNum = 0;
    }
}
