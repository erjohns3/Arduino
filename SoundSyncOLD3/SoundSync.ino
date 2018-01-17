#include <Audio.h>
#include <Math.h>
#include <FastLED.h>

#define UV_START 0
#define UV_END 4
#define RGB_START 5
#define RGB_END 125

#define HUE_CHANGE 15

AudioInputAnalog adc(14);
AudioAnalyzeFFT1024 fft;
AudioConnection patchCord1(adc, fft);

byte hue_shift, count;
int i, j, min_brightness, max_brightness, note_avg_dropoff, note_bottom_dropoff, intensity_top_dropoff;
double uv_multiplier, rgb_multiplier, spectrum_multiplier, hue, rgb_value, uv_value, note, note_curr, note_prev, intensity_curr, intensity_prev, intensity_peak, note_avg, intensity_top, note_max, note_min;
byte output_pin[4];
int freqs_curr[128], freqs_prev[128], freqs_diff[128], freqs_dropoff[128], freqs_peak[128], dropoff_lut[128], freqs_top[128];
double note_lut[128];

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
    uv_multiplier = (max_brightness - min_brightness) / 8000.0;
    rgb_multiplier = (max_brightness - min_brightness) / 40000.0;

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

    for(i=0; i<128; i++){
        double num = pow(i, 3)*0.05;
        dropoff_lut[i] = (int)num;
    }

    for(i=0; i<128; i++){
        note_lut[i] = log(i);
    }

    for(i=0; i<8; i++){
        pinMode(output_pin[i], OUTPUT);
    }
}

void loop() {
    
	if(fft.available()){

        for(i=0; i<127; i++){
            
            if(fft.output[i] >= freqs_curr[i]){
                freqs_curr[i] = fft.output[i];
                freqs_peak[i] = fft.output[i];
                freqs_dropoff[i] = 0;
            }else{
                freqs_curr[i] = max(freqs_peak[i] - dropoff_lut[freqs_dropoff[i]], fft.output[i]);
                freqs_dropoff[i]++;
            }

            /*
            if(freqs_curr[i] >= freqs_top[i]){
                freqs_top[i] = freqs_curr[i];
            }else{
                freqs_top[i] = freqs_top[i] + (freqs_curr[i] - freqs_top[i]) * 0.1;//min((freqs_curr[i] - freqs_top[i])*0.01, 1);
            }
            */
            
            /*
            Serial.println(freqs_curr[i]);
            //Serial.print("\t");
            //Serial.println(freqs_top[i]);
            Serial.println(freqs_curr[i]);
            //Serial.print("\t");
            //Serial.println(freqs_top[i]);
            Serial.println(freqs_curr[i]);
            //Serial.print("\t");
            //Serial.println(freqs_top[i]);
            Serial.println(freqs_curr[i]);
            //Serial.print("\t");
            //Serial.println(freqs_top[i]);
            */
            
        }
        
        /*
        //Serial.print(8000);
        //Serial.print("\t");
        Serial.println(8000);
        //Serial.print(0);
        //Serial.print("\t");
        Serial.println(0);
        //Serial.print(8000);
        //Serial.print("\t");
        Serial.println(8000);
        //Serial.print(0);
        //Serial.print("\t");
        Serial.println(0);
        */

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


        note_curr = 0;
        intensity_curr = 0;
        for(i=RGB_START; i<=RGB_END; i++){
            note_curr += freqs_curr[i] * note_lut[i];
            intensity_curr += freqs_curr[i];
        }
        note_curr = note_curr * spectrum_multiplier / intensity_curr;
        intensity_curr = intensity_curr * rgb_multiplier;

        note_curr = note_prev + (note_curr - note_prev)*min(max(intensity_curr - intensity_prev, 0.0)*0.05, 1.0);        

        note_avg = note_avg + (note_curr - note_avg) * 0.003;

        if(note_curr > note_max){
            note_max = note_curr;
        }else{
            note_max = note_max + (note_curr - note_max) * 0.001;//min(pow(fabs(note_avg - note_max), 2.0)*0.0000003, 1.0);
        }

        if(note_curr < note_min){
            note_min = note_curr;
        }else{
            note_min = note_min + (note_curr - note_min) * 0.001;//min(pow(fabs(note_avg - note_min), 2.0)*0.001, 1.0);
        }
    
        /*
        if(intensity_curr >= intensity_top){
            intensity_top = intensity_top + (intensity_curr - intensity_top)*min(pow(fabs(intensity_curr - intensity_top), 2.0)*0.001, 1.0);
            //intensity_top = intensity_curr;
            intensity_peak = intensity_top;
            intensity_top_dropoff = 0;
        }else{
            //intensity_top = intensity_top + (intensity - intensity_top) * min(pow(intensity_top_dropoff, 3.0)*0.00003, 1.0);
            intensity_top = max(intensity_peak - (pow(intensity_top_dropoff, 3.0) * 0.004), intensity_curr);
            intensity_top_dropoff++;
        }
        */

        //hue = ((note_max - note_min)*1.0) + ((note_avg - note_min) * 85 / (note_max - note_min));
        //rgb_value = intensity_top;
        

        Serial.print(note_curr);
        Serial.print("\t");
        Serial.print(note_avg);
        Serial.print("\t");
        Serial.print(note_max);
        Serial.print("\t");
        Serial.print(note_min);
        Serial.print("\t");
        Serial.print(intensity_curr);
        Serial.print("\t");
        Serial.println(intensity_curr - intensity_prev);

        note_prev = note_curr;
        intensity_prev = intensity_curr;

        //hue = note_curr;
        hue = (85.0 / max(note_max - note_min, 40) * (note_curr - note_avg)) + note_avg;
        rgb_value = intensity_curr + min_brightness;
        if(rgb_value > max_brightness){
            rgb_value = max_brightness;
        }else if(rgb_value <= 4){
            rgb_value = 0;
        }
        if(count >= HUE_CHANGE-1){
            hue_shift++;
        }
        count = (count + 1) % HUE_CHANGE;

        rgb.setHSV((int)(hue + hue_shift)%256, 255, (int)rgb_value);
        analogWrite(output_pin[1], rgb.r);
        analogWrite(output_pin[2], rgb.g);
        analogWrite(output_pin[3], rgb.b);
        analogWrite(output_pin[0], uv.r);
	}

    if(Serial.available() > 0){
        i = Serial.read();
        if(i == 49){        // 1 key
            // -= 1;
            if(min_brightness > max_brightness){
                min_brightness = max_brightness;
            }
        }else if(i == 50){  // 2 key
            min_brightness += 1;
            if(min_brightness < 0){
                min_brightness = 0;
            }
        }else if(i == 51){  // 3 key
            max_brightness += 16;
            if(max_brightness > 255){
                max_brightness = 255;
            }
        }else if(i == 52){  // 4 key
            max_brightness -= 16;
            if(max_brightness < min_brightness){
                max_brightness = min_brightness;
            }
        }else if(i == 53){  // 5 key
            spectrum_multiplier += 20;
            if(spectrum_multiplier > 5000){
                spectrum_multiplier = 5000;
            }
        }else if(i == 54){  // 6 key
            spectrum_multiplier -= 20;
            if(spectrum_multiplier < 0){
                spectrum_multiplier = 0;
            }
        }
        uv_multiplier = (max_brightness - min_brightness) / 8000.0;
        rgb_multiplier = (max_brightness - min_brightness) / 22000.0;
        Serial.println("1: increase min_brightness, 2: decrease min_brightness");
        Serial.println("3: increase max brightness, 4: decrease max brightness");
        Serial.println("5: increase spectrum count, 6: decrease spectrum count");
        Serial.println("7: increase smoothing amount, 8: decrease smoothing amount");
        Serial.println();
        Serial.print("min_brightness: ");
        Serial.println(min_brightness);
        Serial.print("max brightness: ");
        Serial.println(max_brightness);
        Serial.print("spectrum count: ");
        Serial.println(spectrum_multiplier);
        Serial.println();
    }
}

