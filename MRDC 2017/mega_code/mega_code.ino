//Pin Values
#define motor0_pin 10     //Motor 1
#define motor1_pin 9      //Motor 2
#define motor2_pin 6      //Motor 3
#define motor3_pin 5      //Motor 4

#define extender_pin 1
#define traction_pin 2
#define shooter_pin 3
#define gripper_pin 4
#define compressor_pin 5

#define intake_pin 5        //pwm8
#define opener_pin 6        //pwm7
#define arm_pin 9           //pwm5

#define arm_up 42
#define arm_neutral 46
#define arm_down 50

#define opener_up 19
#define opener_neutral 22
#define opener_down 24

#define intake_forward 24
#define intake_neutral 22
#define intake_backward 20

#define stepper_step 14
#define stepper_dir 15
#define stepper_enable 16

#define time_out 200

bool stepper_dir_curr = false;
bool stepper_dir_prev = false;
int steps = 0;
bool step_status = true;

int checkSumRX = 0;         //check sum variable
byte temp0 = 0;
byte temp1 = 0;

int motor0Val = 0;
int motor1Val = 0;
int motor2Val = 0;
int motor3Val = 0;

byte dataRX[8];

unsigned long read_time;
unsigned long stepper_time; 

void setup() {
  //Begin Serial Connection to the XBee
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  //Begin and Initialize I2C Communication 
  Wire.begin();

  //Initialize GPIO Pins
  pinMode(motor0_pin, OUTPUT);
  pinMode(motor1_pin, OUTPUT);
  pinMode(motor2_pin, OUTPUT);
  pinMode(motor3_pin, OUTPUT);

  read_time = millis();
}

void loop() {
	fail_safe();
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

void process()
{    
  //Map from (-242,242) to (32,62) or (16,32)
  //(pwm1 and pwm2) 32 is full reverse, 47 is neutral, 62 is full forward
  //(pwm3 and pwm4) 16 is full reverse, 24 is neutral, 32 is full forward, 
  motor0Val = map(data[4],0,200,32,62);
  motor1Val = map(data[5],0,200,32,62);
  motor2Val = map(data[6],0,200,16,32);
  motor3Val = map(data[7],0,200,16,32);

  if(dataRX[0] & 0x02){
	  digitalWrite(extender_pin, LOW);
  }else{
	  digitalWrite(extender_pin, HIGH);
  }
  if(dataRX[0] & 0x04){
	  digitalWrite(traction_pin, LOW);
  }else{
	  digitalWrite(traction_pin, HIGH);
  }
  if(dataRX[0] & 0x08){
	  digitalWrite(shooter_pin, LOW);
  }else{
	  digitalWrite(shooter_pin, HIGH);
  }
  if(dataRX[0] & 0x10){
	  digitalWrite(gripper_pin, LOW);
  }else{
	  digitalWrite(gripper_pin, HIGH);
  }
  if(dataRX[0] & 0x40){
	  digitalWrite(compressor_pin, LOW);
  }else{
	  digitalWrite(compressor_pin, HIGH);
  }

  if(dataRX[1] & 0x04){
        if(dataRX[0] & 0x01){
            analogWrite(arm_pin, arm_up); // arm up PWM
        } else if(dataRX[1] & 0x02){
            analogWrite(arm_pin, arm_down); // arm down PWM
        } else{
            analogWrite(arm_pin, arm_neutral); // arm neutral PWM
        }
        analogWrite(opener_pin, opener_neutral);
        digitalWrite(stepper_enable, HIGH);
    }else{
        if(dataRX[1] & 0x01){
            analogWrite(opener_pin, opener_up); // opener up PWM
        } else if(dataRX[1] & 0x02){
            analogWrite(opener_pin, opener_down); // opener down PWM
        } else{
            analogWrite(opener_pin, opener_neutral); // opener neutral PWM
        }
        analogWrite(arm_pin, arm_neutral);
        digitalWrite(stepper_enable, LOW);
        steps = 0;
    }

    if(dataRX[0] & 0x08){
        analogWrite(intake_pin, intake_forward); // intake forward PWM
    } else if(dataRX[0] & 0x10){
        analogWrite(intake_pin, intake_backward); // intake backward PWM
    } else{
        analogWrite(intake_pin, intake_neutral); // intake neutral PWM
    }

    /////////////////////////////

    if((dataRX[2] & 0x0F) < 10){
        Serial.write("#");
		Serial.write((dataRX[2] & 0x0F) + 48);
		Serial.write("\n");
    }else if((dataRX[2] & 0x0F) == 10){
        Serial.write("#10\n");
    }else if((dataRX[2] & 0x0F) == 11){
        Serial.write("q");
    }

    ////////////////////////////

    if(dataRX[2] != 0){
        if(dataRX[2] & 0x80){
            digitalWrite(stepper_dir, HIGH);
            stepper_dir_curr = true;
        }else{
            digitalWrite(stepper_dir, LOW);
            stepper_dir_curr = false;
        }
        if(stepper_dir_curr != stepper_dir_prev){
            steps = (dataRX[2] & 0x7F) << 1;
        }else{
            steps += (dataRX[2] & 0x7F) << 1;
        }
        stepper_dir_prev = stepper_dir_curr;
    }
}

void send_feedback() {
  Serial.write(255);
  for(int i=0; i<11; i++)
  {
    Serial.write(0);
  }
  Serial.write(240);
}

void stepperControl(){
    if(steps > 0){
        if(micros() - stepper_time > 1000){
            stepper_time = micros();
            digitalWrite(stepper_step, step_status);
            step_status = !step_status;
            steps--;
        }
    }
}

void fail_safe()
{
	if(millis() - read_time > time_out){
		read_time = millis();
		analogWrite(motor0_pin, map(100,0,200,32,62));
		analogWrite(motor1_pin, map(100,0,200,32,62));
		analogWrite(motor2_pin, map(100,0,200,16,32));
		analogWrite(motor3_pin, map(100,0,200,16,32));
		digitalWrite(stepper_enable, LOW);
		digitalWrite(extender_pin, HIGH);
		digitalWrite(traction_pin, HIGH);
		digitalWrite(shooter_pin, HIGH);
		digitalWrite(gripper_pin, HIGH);
		digitalWrite(compressor_pin, HIGH);
	}
}
