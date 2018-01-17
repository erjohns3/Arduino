#include <Audio.h>
#include <Math.h>
#include <FastLED.h>

AudioInputAnalog adc(14);
AudioAnalyzeFFT1024 fft;
//AudioAnalyzePeak peak;
AudioConnection patchCord1(adc, fft);
//AudioConnection patchCord1(adc, peak);

#define HISTORY_SIZE 32

int i, j, dropoff_num, min_brightness, max_brightness, value_num;
double value, brightness_multiplier, value_avg, value_max, value_cover, tmp;
byte output_pin[4];
double dropoff_LUT[128], value_history[HISTORY_SIZE];
int hue, x;

CRGB rgb;

void setup() {
    Serial.begin(115200);

	// Configure the window algorithm to use
	AudioMemory(32);

    min_brightness = 0;
    max_brightness = 255;
    brightness_multiplier = (max_brightness - min_brightness);

    value_max = 1.0;
    value_cover = 0;

    dropoff_num = 0;

    for(i=0; i<128; i++){
        dropoff_LUT[i] = pow(i,3)*0.00005;
    }

    output_pin[0] = 3;
    output_pin[1] = 4;
    output_pin[2] = 5;
    output_pin[3] = 6;

    hue = 0;
    value = 255;

    for(i=0; i<4; i++){
        pinMode(output_pin[i], OUTPUT);
    }
}

void loop() {


    if(fft.available()){

        note = 0;
        intensity = 0;
        for(i=1; i<=128; i++){
            note += fft.output[i]*log(i);
            intensity += fft.output[i];
        }
        note = note / intensity;
        intensity = intensity * rgb_multiplier;
    }
    
    if(peak.available()){

        //value = peak.read();
        /*
        if(value >= value_cover){
            value_cover = value;
            dropoff_num = 0;
        }else{
            value_cover = value_cover - dropoff_LUT[dropoff_num];
            dropoff_num = min(128, dropoff_num + 1);
        }
        */

        value_history[value_num] = peak.read();
        value_num = (value_num+1)%HISTORY_SIZE;

        tmp = 0;
        for(i=0; i<HISTORY_SIZE; i++){
            tmp += value_history[i];
        }
        value_cover = tmp / HISTORY_SIZE;

        if(value_cover >= value_max){
            value_max = value_cover;
        }else{
            value_max = max(value_max - 0.0004, 0.10);
        }

        /*
        Serial.print(value);
        Serial.print("\t");
        Serial.print(value_max);
        Serial.print("\t");
        Serial.println(value_cover);
        */

        value = min(255, min_brightness + (brightness_multiplier*value_cover/value_max));
        hue = (int(floor(millis()/20))) % 256;

        rgb.setHSV(hue, 255, value);
        //rgb.setHSV(192, 255, 255);

        analogWrite(output_pin[1], rgb.r);
        analogWrite(output_pin[2], rgb.g);
        analogWrite(output_pin[3], rgb.b);
    }
    
}

