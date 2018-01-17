#include <Math.h>
#include <FastLED.h>

int i, c;
unsigned long power;
byte output_pin[5];

CRGB rgb;

void setup() {
    Serial.begin(115200);

    output_pin[0] = 3;
    output_pin[1] = 4;
    output_pin[2] = 5;
    output_pin[3] = 6;
    output_pin[4] = 9;

    for(i=0; i<8; i++){
        pinMode(output_pin[i], OUTPUT);
    }
}

void loop() {
    rgb.setHSV(0, 255, 255);
    analogWrite(output_pin[1], rgb.r);
    delay(500);
    analogWrite(output_pin[1], 511);
    delay(500);

}
