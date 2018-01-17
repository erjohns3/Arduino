#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7       // VS1053 chip select pin (output)
#define SHIELD_DCS    6       // VS1053 Data/command select pin (output)
#define CARDCS 4              // Card chip select pin
#define DREQ 3                // VS1053 Data request, ideally an Interrupt pin

int song = 12;
int soundfx = 13;
boolean just_playing = true;

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

void setup() {
  Serial.begin(9600);
  musicPlayer.begin();
  SD.begin(CARDCS);
  musicPlayer.setVolume(20, 20);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT));
  pinmode(song, OUTPUT);
  pinmode(soundfx, OUTPUT);
  digitalWrite(soundfx, LOW);
  digitalWrite(song, HIGH);
}

void loop() {

  if (musicPlayer.stopped() && just_playing) {
    digitalWrite(soundfx, LOW);
    digitalWrite(song, HIGH);
    just_playing = false;
  }

  if (Serial.available()) {
    char c = Serial.read();
    
    digitalWrite(song, LOW);
    digitalWrite(soundfx, HIGH);
    just_playing = true;
    
    if (c == 'u') {
      musicPlayer.stopPlaying();
      musicPlayer.startPlayingFile("soundfx0.mp3"));
    } else if (c == 'r') {
      musicPlayer.stopPlaying();
      musicPlayer.startPlayingFile("soundfx1.mp3"));
    } else if (c == 'd') {
      musicPlayer.stopPlaying();
      musicPlayer.startPlayingFile("soundfx2.mp3"));
    } else if (c == 'l') {
      musicPlayer.stopPlaying();
      musicPlayer.startPlayingFile("soundfx3.mp3"));
    }
  }

  delay(100);
}
