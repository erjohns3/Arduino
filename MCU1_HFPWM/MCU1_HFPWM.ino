//#define DEBUG 1   //Debug (uncomment this line to enable serial monitor debugging)

//I2C
#include <Wire.h>                 //Library
#define MCU2_ADDRESS 0x23            // MCU2 Address TODO CHANGE THIS
#define MCP23008_ADDRESS 0x20     //GPIO Extender Address and Registers
// registers
#define MCP23008_IODIR 0x00
#define MCP23008_GPIO 0x09
//PWM Module
#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x6
#define PWM_MODULE_ADDRESS 0x40

#define BUFF_SIZE 12    //Size of packet (only the data, not including start byte, checksum, or end byte)
#define TIME_OUT_ms  200  //number of milliseconds to timeout after
#define t0_scale 16 //this is actually defined by the TCCR0B register (base 64 prescaler, at 1024 now)

//Pin Values
//const int pwm1 = 10;     //Motor 1
//const int pwm2 = 9;      //Motor 2
//const int pwm3 = 6;      //Motor 3
//const int pwm4 = 5;      //Motor 4
const int pwmDisable = 10;   //Disable PWM

//Receive varialbes
byte buf[BUFF_SIZE] = {0};  //packet buffer
byte data[BUFF_SIZE] = {0}; //good data
byte dataTX[1] = {0};
int idx = 0;               //index into buffer
int recv_error = 0;         //return value from receive (1 = new packet, 0 = timeout condition)
int checkSumRX = 0;         //check sum variable
int checkSumTX = 0; 
int bytesAvailable = 0;     //store # of Serial Bytes on the buffer

///time out varialbes
int TO_error = 0;     //return value from timeout check
unsigned long last_recv = 0;  //last received value
int TIME_OUT = 13; //TIME_OUT_ms / t0_scale;  //adjust for millis() having scaled output

//Control Output Values
int pwm1Val = 0;
int pwm2Val = 0;
int pwm3Val = 0;
int pwm4Val = 0;
byte relays = 0x00;

void setup() {
  //Set Timers for PWM Frequencies
  TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz (16.38ms period) (pins 5 and 6)
  TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz (8.16ms period)  (pins 9 and 10)
  TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz (8.16ms period) (pins 3 and 11)

  //Begin Serial Connection to the XBee
  Serial.begin(9600);

  //Begin and Initialize I2C Communication 
  Wire.begin();
  begin_gpio_extender();
  pwmModuleBegin();
  pwmModuleSetPWMFreq(250);    //4 ms -> 1024, 1536, 2048

  pinMode(pwmDisable, OUTPUT);
  digitalWrite(pwmDisable, LOW);
}

void loop() {
  //#ifdef DEBUG
  //Serial.println("Start of Loop");
  //#endif
  
  recv_error = receive();               //returns 0 with no new packet, 1 with one
  TO_error = timeout_check(recv_error); //returns 0 for failsafe, 1 for all good
  if(recv_error && TO_error) {          //only process if new packet and not fail safe
    //digitalWrite(pwmDisable,LOW);
    process();       
    send_feedback();
    commandPWM();
    commandI2C();                            //always send the command
  }
}

int receive() {
  while (Serial.available() >= 30)
  {
    #ifdef DEBUG
    Serial.write("Purge\n");
    #endif
    Serial.read();
  }
  bytesAvailable = Serial.available();
  while (bytesAvailable--)        //avoid being stuck
  {
    byte byte_rx = (byte)Serial.read();
    if (idx == 0)
    {
      if (byte_rx == 0xFF)
      {
        checkSumRX = 0;   //reset checksum value
        idx++;
        #ifdef DEBUG
        Serial.write("Start\n");
        #endif
      }
      else
      {
        //discard bytes that are lost
        #ifdef DEBUG
        Serial.write("discard\n");
        #endif
      }
      continue;
    }
    else if (idx < BUFF_SIZE+1)
    {
      buf[idx - 1] = byte_rx;
      checkSumRX += buf[idx - 1];
      idx++;
      continue;
    }
    else if (idx == BUFF_SIZE+1)
    {
      if (byte_rx == (byte)checkSumRX)
      {
        #ifdef DEBUG 
        Serial.write("correct checksum\n");
        #endif
        idx++;  //all that is left is the end byte
      }
      else
      {
        #ifdef DEBUG 
        Serial.write("wrong checksum\n");
        #endif
        idx = 0;  //checksum error
      }
      continue;
    }
    else //idx = 10
    {
      idx = 0;
      if (byte_rx == 0xF0) //everything is beautiful, update the controller data and exit
      {
        #ifdef DEBUG 
        Serial.write("full packet\n");
        #endif
        //copy buffer to data array
        for(int i=0; i<BUFF_SIZE; i++)
        {
          data[i] = buf[i];
        }
        return 1;       //0 means we have a new packet that is intact
      }
      else              //something went wrong, go back to the first index and keep looking for a full packet
      {
        #ifdef DEBUG 
        Serial.write("fail10\n");
        #endif
        continue;
      }
    }
  }
  return 0; //timeout condition (no more bytes available)
}

//Convert Raw Button Values into Robot Controls
void process()
{     
  //Map from (-242,242) to (205,410)
  pwm1Val = data[4] + (data[5]<<6);
  pwm2Val = data[6] + (data[7]<<6);
  pwm3Val = data[8] + (data[9]<<6);
  pwm4Val = data[10] + (data[11]<<6);

  //Relay Outputs (active low)
  //bits (4)      (3)      (2)      (1)    (0)
  relays = ~(data[0]);    //active low
}

void send_feedback() {
  checkSumTX = 0;
  Serial.write(255);
  for(int i=0; i<1; i++)
  {
    Serial.write(dataTX[i]);
    checkSumTX += dataTX[i];
  }
  Serial.write(checkSumTX);
  Serial.write(240);
}

int mapValues(int input, int inLow, int inHigh, int outLow, int outHigh)
{
  int inputRange = inHigh - inLow;
  int outputRange = outHigh - outLow;
  int inDiff = input - inLow;
  float outPercent = (float) inDiff / inputRange * outputRange;
  int outDiff = (int) outPercent;
  return  outDiff + outLow;
}

int timeout_check(int recv_error)
{
  if(recv_error){              //if we received a full packet
    last_recv = millis() - last_recv;
    if(last_recv > 255){
      dataTX[0] = 255;
    }else{
      dataTX[0] = (byte)last_recv;
    }
    last_recv = millis();         //update the last recieved time
  }
  if ((millis() - last_recv) > (TIME_OUT) )
  {
    fail_safe();
    return 0;
  }
  else
    return 1;
}

void fail_safe()
{
  //set all values to failsafe values
  pwm1Val = 1536;
  pwm2Val = 1536;
  pwm3Val = 1536;
  pwm4Val = 1536;
  setPWM(0,0,pwm1Val);
  setPWM(1,0,pwm2Val);
  setPWM(2,0,pwm3Val);
  setPWM(3,0,pwm4Val);
  //digitalWrite(pwmDisable,HIGH);
  write_gpio_extender(0xFF);                         //all relays disengaged
  Wire.beginTransmission((byte)MCU2_ADDRESS);
  Wire.write(0);
  Wire.write(11);     
  Wire.write(0);         
  Wire.endTransmission();
}

void commandI2C()
{
  write_gpio_extender(relays);
  write_MCU2();
}

void commandPWM()
{
  setPWM(0,0,pwm1Val);
  setPWM(1,0,pwm2Val);
  setPWM(2,0,pwm3Val);
  setPWM(3,0,pwm4Val); 
}

void begin_gpio_extender()
{
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

void write_gpio_extender(byte data)
{
  write8(MCP23008_GPIO, data);
}

void write8(uint8_t addr, uint8_t data) {
  Wire.beginTransmission((byte)MCP23008_ADDRESS);
  Wire.write((byte)addr);
  Wire.write((byte)data);
  Wire.endTransmission();
}

void write_MCU2()
{
  Wire.beginTransmission((byte)MCU2_ADDRESS);
  Wire.write((byte)data[1]);         
  Wire.write((byte)data[2]);     
  Wire.write((byte)data[3]);         
  Wire.endTransmission();
}

void pwmModuleBegin()
{
  write8PWM(PCA9685_MODE1, 0x0);
}

void pwmModuleSetPWMFreq(float freq) 
{
  freq *= 0.9;  // Correct for overshoot in the frequency setting (see issue #11).
  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;
  uint8_t prescale = floor(prescaleval + 0.5);
  
  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep
  write8PWM(PCA9685_MODE1, newmode); // go to sleep
  write8PWM(PCA9685_PRESCALE, prescale); // set the prescaler
  write8PWM(PCA9685_MODE1, oldmode);
  delay(5);
  write8PWM(PCA9685_MODE1, oldmode | 0xa1);  //  This sets the MODE1 register to turn on auto increment.
                                          // This is why the beginTransmission below was not working.
}


void setPWM(uint8_t num, uint16_t on, uint16_t off) {
  Wire.beginTransmission(PWM_MODULE_ADDRESS);
  Wire.write(LED0_ON_L+4*num);
  Wire.write(on);
  Wire.write(on>>8);
  Wire.write(off);
  Wire.write(off>>8);
  Wire.endTransmission();
}

uint8_t read8(uint8_t addr) {
  Wire.beginTransmission(PWM_MODULE_ADDRESS);
  Wire.write(addr);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)PWM_MODULE_ADDRESS, (uint8_t)1);
  return Wire.read();
}

void write8PWM(uint8_t addr, uint8_t d) {
  Wire.beginTransmission(PWM_MODULE_ADDRESS);
  Wire.write(addr);
  Wire.write(d);
  Wire.endTransmission();
}


