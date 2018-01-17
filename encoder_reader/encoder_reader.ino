bool prev = 8;
bool curr = 8;
byte check = 8;
int count[4];
byte mapping[16];
int i = 0;
byte x = 0;
bool fill_bool0 = 1;
bool fill_bool1 = 1;
int fill_int = 0;
int j = 0;
unsigned long start_micros, end_micros, start_millis, end_millis;

void setup(){
    Serial.begin(9600);
    DDRD = 0x00;
    count[0] = 0;
    count[1] = 0;
    count[2] = 0;
    count[3] = 0;

    for(i=0; i<16; i++){
        mapping[i] = 0;
    }
    mapping[0] = -1;
}

void encoders(int* count, byte* mapping){
    
    asm volatile(

    "mov r2, r26 \n\t"
    "mov r3, r27 \n\t"
    "mov r4, r18 \n\t"
    "mov r5, r19 \n\t"

    "sbr r26, 64 \n\t"
    "sbr r27, 0 \n\t"
    "clr r7 \n\t"
    "clr r8 \n\t"
    "clr r9 \n\t"
    "clr r10 \n\t"
    "clr r16 \n\t"
    "clr r17 \n\t"
    "clr r19 \n\t"

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r16, 0 \n\t"   // register for encoder 1

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r16, 2 \n\t"   // register for encoder 1

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 2

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder 2

    "LOOP:     \n\t"    // start loop

    "lsl r16 \n\t"
    "lsl r17 \n\t"

    "in r18, 0x05 \n\t"
    "bst r18, 2 \n\t"
    "bld r16, 0 \n\t"

    "in r18, 0x0B \n\t"
    "bst r18, 0 \n\t"
    "bld r16, 2 \n\t"

    "in r18, 0x05 \n\t" // port B
    "bst r18, 2 \n\t"   // pin number
    "bld r17, 0 \n\t"   // register for encoder 2

    "in r18, 0x0B \n\t" // port D
    "bst r18, 0 \n\t"   //pin number
    "bld r17, 2 \n\t"   // register for encoder 2

    "andi r16, 0x0F \n\t"
    "andi r17, 0x0F \n\t"

    "mov r28, r22 \n\t"
    "mov r29, r23 \n\t"
    "add r28, r16 \n\t"
    "adc r29, r19 \n\t"
    
    "ldd r6, Y+0 \n\t"
    "add r7, r6 \n\t"

    "mov r28, r22 \n\t"
    "mov r29, r23 \n\t"
    "add r28, r17 \n\t"
    "adc r29, r19 \n\t"
    
    "ldd r6, Y+1 \n\t"
    "add r8, r6 \n\t"

    "subi r26, 1 \n\t"
    "sbc r27, r19 \n\t"
    
    "brne LOOP \n\t"    // end loop

    "mov r26, r2 \n\t"
    "mov r27, r3 \n\t"
    "mov r18, r4 \n\t"
    "mov r19, r5 \n\t"

    "mov r28, r24 \n\t"
    "mov r29, r25 \n\t"
    "std Y+0, r7 \n\t"
    "std Y+0, r8 \n\t"
    
    );
}


void loop(){

    start_micros = micros();
    encoders(count, mapping);
    end_micros = micros();
    //Serial.println(end_micros - start_micros);
    Serial.println(count[0]);
    delay(500);    
}
