#include <TimerOne.h>

unsigned long high_start, high_end, average = 0;
int i = 0;

void setup() {
    Serial.begin(115200);
    analogReference(EXTERNAL);
    analogReadRes(16);
    pinMode(13, OUTPUT);

    //Timer1.initialize(500);
    //Timer1.attachInterrupt(analogSample);
    digitalWrite(13,HIGH);
    
}

void loop() {
    if(i >= 510){
        i = 0;
        Serial.println(0);
        Serial.println(65535);
    }
    i++;
    Serial.println(analogRead(14));
}
