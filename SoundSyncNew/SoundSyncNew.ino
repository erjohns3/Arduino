#include <Audio.h>
#include <Math.h>
#include <FastLED.h>

#define HISTORY_LARGE 128
#define HISTORY_SMALL 7
#define HISTORY_COVER 4

#define HUE_CHANGE 10

AudioInputAnalog adc(14);
AudioAnalyzeFFT1024 fft;
AudioConnection patchCord1(adc, fft);

byte hue_shift, count;
int i, min_brightness, max_brightness, note_top_dropoff, note_bottom_dropoff, intensity_top_dropoff, note_count;
double uv_multiplier, rgb_multiplier, spectrum_multiplier, hue, rgb_value, uv_value, tmp, note, intensity, intensity_prev, intensity_diff, intensity_red, intensity_green, intensity_blue, intensity_red_smooth, intensity_green_smooth, intensity_blue_smooth;
byte output_pin[4];
double freqs[256], freqs_mult[256], freqs_peak[256], freqs_prev[256];
int freqs_dropoff[256];
double range_raw[3], range_smooth[3], range_mult[3], range_peak[256], range_prev[256];
int range_dropoff[256];
double dropoff_lut[128];

CRGB rgb;
CRGB uv;

void setup() {
    Serial.begin(115200);
    //analog write resolution is set to 8 bits by FastLED, DO NOT CHANGE

	// Configure the window algorithm to use
	AudioMemory(32);
	fft.windowFunction(AudioWindowHanning1024);

    min_brightness = 0;
    max_brightness = 255;
    spectrum_multiplier = 100;
    uv_multiplier = 255.0 / 8000.0;
    rgb_multiplier = 255.0 / 30000.0;

    intensity_prev = 0;
    intensity_diff = 0;
    hue = 0;
    uv_value = 0;
    rgb_value = 0;
    hue_shift = 0;

    output_pin[0] = 3;
    output_pin[1] = 4;
    output_pin[2] = 5;
    output_pin[3] = 6;

    for(i=0; i<128; i++){
        dropoff_lut[i] = max(1.0 - double(pow(double(i), 2)) / 1000, 0);
    }

    for(i=0; i<256; i++){
        freqs[i] = 0;
        freqs_dropoff[i] = 0;
        freqs_mult[i] = 0;
        freqs_peak[i] = 0;
        freqs_prev[i] = 0;
    }

    for(i=0; i<3; i++){
        range_raw[i] = 0;
        range_smooth[i] = 0;
        range_dropoff[i] = 0;
        range_mult[i] = 0;
        range_peak[i] = 0;
        range_prev[i] = 0;
    }

    for(i=0; i<4; i++){
        freqs_mult[i] = 1;
    }

    for(i=100; i<256; i++){
        freqs_mult[i] = 0;
    }

    for(i=0; i<8; i++){
        pinMode(output_pin[i], OUTPUT);
    }
}

void loop() {
    
	if(fft.available()){

        /*
        note = 0;
        intensity = 0;
        intensity_diff = 0;

        for(i=0; i<256; i++){
            
            
            intensity_diff += max((fft.output[i]-freqs[i]), 0) * freqs_mult[i];
            
            if(fft.output[i] > freqs[i]){
                freqs[i] += 1*(double(fft.output[i]) - freqs[i]);
                //freqs_peak[i] = fft.output[i];
                //freqs_dropoff[i] = 0;
            }else{
                freqs[i] += 0.02*(double(fft.output[i]) - freqs[i]);
                //freqs[i] = max(freqs_peak[i] - dropoff_lut[freqs_dropoff[i]], fft.output[i]);
                //freqs_dropoff[i] = min(freqs_dropoff[i] + 1, 127);
            }
            
            
            //note += freqs[i]*log(i);
            //intensity += fft.output[i];
            //intensity_diff += max((fft.output[i])*(1+i)*0.01, 0);
            //note += freqs[i]*i;
            //intensity_smooth += freqs[i];

            //freqs_prev[i] = fft.output[i];
        }
        */

        /*
        intensity_red = 0;
        intensity_green = 0;
        intensity_blue = 0;

        intensity_red_smooth = 0;
        intensity_green_smooth = 0;
        intensity_blue_smooth = 0;
        */

        range_raw[0] = 0;
        for(i=0; i<4; i++){
            /*
            if(fft.output[i] >= freqs[i]){
                //freqs[i] += 1*(double(fft.output[i]) - freqs[i]);
                freqs[i] = fft.output[i];
                freqs_peak[i] = fft.output[i];
                freqs_dropoff[i] = 0;
            }else{
                //freqs[i] += 0.02*(double(fft.output[i]) - freqs[i]);
                freqs[i] = max(freqs_peak[i] - dropoff_lut[freqs_dropoff[i]], fft.output[i]);
                freqs_dropoff[i] = min(freqs_dropoff[i] + 1, 127);
            }
            intensity_red += fft.output[i];
            intensity_red_smooth += freqs[i];
            */

            range_raw[0] += fft.output[i];
        }
        range_raw[1] = 0;
        for(i=4; i<150; i++){
            /*
            if(fft.output[i] >= freqs[i]){
                freqs[i] += 0.02*(double(fft.output[i]) - freqs[i]);
            }else{
                freqs[i] += 0.02*(double(fft.output[i]) - freqs[i]);
            }
            intensity_green += fft.output[i];
            intensity_green_smooth += freqs[i];
            */

            range_raw[1] += fft.output[i];
        }
        range_raw[2] = 0;
        for(i=128; i<256; i++){
            /*
            if(fft.output[i] >= freqs[i]){
                //freqs[i] += 1*(double(fft.output[i]) - freqs[i]);
                freqs[i] = fft.output[i];
                freqs_peak[i] = fft.output[i];
                freqs_dropoff[i] = 0;
            }else{
                //freqs[i] += 0.02*(double(fft.output[i]) - freqs[i]);
                freqs[i] = max(freqs_peak[i] - dropoff_lut[freqs_dropoff[i]], fft.output[i]);
                freqs_dropoff[i] = min(freqs_dropoff[i] + 1, 127);
            }
            intensity_blue += fft.output[i];
            intensity_blue_smooth += freqs[i];
            */

            range_raw[2] += fft.output[i];
            //range_raw[2] += double(fft.output[i]) * double(i) / 150;
        }

        if(range_raw[0] >= range_smooth[0]){
            range_smooth[0] = range_raw[0];
            range_peak[0] = range_raw[0];
            range_dropoff[0] = 0;
        }else{
            range_smooth[0] = max(range_peak[0] * dropoff_lut[range_dropoff[0]], range_raw[0]);
            range_dropoff[0] = min(range_dropoff[0] + 1, 127);
        }

        if(range_raw[1] > range_smooth[1]){
            range_smooth[1] += 0.02*(range_raw[1] - range_smooth[1]);
        }else{
            range_smooth[1] += 0.02*(range_raw[1] - range_smooth[1]);
        }

        if(range_raw[2] > range_smooth[2]){
            range_smooth[2] = range_raw[2];
            range_peak[2] = range_raw[2];
            range_dropoff[2] = 0;
        }else{
            range_smooth[2] = max(range_peak[2] * dropoff_lut[range_dropoff[2]], range_raw[2]);
            range_dropoff[2] = min(range_dropoff[2] + 1, 127);
        }

        //note = note / intensity;
        //intensity_diff = max(intensity - intensity_prev, 0);
        //intensity = intensity * rgb_multiplier;

        /*
        if(intensity >= intensity_top){
            intensity_top = intensity;
        }else{
            intensity_top = max(intensity, intensity_top - 20);
        }
        */

        /*
        Serial.print(600);
        Serial.print("\t");
        Serial.println(600);

        Serial.print(0);
        Serial.print("\t");
        Serial.println(0);
        
        for(i=0; i<248; i++){
            Serial.print(int(freqs[i]));
            Serial.print("\t");
            Serial.println(fft.output[i]);
            Serial.print(int(freqs[i]));
            Serial.print("\t");
            Serial.println(fft.output[i]);
        }

        Serial.print(0);
        Serial.print("\t");
        Serial.println(0);

        Serial.print(600);
        Serial.print("\t");
        Serial.println(600);
        */
        
        //Serial.print(intensity_blue_smooth);
        //Serial.print("\t");
        Serial.print(int(range_smooth[2]);
        Serial.print("\t");
        Serial.print(int(range_smooth[0]));
        Serial.print("\t");
        Serial.print(int(range_raw[2]));
        Serial.print("\t");
        Serial.println(int(range_raw[0]));
        //Serial.print("\t");
        //Serial.println(intensity_green);
        

        //intensity_prev = intensity;

        /*
        note_history_small[history_index_small] = note;
        intensity_history_small[history_index_small] = intensity;
        note_history_avg[history_index_avg] = 0;
        intensity_history_avg[history_index_avg] = 0;
        for(i=0; i<HISTORY_SMALL; i++){
            note_history_avg[history_index_avg] += note_history_small[i];
            intensity_history_avg[history_index_avg] += intensity_history_small[i];
        }
        note_history_avg[history_index_avg] /= HISTORY_SMALL;
        intensity_history_avg[history_index_avg] /= HISTORY_SMALL;

        hue = note_history_avg[(history_index_avg+1) % HISTORY_AVG];
        rgb_value = intensity_history_avg[(history_index_avg+1) % HISTORY_AVG];

        history_index_small = (history_index_small + 1) % HISTORY_SMALL;
        history_index_avg = (history_index_avg + 1) % HISTORY_AVG;
        */

        /*
        note_history_large[history_index_large] = note;
        history_index_large = (history_index_large + 1) % HISTORY_LARGE;
        note_avg = 0;
        note_max = 0;
        for(i=0; i<HISTORY_LARGE; i++){
            note_avg += note_history_large[i];
            if(note_history_large[i] > note_max){
                note_max = note_history_large[i];
            }
        }
        note_avg /= HISTORY_LARGE;
        */
        
        /*
        if(note - note_top >= 0){
            note_top = note_top + (note - note_top)*min(pow(fabs(note - note_top), 2.0)*10.0, 1.0);
            //note_top = note;
            note_top_dropoff = 0;
        }else{
            note_top = note_top + (note - note_top) * min(pow(note_top_dropoff, 2.0)*0.002, 1.0);
            note_top_dropoff++;
        }

        if(note_top > note_max){
            note_max = note_top;
        }
        if(note_top < note_min){
            note_min = note_top;
        }
        note_count++;
        if(note_count >= 128){
            note_count = 0;
            top_cover_diff = (note_max - top_cover) / 512;
            bottom_cover_diff = (note_min - bottom_cover) / 512;
            note_max = 0;
            note_min = 10000;
        }

        top_cover += top_cover_diff;
        bottom_cover += bottom_cover_diff;

        /*
        note_history_cover[history_index_cover] = note_top;
        note_max = -1;
        note_min = 100000;
        for(i=0; i<HISTORY_COVER; i++){
            if(note_history_cover[i] > note_max){
                note_max = note_history_cover[i];
            }
            if(note_history_cover[i] < note_min){
                note_min = note_history_cover[i];
            }
        }
        for(i=0; i<HISTORY_COVER; i++){
            if(note_history_cover[i] < note_max){
                note_history_cover[i] = note_min;
            }
        }
        hue = note_history_cover[(history_index_cover + 1) % HISTORY_COVER];
        

        if(intensity - intensity_top > 0){
            //intensity_top = intensity_top + (intensity - intensity_top)*min(pow(fabs(intensity - intensity_top), 2.0)*0.001, 1.0);
            intensity_top = intensity;
            intensity_top_dropoff = 0;
        }else{
            //hue = hue + (note_avg - hue) * min(pow(intensity_top_dropoff, 1.5)*0.0005, 1.0);
            intensity_top = intensity_top + (intensity - intensity_top) * min(pow(intensity_top_dropoff, 2.0)*0.001, 1.0);
            intensity_top_dropoff++;
        }

        hue = (note_top * 85.0 / (top_cover - bottom_cover)) + 256;
        //hue = note_avg + (note_top - note_avg) * min(pow(intensity_top, 1.0) * 0.005, 1.0);
        rgb_value = intensity_top;

        intensity_history_cover[history_index_cover] = intensity_top;
        note_max = -1;
        note_min = 100000;
        for(i=0; i<HISTORY_COVER; i++){
            if(intensity_history_cover[i] > note_max){
                note_max = intensity_history_cover[i];
            }
            if(intensity_history_cover[i] < note_min){
                note_min = intensity_history_cover[i];
            }
        }
        for(i=0; i<HISTORY_COVER; i++){
            if(intensity_history_cover[i] < note_max){
                intensity_history_cover[i] = note_min;
            }
        }

        rgb_value = intensity_history_cover[(history_index_cover + 1) % HISTORY_COVER];

        history_index_cover = (history_index_cover + 1) % HISTORY_COVER;

        if(note - note_avg > 0){
            hue_history[history_index_small] = pow(fabs(note - note_avg), 1.0) * (intensity - intensity_avg);
        }else{
            hue_history[history_index_small] = -pow(fabs(note - note_avg), 1.0)*0.01;
        }

        hue_history[history_index_small] = (note - note_avg) * max(intensity - intensity_avg, 0) * 0.04;
        history_index_small = (history_index_small + 1) % HISTORY_SMALL;
        hue_avg = 0;
        for(i=0; i<HISTORY_SMALL; i++){
            hue_avg += hue_history[i];
        }
        hue_avg /= HISTORY_SMALL;
        
        note_history_large[history_index_large] = note;
        intensity_history_large[history_index_large] = intensity;
        history_index_large = (history_index_large + 1) % HISTORY_LARGE; 
        note_avg = 0;
        intensity_avg = 0;
        for(i=0; i<HISTORY_LARGE; i++){
            tmp = pow(intensity_history_large[i], 4);
            note_avg += note_history_large[i] * tmp;
            intensity_avg += tmp;
        }
        note_avg /= intensity_avg;

        intensity_history_small[history_index_small] = intensity;
        history_index_small = (history_index_small + 1) % HISTORY_SMALL; 
        intensity_avg = 0;
        for(i=0; i<HISTORY_SMALL; i++){
            intensity_avg += intensity_history_small[i];
        }
        intensity_avg /= HISTORY_SMALL;

        hue = note_avg;
        rgb_value = intensity_avg;
        */
        
        /*
        for(i=0; i<128; i++){
            Serial.println(fft.output[i]);
            Serial.println(fft.output[i]);
            Serial.println(fft.output[i]);
            Serial.println(fft.output[i]);
        }
        Serial.println(0.0);
        Serial.println(0.0);
        Serial.println(0.0);
        Serial.println(150.0);
        */

        //hue = fmod(hue + hue_shift, 256);
        //rgb_value = (rgb_value * (max_brightness - min_brightness) / 255) + min_brightness;
        if(rgb_value > 255){
            rgb_value = 255;
        }else if(rgb_value <= 4){
            rgb_value = 0;
        }
        if(count >= HUE_CHANGE-1){
            hue_shift++;
        }
        count = (count + 1) % HUE_CHANGE;

        //rgb.setHSV((int)(hue + hue_shift)%256, 255, (int)rgb_value);
        //analogWrite(output_pin[1], rgb.r);
        //analogWrite(output_pin[2], rgb.g);
        //analogWrite(output_pin[3], rgb.b);
	}

    if(Serial.available() > 0){
        i = Serial.read();
        if(i == 49){        // 1 key
            min_brightness += 16;
            if(min_brightness > max_brightness){
                min_brightness = max_brightness;
            }
        }else if(i == 50){  // 2 key
            min_brightness -= 16;
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

