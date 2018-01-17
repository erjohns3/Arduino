#include <Wire.h>

#define MCP23008_ADDRESS 0x20     //Address
#define MCP23008_IODIR 0x00       //Registers
#define MCP23008_GPIO 0x09

#define motor_pin0 5
#define motor_pin1 6
#define motor_pin2 10
#define motor_pin3 11

#define MCU2_ADDRESS 0x23
#define accel_address
#define gyro_address

#define adc_pin 0

byte motor_pwm[4];
int encoder_count[4];
byte encoder_config[16];
byte dataRX[8];
byte dataTX[12];        // includes the 255
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

    begin_gpio_extender();
    //begin_gyro();
    //begin_accel();    //TODO make this

    TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz (16.38ms period) (pins 5 and 6)
    TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz (8.16ms period)  (pins 9 and 10)
    TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz (8.16ms period) (pins 3 and 11)

    pinMode(motor_pin0, OUTPUT);
    pinMode(motor_pin1, OUTPUT);
    pinMode(motor_pin2, OUTPUT);
    pinMode(motor_pin3, OUTPUT);

    memset(motor_pwm,0,sizeof(motor_pwm));
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
    encoderRead();
    motorControl();
    //pressureRead();
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
        temp1--;
    }
}

void process(){

    Wire.beginTransmission(MCP23008_ADDRESS);
    Wire.write(MCP23008_GPIO);
    Wire.write(~(dataRX[0] & 0x1F));
    Wire.endTransmission();

    dataMCU2[0] = dataRX[1];
    dataMCU2[1] = dataRX[2];
    dataMCU2[2] = dataRX[3];

    Wire.beginTransmission(MCU2_ADDRESS);
    Wire.write(dataMCU2, 3);
    Wire.endTransmission();

    motor_pwm[0] = map(dataRX[4],0,200,32,62);
    motor_pwm[1] = map(dataRX[5],0,200,32,62);
    motor_pwm[2] = map(dataRX[6],0,200,16,32);
    motor_pwm[3] = map(dataRX[7],0,200,16,32);

    analogWrite(motor_pin0, motor_pwm[0]);
    analogWrite(motor_pin1, motor_pwm[1]);
    analogWrite(motor_pin2, motor_pwm[2]);
    analogWrite(motor_pin3, motor_pwm[3]);

    /////////////////////////////////////////////////////////////////////////
}

void transmit(){
    dataTX[1] = pressure;
    dataTX[2] = lowByte(count[0]);
    dataTX[3] = highByte(count[0]);
    dataTX[4] = lowByte(count[1]);
    dataTX[5] = highByte(count[1]);
    dataTX[6] = lowByte(count[2]);
    dataTX[7] = highByte(count[2]);
    dataTX[8] = lowByte(count[3]);
    dataTX[9] = highByte(count[3]);
    dataTX[10] = 0;
    for(i=1; i<10; i++){
        dataTX[10] += dataTX[i];
    }
    Serial.write(dataTX, 12);
    
}

void failsafe(){
    if(unfailsafed && millis() - read_time >= 500){

        Wire.beginTransmission(MCP23008_ADDRESS);
        Wire.write(MCP23008_GPIO);
        Wire.write(0xFF);
        Wire.endTransmission();

        dataMCU2[0] = 32;
        dataMCU2[1] = 10;
        dataMCU2[2] = 0;

        Wire.beginTransmission(MCU2_ADDRESS);
        Wire.write(dataMCU2, 3);
        Wire.endTransmission();

        analogWrite(motor_pin0, map(100,0,200,32,62));
        analogWrite(motor_pin1, map(100,0,200,32,62));
        analogWrite(motor_pin2, map(100,0,200,16,32));
        analogWrite(motor_pin3, map(100,0,200,16,32));

        unfailsafed = false;
    }
}

void motorControl(){
    
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

    "in r18, 0x03 \n\t" // port B
    "bst r18, 0 \n\t"   // pin number
    "bld r16, 0 \n\t"   // register for encoder 1A

    "in r18, 0x09 \n\t" // port D
    "bst r18, 2 \n\t"   //pin number
    "bld r16, 2 \n\t"   // register for encoder 1B

    "in r18, 0x09 \n\t" // port D
    "bst r18, 4 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 2A

    "in r18, 0x09 \n\t" // port D
    "bst r18, 7 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder 2B

    ////////////////////////

    "LOOP:     \n\t"    // start loop

    "lsl r16 \n\t"
    "lsl r17 \n\t"

    ///////////////////////////////////

    "in r18, 0x03 \n\t"
    "bst r18, 0 \n\t"
    "bld r16, 0 \n\t"

    "in r18, 0x09 \n\t"
    "bst r18, 2 \n\t"
    "bld r16, 2 \n\t"

    "in r18, 0x09 \n\t" // port B
    "bst r18, 4 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 2

    "in r18, 0x09 \n\t" // port D
    "bst r18, 7 \n\t"   //pin number
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

    "in r18, 0x06 \n\t" // port C
    "bst r18, 1 \n\t"   // pin number
    "bld r16, 0 \n\t"   // register for encoder 3A

    "in r18, 0x06 \n\t" // port C
    "bst r18, 0 \n\t"   //pin number
    "bld r16, 2 \n\t"   // register for encoder 3B

    "in r18, 0x06 \n\t" // port C
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 4A

    "in r18, 0x06 \n\t" // port C
    "bst r18, 3 \n\t"   //pin number
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

void begin_gpio_extender(){
  // set defaults!
  Wire.beginTransmission(MCP23008_ADDRESS);
  Wire.write((byte)MCP23008_IODIR);
  Wire.write((byte)0x00);  // all outputs (1 == input)
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);  
  Wire.endTransmission();
}

void begin_gyro()
{
  /* Reset then switch to normal mode and enable all three channels */
  //write8(GYRO_REGISTER_CTRL_REG1, 0x00);
  //write8(GYRO_REGISTER_CTRL_REG1, 0x0F);

  /* Adjust resolution --- (adafruit library is weird, check datasheet if you want to change this) */
  //write8(GYRO_REGISTER_CTRL_REG4, 0x00);   //250 dps == 00 (default)   500 dps == 01, 2000 dps == 1x   
  //return true;
}

void begin_accel()
{
  
}