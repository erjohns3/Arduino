#include <Audio.h>
#include <Math.h>
#include <FastLED.h>

#define UV_START 0
#define UV_END 4
#define RGB_START 5
#define RGB_END 128

#define HUE_CHANGE 15

AudioInputAnalog adc(14);
AudioAnalyzeFFT1024 fft;
AudioConnection patchCord1(adc, fft);

byte hue_shift, count;
int i, j, min_brightness, max_brightness, note_top_dropoff, note_bottom_dropoff, intensity_top_dropoff, max_diff, index_diff;
double uv_multiplier, rgb_multiplier, spectrum_multiplier, hue, rgb_value, uv_value, note, note_curr, note_prev, intensity, intensity_peak, note_top,x intensity_top, note_max, note_min;
byte output_pin[4];
int freqs[128], freqs_dropoff[128], freqs_peak[128], dropoff_lut[128];

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
    spectrum_multiplier = 75;
    uv_multiplier = 0.03;
    rgb_multiplier = 0.0064;

    note_top_dropoff = 0;
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
        dropoff_lut[i] = pow(i, 2);
    }

    for(i=0; i<8; i++){
        pinMode(output_pin[i], OUTPUT);
    }
}

void loop() {
    
	if(fft.available()){
        uv_value = 0;
        for(i=UV_START; i<=UV_END; i++){
            uv_value += fft.output[i];
        }
        uv_value *= uv_multiplier;
        if(uv_value > 255){
            uv_value = 255;
        }else if(uv_value <= 3){
            uv_value = 0;
        }
        uv.setHSV(0, 255, uv_value + min_brightness);
        //analogWrite(output_pin[0], uv.r);

        /*
        note = 0;
        intensity = 0;
        for(i=RGB_START; i<=RGB_END; i++){
            note += fft.output[i]*log(i);
            intensity += fft.output[i];
        }
        note = note * 100.0 / intensity;
        intensity = intensity * rgb_multiplier;

        if(note >= note_top){
            note_top = note_top + (note - note_top) * min(pow(fabs(note - note_top), 2.0)*0.001, 1.0);
            note_top_dropoff = 0;
        }else{
            note_top = note_top + (note - note_top) * min(pow(note_top_dropoff, 2.0)*0.002, 1.0);
            note_top_dropoff++;
        }

        if(note_top >= note_max){
            note_max = note_max + (note_top - note_max) * min(pow(fabs(note_top - note_max), 2.0)*0.001, 1.0);
        }else{
            note_max = note_max + (note_top - note_max) * min(pow(fabs(note_top - note_max), 2.0)*0.0000003, 1.0);
        }

        if(note_top <= note_min){
            note_min = note_min + (note_top - note_min) * min(pow(fabs(note_top - note_min), 2.0)*0.001, 1.0);
        }else{
            note_min = note_min + (note_top - note_min) * min(pow(fabs(note_top - note_min), 2.0)*0.0000003, 1.0);
        }

        if(intensity >= intensity_top){
            intensity_top = intensity_top + (intensity - intensity_top)*min(pow(fabs(intensity - intensity_top), 2.0)*0.001, 1.0);
            //intensity_top = intensity;
            intensity_peak = intensity_top;
            intensity_top_dropoff = 0;
        }else{
            //hue = hue + (note_avg - hue) * min(pow(intensity_top_dropoff, 1.5)*0.0005, 1.0);
            //intensity_top = intensity_top + (intensity - intensity_top) * min(pow(intensity_top_dropoff, 3.0)*0.00003, 1.0);
            intensity_top = max(intensity_peak - (pow(intensity_top_dropoff, 3.0) * 0.004), intensity);
            intensity_top_dropoff++;
        }

        hue = ((note_max - note_min)*1.0) + ((note_top - note_min) * 42.5 / (note_max - note_min));
        rgb_value = intensity_top;
        //hue = 0;
        */

        /*
        intensity_low = 0;
        intensity_mid = 0;
        intensity_high = 0;
        for(i=3; i<12; i++){
            intensity_low += fft.output[i];
        }
        for(i=14; i<30; i++){
            intensity_mid += fft.output[i];
        }
        for(i=30; i<128; i++){
            intensity_high += fft.output[i];
        }

        Serial.print(intensity_low);
        Serial.print("\t");
        Serial.print(intensity_mid);
        Serial.print("\t");
        Serial.println(intensity_high);
        */
        

        /*
        intensity_low = 0;
        for(i=0; i<128; i++){
            intensity_low += fft.output[i];
        }
        Serial.println(intensity_low);
        */

        /*
        for(i=0; i<6; i++){
            intensity_low = 0;
            for(j=i*10+5; j<i*10+25; j++){
                intensity_low += fft.output[j];
            }
            Serial.print(intensity_low);
            Serial.print("\t");
        }
        Serial.println();
        */
        

        /*
        max_diff = 0;
        index_diff = 0;
        for(i=0; i<127; i++){
            notes_diff[i] = fft.output[i] - notes_prev[i];
            if(notes_diff[i] > max_diff){
                max_diff = notes_diff[i];
                index_diff = i;
            }
            notes_prev[i] = fft.output[i];
        }

        for(i=0; i<127; i++){
            if(i == index_diff){
                notes_curr[i] = fft.output[i];
            }else{
                notes_curr[i] = max(notes_curr[i]-50, 0);
            }
            Serial.println(notes_curr[i]);
            Serial.println(notes_curr[i]);
            Serial.println(notes_curr[i]);
            Serial.println(notes_curr[i]);
        }
        */
        

        /*
        for(i=0; i<127; i++){
            if(fft.output[i] >= notes_curr[i]){
                notes_curr[i] = fft.output[i];
            }else{
                notes_curr[i] = max(notes_curr[i]-20, 0);
            }
            Serial.println(notes_curr[i]);
            Serial.println(notes_curr[i]);
            Serial.println(notes_curr[i]);
            Serial.println(notes_curr[i]);
        }

        Serial.println(0);
        Serial.println(8000);
        Serial.println(0);
        Serial.println(8000);
        */
        
        for(i=0; i<127; i++){
            if(fft.output[i] >= freqs[i]){
                freqs[i] = fft.output[i];
                freqs_peak[i] = fft.output[i];
                freqs_dropoff[i] = 0;
            }else{
                freqs[i] = max(freqs_peak[i] - dropoff_lut[freqs_dropoff[i]], fft.output[i]);
                freqs_dropoff[i]++;
            }
            //Serial.println(freqs[i]);
            //Serial.println(freqs[i]);
            //Serial.println(freqs[i]);
            //Serial.println(freqs[i]);
        }
        //Serial.println(0);
        //Serial.println(8000);
        //Serial.println(0);
        //Serial.println(8000);

        note_curr = 0;
        intensity = 0;
        for(i=RGB_START; i<=RGB_END; i++){
            note_curr += freqs[i]*log(i);
            intensity += freqs[i];
        }
        note_curr = note_curr * spectrum_multiplier / intensity;
        intensity = intensity * rgb_multiplier;

        note_curr = note_prev + (note_curr - note_prev)*min(intensity*0.004, 1.0);
        note_prev = note_curr;

        Serial.print(note_curr);
        Serial.print("\t");
        Serial.println(intensity);

        /*
        Serial.print(note_top);
        Serial.print("\t");
        Serial.print(note_max);
        Serial.print("\t");
        Serial.print(note);
        Serial.print("\t");
        Serial.print(intensity);
        Serial.print("\t");
        Serial.print(note_min);
        Serial.print("\t");
        Serial.println(intensity_top);
        */

        hue = note_curr;
        rgb_value = intensity;
        if(rgb_value > 255){
            rgb_value = 255;
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

