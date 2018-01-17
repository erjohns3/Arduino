#include <Wire.h>

#define motor_pin0 5
#define motor_pin1 6
#define motor_pin2 10
#define motor_pin3 11
#define adc_pin 0

typedef struct state{
    bool mecanum;
    bool gate;
    bool shooter;
    bool extender;
    bool compressor;
} state;

state current = {false, false, false, false, false};
state received = {false, false, false, false, false};

int encoder_count[4];
byte encoder_config[16];
byte dataRX[8];
byte dataTX[13];        // includes the 255
bool unfailsafed = true;
int pressure;
int pressAvg;
int pressNum;
byte i;
byte temp0;
byte temp1;
byte checkSumTX;
byte checkSumRX;
byte dataMCU2[3];
unsigned long read_time;


void setup(){
    Wire.begin();
    Serial.begin(9600);

    pinMode(motor_pin0, OUTPUT);
    pinMode(motor_pin1, OUTPUT);
    pinMode(motor_pin2, OUTPUT);
    pinMode(motor_pin3, OUTPUT);

    memset(mapping,0,sizeof(mapping));
    memset(count,0,sizeof(count));
    memset(dataTX,0,sizeof(dataTX));
    memset(dataRX,0,sizeof(dataRX));

    dataTX[0] = 255;
    dataTX[11] = 240;

    pressAvg = 0;
    pressNum = 0;
    checkSumRX = 0;
    failsafe();
    read_time = millis();
}

void loop(){
    //encodersRead();
    pressureRead();
    failsafe();
}

void serialEvent(){
    while(Serial.available() >= 22 && temp0 == 0){
        Serial.read();
    }

    temp1 = Serial.available();

    while(temp1 > 0){
        if(temp0 == 0){
            if((byte)Serial.read() == 255){
                temp0++;
            }
        }
        else if(temp0 < 9){
            dataRX[temp0-1] = (byte)Serial.read();
            checkSumRX += dataRX[temp0-1];
            temp0++;
        }
        else if(temp0 == 9){
            if((byte)Serial.read() == checkSumRX){
                temp0++;
            }else{
                temp0=0;
            }
            checkSumRX = 0;
        }
        else if(temp0 == 10){
            if((byte)Serial.read() == 240){
                unfailsafed = true;
                process();
                transmit();
                read_time = millis();
            }
            temp0=0;
        }
    }
}

void process(){
    current.mecanum = bitRead(dataRX[0],0);
    current.gate = bitRead(dataRX[0],1);
    current.shooter = bitRead(dataRX[0],2);
    current.extender = bitRead(dataRX[0],3);
    current.compressor = bitRead(dataRX[0],4);

    ///////////////////////////

    if(current.mecanum != received.mecanum){
        current.mecanum = received.mecanum;
        if(current.mecanum){
            
        }else{
            
        }
    }

    if(current.gate != received.gate){
        current.gate = received.gate;
        if(current.gate){
            
        }else{
            
        }
    }
    if(current.shooter != received.shooter){
        current.shooter = received.shooter;
        if(current.shooter){
            
        }else{
            
        }
    }
    if(current.extender != received.extender){
        current.extender = received.extender;
        if(current.extender){
            
        }else{
            
        }
    }

    if(current.compressor != received.compressor){
        current.compressor = received.compressor;
        if(current.compressor){
            
        }else{
            
        }
    }

    dataMCU2[0] = dataRX[1];
    dataMCU2[1] = dataRX[2];
    dataMCU2[2] = dataRX[3];

    Wire.beginTransmission(8);
    Wire.write(dataMCU2, 3);
    Wire.endTransmission();
    
    ///////////////////////////////////////////////////////////////

    analogWrite(motor_pin0, map(motor_pwm[0],0,200,32,62));
    analogWrite(motor_pin1, map(motor_pwm[1],0,200,32,62));
    analogWrite(motor_pin2, map(motor_pwm[2],0,200,32,62));
    analogWrite(motor_pin3, map(motor_pwm[3],0,200,32,62));

    /////////////////////////////////////////////////////////////////////////
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
        //analogWrite(0, neutral);
        //analogWrite(1, neutral);
        //analogWrite(2, neutral);
        //analogWrite(3, neutral);

        //analogWrite(4, neutral);
        //analogWrite(5, neutral);
        //analogWrite(6, neutral);

        //digitalWrite(7, LOW);
        unfailsafed = false;
    }
}

void encodersRead(int* encoder_count, byte* encoder_config){
    
    asm volatile(

    // save registers
    "mov r2, r26 \n\t"
    "mov r3, r27 \n\t"
    "mov r4, r18 \n\t"
    "mov r5, r19 \n\t"

    //////////////////////// start of encoder 1 and 2

    "sbr r26, 64 \n\t"
    "sbr r27, 0 \n\t"
    "clr r7 \n\t"
    "clr r8 \n\t"
    "clr r9 \n\t"
    "clr r10 \n\t"
    "clr r16 \n\t"
    "clr r17 \n\t"
    "clr r19 \n\t"

    /////////////////////////

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r16, 0 \n\t"   // register for encoder 1A

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r16, 2 \n\t"   // register for encoder 1B

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 2A

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder 2B

    ////////////////////////

    "LOOP:     \n\t"    // start loop

    "lsl r16 \n\t"
    "lsl r17 \n\t"

    ///////////////////////////////////

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

    ////////////////////////////////

    "andi r16, 0x0F \n\t"   // r16 holds encoder 1 config
    "andi r17, 0x0F \n\t"   // r17 holds encoder 2 config

    ///////////////////////////////

    "mov r28, r22 \n\t"     
    "mov r29, r23 \n\t"
    "add r28, r16 \n\t"
    "adc r29, r19 \n\t"     // r19 is zero
    
    "ldd r6, Y+0 \n\t"      // r6 holds the config value
    "add r7, r6 \n\t"       // r7 holds total count of encoder 1 low byte
    "adc r8, r19 \n\t"      // r7 holds total count of encoder 1 high byte
    "subi r7, 1 \n\t"       // subtract 1 low byte
    "sbc r8, r19 \n\t"      // subtract high byte

    //////////////////////////////

    "mov r28, r22 \n\t"
    "mov r29, r23 \n\t"
    "add r28, r17 \n\t"
    "adc r29, r19 \n\t"
    
    "ldd r6, Y+0 \n\t"      // r6 holds the config value
    "add r9, r6 \n\t"       // r8 holds total count of encoder 2 low byte
    "adc r10, r19 \n\t"     // r7 holds total count of encoder 1 low byte
    "subi r9, 1 \n\t"       // subtract 1 low byte
    "sbc r10, r19 \n\t"     // subtract high byte

    //////////////////////////////

    "subi r26, 1 \n\t"  // loop counter low byte
    "sbc r27, r19 \n\t" // loop counter high byte
    
    "brne LOOP \n\t"    // end loop

    "mov r28, r24 \n\t"
    "mov r29, r25 \n\t"
    "std Y+0, r7 \n\t"
    "std Y+1, r8 \n\t"
    "std Y+2, r9 \n\t"
    "std Y+3, r10 \n\t"

    ///////////////////////////// end of encoders 1 and 2

    ///////////////////////////// start of encoder 3 and 4

    "sbr r26, 64 \n\t"
    "sbr r27, 0 \n\t"
    "clr r7 \n\t"
    "clr r8 \n\t"
    "clr r9 \n\t"
    "clr r10 \n\t"
    "clr r16 \n\t"
    "clr r17 \n\t"
    "clr r19 \n\t"

    /////////////////////////

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r16, 0 \n\t"   // register for encoder 3A

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r16, 2 \n\t"   // register for encoder 3B

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 4A

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder 4B

    ////////////////////////

    "LOOP:     \n\t"    // start loop

    "lsl r16 \n\t"
    "lsl r17 \n\t"

    ///////////////////////////////////

    "in r18, 0x05 \n\t"
    "bst r18, 2 \n\t"
    "bld r16, 0 \n\t"

    "in r18, 0x0B \n\t"
    "bst r18, 0 \n\t"
    "bld r16, 2 \n\t"

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder

    ////////////////////////////////

    "andi r16, 0x0F \n\t"   // r16 holds encoder 1 config
    "andi r17, 0x0F \n\t"   // r17 holds encoder 2 config

    ///////////////////////////////

    "mov r28, r22 \n\t"     
    "mov r29, r23 \n\t"
    "add r28, r16 \n\t"
    "adc r29, r19 \n\t"     // r19 is zero
    
    "ldd r6, Y+0 \n\t"      // r6 holds the config value
    "add r7, r6 \n\t"       // r7 holds total count of encoder 1 low byte
    "adc r8, r19 \n\t"      // r7 holds total count of encoder 1 high byte
    "subi r7, 1 \n\t"       // subtract 1 low byte
    "sbc r8, r19 \n\t"      // subtract high byte

    //////////////////////////////

    "mov r28, r22 \n\t"
    "mov r29, r23 \n\t"
    "add r28, r17 \n\t"
    "adc r29, r19 \n\t"
    
    "ldd r6, Y+0 \n\t"      // r6 holds the config value
    "add r9, r6 \n\t"       // r8 holds total count of encoder 2 low byte
    "adc r10, r19 \n\t"     // r7 holds total count of encoder 1 low byte
    "subi r9, 1 \n\t"       // subtract 1 low byte
    "sbc r10, r19 \n\t"     // subtract high byte

    //////////////////////////////

    "subi r26, 1 \n\t"  // loop counter low byte
    "sbc r27, r19 \n\t" // loop counter high byte
    
    "brne LOOP \n\t"    // end loop

    "mov r28, r24 \n\t"
    "mov r29, r25 \n\t"
    "std Y+4, r7 \n\t"
    "std Y+5, r8 \n\t"
    "std Y+6, r9 \n\t"
    "std Y+7, r10 \n\t"

    ///////////////////////////// end of encoders 3 and 4

    // restore registers
    "mov r26, r2 \n\t"
    "mov r27, r3 \n\t"
    "mov r18, r4 \n\t"
    "mov r19, r5 \n\t"
    
    );
}

void pressureRead(){
    pressAvg += (0.966*analogRead(adc_pin) - 52.076);
    pressNum++;
    if(pressNum >= 10){
        pressure = pressAvg / pressNum;
        dataTX[6] = pressure;
        pressAvg = 0;
        pressNum = 0;
    }
}
