
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

boolean pressed[14];
boolean toggled[14];
byte feedback[10];
int motor_config[4];
float full_speed[4];
int motor_pwm[4];
byte controller[8];
byte past_controller[2];
byte data[8];

int LSX;
int LSY;
int RSX;
float trigger_diff;
float power;
boolean connection;
int pressure;
int pressAvg;
int pressNum;
byte temp0;
byte temp1;
byte i;
byte checkSumTX;
byte checkSumRX;
unsigned long read_time;


void setup(){
    Serial.begin(9600);

    memset(controller,0,sizeof(controller));
    memset(past_controller,0,sizeof(past_controller));
    memset(pressed,0,sizeof(pressed));
    memset(toggled,0,sizeof(toggled));
    memset(feedback,0,sizeof(feedback));
    memset(full_speed,0,sizeof(full_speed));

    full_speed[0] =  (131*(1.0) - deadzone) / 100;
    full_speed[1] =  (131*(1.0) - deadzone) / 100;
    full_speed[2] =  (131*(1.0) - deadzone) / 100;
    full_speed[3] =  (131*(1.0) - deadzone) / 100;

    pressAvg = 0;
    pressNum = 0;
    connection = false;
    read_time = millis();
    checkSumRX = 0;
    checkSumTX = 0;
    temp0 = 0;
    temp1 = 0;
    failsafe();
}

void loop(){
    receive();
    //process();
    //transmit();    
}

void receive(){
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
            data[temp0-1] = (byte)Serial.read();
            checkSumRX += data[temp0-1];
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
                for(i=0; i<8; i++){
                    controller[i] = data[i];
                }
                connection = true;
                process();
                transmit();
                read_time = millis();
            }
            temp0=0;
        }
    }
    
    if(connection && millis() - read_time >= 500){
        connection = false;
        failsafe();
    }
}

void process(){
    if(connection){
        i = 0;
        temp0 = 1;
        temp1 = 0;
        while(i<14){
            pressed[i] = false;
            if(controller[temp1]&temp0 && !past_controller[temp1]&temp0){
                toggled[i] = !toggled[i];
                pressed[i] = true;
            }
            i++;
            temp0 = temp0<<1;
            if(i == 7){
                temp0 = 1;
                temp1 = 1;
            }
        }
        for(i=0; i<2; i++){
            past_controller[i] = controller[i];
        }

        ///////////////////////////

        if(pressed[0]){
            if(toggled[0]){
                //analogWrite(, 550);
                //analogWrite(14, 550);
            }else{
                //analogWrite(12, 250);
                //analogWrite(14, 250);
            }
            bitWrite(feedback[2], 5, toggled[0]);
        }
        if(pressed[1]){
            if(toggled[1]){
                //analogWrite(8, neutral+150);
                //analogWrite(10, neutral+150);
            }else{
                //analogWrite(8, neutral);
                //analogWrite(10, neutral);
            }
            bitWrite(feedback[0], 5, toggled[1]);
        }
        if(pressed[3]){
            if(toggled[1]){
                //analogWrite(15, 300);
            }else{
                //analogWrite(15, 600);
            }
            bitWrite(feedback[0], 6, toggled[3]);
        }

        ////////////////////////////////////////////////////////////////////

        trigger_diff = (controller[7] - controller[6])/100;
        bitClear(feedback[1], 6);
        bitSet(feedback[1], 5);

        if(toggled[0]){
            LSX = controller[2] - 100;
            if(trigger_diff < -1){
                power = 0.41 - 0.21*trigger_diff;
                bitClear(feedback[1], 5);
            }else if(trigger_diff > 1){
                power = 0.41 + 0.59*trigger_diff; 
                bitSet(feedback[1], 6);
                bitClear(feedback[1], 5);
            }
        }else{
            LSX = 0;
            if(trigger_diff < -1){
                power = 0.5 - 0.3*trigger_diff;
                bitClear(feedback[1], 5);
            }else if(trigger_diff > 1){
                power = 0.5 + 0.5*trigger_diff; 
                bitSet(feedback[1], 6);
                bitClear(feedback[1], 5);
            }
        }
        LSY = controller[3] - 100;
        RSX = controller[4] - 100;

        /*
        int dps = gyro.read();
        int yaw = dps[1];
        RSX = RSX + feedback1*(RSX - feedback2*yaw);
        */

        motor_config[0] = -LSY - LSX - RSX;
        motor_config[1] = LSY - LSX - RSX;
        motor_config[2] = -LSY + LSX - RSX;
        motor_config[3] = LSY + LSX - RSX;

        for(i=0; i<4; i++){

            motor_pwm[i] = neutral;

            if(motor_config[i] > 1){
                motor_pwm[i] += motor_config[i]*full_speed[i]*power;
                motor_pwm[i] += deadzone;
            }else if(motor_config[i] < -1){
                motor_pwm[i] += motor_config[i]*full_speed[i]*power;
                motor_pwm[i] -= deadzone;
            }

            if(motor_pwm[i] > neutral + deadzone + 131){
                motor_pwm[i] = neutral + deadzone + 131;
            }else if(motor_pwm[i] < neutral - deadzone - 131){
                motor_pwm[i] = neutral - deadzone - 131;
            }
            feedback[6+i] = int((2*(i%2)-1)*motor_config[i] + 100);
        }
        
        //analogWrite(0, motor_pwm[0]);
        //analogWrite(1, motor_pwm[1]);
        //analogWrite(2, motor_pwm[2]);
        //analogWrite(3, motor_pwm[3]);

        /////////////////////////////////////////////////////////////////////////

        if(toggled[7]){
            if(pressure > max_pressure){
                //digitalWrite(compress, HIGH);
                bitClear(feedback[0], 4);
            }else if(pressure < min_pressure){
                //digitalWrite(compress, LOW);
                bitSet(feedback[0], 4);
            }
            bitSet(feedback[0], 3);
        }else if(pressed[7]){
            //digitalWrite(compress, HIGH);
            bitClear(feedback[0], 4);
            bitClear(feedback[0], 3);
        }
    }
}

void transmit(){
    Serial.write(255);
    checkSumTX = 0;
    for(i=0; i<10; i++){
        Serial.write(feedback[i]);
        checkSumTX += feedback[i];
    }
    Serial.write(checkSumTX);
    Serial.write(240);
}

void failsafe(){
    //analogWrite(0, neutral);
    //analogWrite(1, neutral);
    //analogWrite(2, neutral);
    //analogWrite(3, neutral);

    //analogWrite(4, neutral);
    //analogWrite(5, neutral);
    //analogWrite(6, neutral);

    //digitalWrite(7, LOW);
}

void autonomy(){
    pressAvg += (0.966*analogRead(adc) - 52.076);
    pressNum++;
    if(pressNum >= 10){
        pressure = pressAvg / pressNum;
        feedback[5] = pressure;
        pressAvg = 0;
        pressNum = 0;
    }
}
