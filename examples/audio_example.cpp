#include <Arduino.h>
#include "AudioTools.h" // install https://github.com/pschatzmann/arduino-audio-tools
#include "Audioboard.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"

AudioBoard board{AudioDriverES8311, NoPins};
AudioBoardStream out(board);
AudioInfo info(32000, 2, 16);

SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> sound(sineWave);
StreamCopy copier(out, sound, 4096);

void audioTask(void*){
  for(;;) {
    copier.copy();
    vTaskDelay(1);
  }
}

void setup() {
    // Setup logging
    Serial.begin(115200);
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Debug);
    AudioDriverLogger.begin(Serial, AudioDriverLogLevel::Debug);

    Serial.println("initializing pins...");
    DriverPins pins;
    pins.addI2C(PinFunction::CODEC, 18, 8);
    pins.addI2S(PinFunction::CODEC, 2, 17, 45, 15);
    pins.addPin(PinFunction::PA, PA_PIN, PinLogic::Output);
    board.setPins(pins);
    
    // start I2S & codec
    Serial.println("starting I2S...");
    out.begin();
    out.setAudioInfo(info);
    out.setVolume(0.7f);

    // Setup sine wave
    Serial.println("setting up sine wave...");
    sineWave.begin(info, N_B4);
    
    copier.begin();
    xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 10, nullptr, 0);
}



// Arduino loop - copy sound to out
void loop() {
    // copier.copy();
    delay(1000);
}