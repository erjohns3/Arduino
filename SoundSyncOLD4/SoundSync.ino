#include <Audio.h>
#include <Math.h>
#include <FastLED.h>

#define UV_START 0
#define UV_END 4
#define RGB_START 5
#define RGB_END 505

#define HUE_CHANGE 10

AudioInputAnalog adc(14);
AudioAnalyzeFFT1024 fft;
AudioConnection patchCord1(adc, fft);

byte hue_shift, count;
int i, j, min_brightness, max_brightness, note_avg_dropoff, note_bottom_dropoff, intensity_top_dropoff;
double uv_multiplier, rgb_multiplier, spectrum_multiplier, hue, rgb_value, uv_value, note, note_curr, note_prev, intensity_curr, intensity_prev, intensity_peak, intensity_diff, intensity_avg, intensity_rel, note_rel, note_avg, intensity_top, note_max, note_min;
byte output_pin[4];
int freqs_curr[512], freqs_dropoff[512], freqs_peak[512], freqs_prev[512], freqs_diff[512], freqs_top[512], dropoff_lut[128];
double log_lut[512], freqs_avg[512];
double tmp;

unsigned long start_time, end_time;

CRGB rgb;
CRGB uv;

void setup() {
    Serial.begin(115200);
    //analog write resolution is set to 8 bits by FastLED, DO NOT CHANGE

	// Configure the window algorithm to use
	AudioMemory(32);
	fft.windowFunction(AudioWindowHanning1024);

    min_brightness = 64;
    max_brightness = 255;
    spectrum_multiplier = 100;
    value_multiplier = (max_brightness - min_brightness) / 255.0;

    note_avg_dropoff = 0;
    note_bottom_dropoff = 0;
    intensity_top_dropoff = 0;
    hue = 0;
    hue_shift = 0;
    note_min = 0;
    note_max = 0;

    output_pin[0] = 3;
    output_pin[1] = 4;
    output_pin[2] = 5;
    output_pin[3] = 6;

    for(i=0; i<16; i++){
        dropoff_lut[i] = 0;//pow(i, 3)*0.1;
    }

    for(i=16; i<128; i++){
        dropoff_lut[i] = 20000;//pow(i, 3)*0.1;
    }

    for(i=0; i<512; i++){
        log_lut[i] = log(i);
    }

    for(i=0; i<8; i++){
        pinMode(output_pin[i], OUTPUT);
    }
}

void loop() {
    
	if(fft.available()){
        start_time = micros();
        for(i=0; i<510; i++){
            
            if(fft.output[i] > freqs_curr[i]){
                freqs_curr[i] = fft.output[i];
                freqs_peak[i] = fft.output[i];
                freqs_dropoff[i] = 0;
            }else{
                freqs_curr[i] = max(freqs_peak[i] - dropoff_lut[freqs_dropoff[i]], fft.output[i]);
                freqs_dropoff[i]++;
            }
            freqs_diff[i] = freqs_curr[i] - freqs_prev[i];
            freqs_prev[i] = freqs_curr[i];

            freqs_avg[i] = freqs_avg[i] + ((freqs_curr[i] - freqs_avg[i]) * 0.005);

            /*
            if(freqs_curr[i] >= freqs_top[i]){
                freqs_top[i] = freqs_curr[i];
            }else{
                freqs_top[i] = freqs_top[i] + (freqs_curr[i] - freqs_top[i]) * 0.1;//min((freqs_curr[i] - freqs_top[i])*0.01, 1);
            }
            */
            
            //Serial.println(freqs_curr[i]);
            //Serial.print("\t");
            //Serial.println(freqs_avg[i]);
        
        }
        //Serial.print(3000);
        //Serial.print("\t");
        //Serial.println(2000);
        //Serial.print(0);
        //Serial.print("\t");
        //Serial.println(0);
        
        /*
        intensity_curr = 0;
        for(i=UV_START; i<=UV_END; i++){
            intensity_curr += fft.output[i];
        }
        intensity_curr = intensity_curr * uv_multiplier;

        uv_value = intensity_curr + min_brightness;
        if(uv_value > max_brightness){
            uv_value = 255;
        }else if(uv_value <= 3){
            uv_value = 0;
        }
        uv.setHSV(0, 255, (int)uv_value);
        */

        note_curr = 0;
        note_rel = 0;
        intensity_curr = 0;
        intensity_rel = 0;
        intensity_diff = 0;
        for(i=5; i<=500; i++){
            note_curr += freqs_curr[i] * i;
            note_rel += max(freqs_curr[i] - freqs_avg[i], 0) * i;
            intensity_curr += freqs_curr[i];
            intensity_rel += max(freqs_curr[i] - freqs_avg[i], 0);
            //intensity_diff += max(freqs_diff[i], 0);
        }
        note_curr = note_curr * 100 / intensity_curr;
        note_rel = note_rel * 100 / intensity_rel;
        intensity_curr = intensity_curr * 0.006;
        /*
        if(intensity_curr > 1500){
            intensity_curr = intensity_curr * 0.008;
        }else{
            intensity_curr = 0;
        }
        if(intensity_diff > 200){
            intensity_diff = intensity_diff * 0.008;
        }else{
            intensity_diff = 0;
        }
        */
        
        intensity_avg = intensity_avg + (intensity_curr - intensity_avg) * 0.1;
        note_avg = note_avg + (note_curr - note_avg) * 0.1;

        //Serial.print(note_curr);
        //Serial.print("\t");
        //Serial.print(note_rel);
        //Serial.print("\t");
        //Serial.print(note_avg);
        //Serial.print("\t");
        Serial.print(intensity_curr);
        Serial.print("\t");
        Serial.println(intensity_avg);
        //Serial.print("\t");
        //Serial.println(intensity_rel);

        intensity_prev = intensity_curr;

        hue = 0;
        rgb_value = intensity_avg*value_multiplier + min_brightness;
        if(rgb_value > max_brightness){
            rgb_value = max_brightness;
        }else if(rgb_value <= 4){
            rgb_value = 0;
        }
        if(count >= HUE_CHANGE-1){
            hue_shift++;
        }
        count = (count + 1) % HUE_CHANGE;

        rgb.setHSV(((int)hue + hue_shift)%256, 255, rgb_value);
        analogWrite(output_pin[1], rgb.r);
        analogWrite(output_pin[2], rgb.g);
        analogWrite(output_pin[3], rgb.b);
        analogWrite(output_pin[0], uv.r);
        end_time = micros();
        //Serial.println(end_time - start_time);
	}
}

