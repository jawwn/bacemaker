/*  BaceMaker Foot Synth
    using Mozzi sonification library.
    check the README or http://sensorium.github.com/Mozzi/
    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

    Uses some example code from Mozzi Examples
    Tim Barrass 2013, CC by-nc-sa.
*/
// First array entry can be whatever, pins[1] if the bottom C
// Use these to specify the pins you'll be using on your Arduino
int pins[] = {
  100, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 15, 16
};
#include <EEPROM.h>
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template

#include <EventDelay.h>
#include <Line.h>
#include <ADSR.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <WaveShaper.h>
#include <Smooth.h>
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/saw_analogue512_int8.h>
#include <tables/sin512_int8.h>
#include <tables/sin1024_int8.h>
#include <tables/sin2048_int8.h>
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/waveshape_chebyshev_3rd_256_int8.h>
#include <tables/waveshape_chebyshev_6th_256_int8.h>
#include <tables/waveshape_compress_512_to_488_int16.h>



int ledPin = A4;
// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);
Oscil <SAW_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aSaw1(SAW_ANALOGUE512_DATA);
Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> aTri1(TRIANGLE_HERMES_2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin3(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin4(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin5(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin6(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin7(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin8(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin9(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, CONTROL_RATE> kTremolo(SIN2048_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA); // sine wave sound source
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aGain1(SIN2048_DATA); // to fade sine wave in and out before waveshaping
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aGain2(SIN2048_DATA); // to fade sine wave in and out before waveshaping

// Chebyshev polynomial curves, The nth curve produces the n+2th harmonic component.
WaveShaper <char> aCheby3rd(CHEBYSHEV_3RD_256_DATA); // 5th harmonic
WaveShaper <char> aCheby6th(CHEBYSHEV_6TH_256_DATA); // 8th harmonic
WaveShaper <int> aCompress(WAVESHAPE_COMPRESS_512_TO_488_DATA); // to compress instead of dividing by 2 after adding signals
// smooth transitions between notes
Smooth <unsigned int> kSmoothFreq(0.85f);
int target_freq, smoothed_freq;


int octave_addr = 1;
int sin_addr = 8;
int saw_addr = 16;
int tri_addr = 24;
int sin2_addr = 32;
int sin3_addr = 40;
int sin4_addr = 48;
int sin5_addr = 56;
int sin6_addr = 64;
int sin7_addr = 72;
int sin8_addr = 80;
int sin9_addr = 88;
int waveshaper_addr = 224;
int waveshaper_speed1_addr = 216;
int waveshaper_speed2_addr = 232;

int trem_speed_addr = 240;
int trem_addr = 248;
int long_hold_addr = 256;
int swell_addr = 208;
int sin_active = EEPROM.read(sin_addr);
int saw_active = EEPROM.read(saw_addr);
int tri_active = EEPROM.read(tri_addr);
int sin2_active = EEPROM.read(sin2_addr);
int sin3_active = EEPROM.read(sin2_addr);
int sin4_active = EEPROM.read(sin4_addr);
int sin5_active = EEPROM.read(sin5_addr);
int sin6_active = EEPROM.read(sin6_addr);
int sin7_active = EEPROM.read(sin7_addr);
int sin8_active = EEPROM.read(sin8_addr);
int sin9_active = EEPROM.read(sin9_addr);
int trem_active = EEPROM.read(trem_addr);
int trem_speed = EEPROM.read(trem_speed_addr);
int waveshaper_active = EEPROM.read(waveshaper_addr);
int waveshaper_speed1 = EEPROM.read(waveshaper_speed1_addr);
int waveshaper_speed2 = EEPROM.read(waveshaper_speed2_addr);
int swell = EEPROM.read(swell_addr);
int long_hold = EEPROM.read(long_hold_addr);
int long_hold_active = 0;
EventDelay longHoldDelay;
EventDelay miscDelay;
int max_octaves = 4;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope;

Line <unsigned int> tremGain;

int key_pressed = 0;
int octave = EEPROM.read(octave_addr);

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 64 // powers of 2 please



int current_pin = 0;
int base = pins[1];
int second = pins[3];
int fx_base = pins[13];
int fx_second = pins[12];

float notes[][7] = {
  { 23 },
  { 32.703, 65.406, 130.813, 261.626, 523.251, 1046.502, 1975.533 }, // C1
  { 34.648, 69.296, 138.591, 277.183, 554.365, 1108.731, 2217.461 }, // Db
  { 36.708, 73.416, 146.832, 293.665, 587.330, 1174.659, 2349.318 }, // D
  { 38.891, 77.782, 155.563, 311.127, 622.254, 1244.508, 2489.016 },  // Eb
  { 41.203, 82.407, 164.814, 329.628, 659.255, 1318.510, 2637.020 }, // E
  { 43.654, 87.307, 174.614, 349.228, 698.456, 1396.913, 2793.826 }, // F
  { 46.249, 92.499, 184.997, 369.994, 739.989, 1479.978, 2959.955 },// Gb
  { 48.999, 97.999, 195.998, 391.995, 783.991, 1567.982, 3135.963 },// G
  { 51.913, 103.826, 207.652, 415.305, 830.609, 1661.219, 3322.438 },// Ab
  { 55.000, 110.000, 220.000, 440.000, 880.000, 1760.000, 3520.000 },// A
  { 58.270, 116.541, 233.082, 466.164, 932.328, 1864.655, 3729.310 },// Bb
  { 61.735, 123.471, 246.942, 493.883, 987.767, 1975.533, 3951.066 },// B
  { 65.406, 130.813, 261.626, 523.251, 1046.502, 2093.005, 4186.009 }// C
};


int buttonState = 0;
EventDelay settingDelay;
void setup(){
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,HIGH);
  // In case of first run:
  int octave_test = EEPROM.read('octave_addr');
  if (octave_test==NULL) {
    firstRun();
  }
  
  Serial.begin(115200);
  for (int pullup = 0; pullup <= 13; pullup ++) {
    pinMode(pins[pullup], INPUT_PULLUP);
  }
  silenceMaker();
  kTremolo.setFreq(trem_speed);
  aGain1.setFreq(EEPROM.read(waveshaper_speed1_addr)); // use a float for low frequencies, in setup it doesn't need to be fast
  aGain2.setFreq(EEPROM.read(waveshaper_speed2_addr));

  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  longHoldDelay.set(EEPROM.read(long_hold_addr));
}

char master_gain = 1;
byte long_hold_gain;
unsigned int duration, attack, decay, sustain, release_ms;

void updateControl(){
  

    current_pin = 0;
    for (int pin = 1; pin <= 13; pin ++) {
      settingDelay.start();
      int pin_value;
      if (pins[pin] >= 14) {
        if (mozziAnalogRead(pins[pin]) < 100) {
          current_pin = pin;
          break;        
        }
      }
      else {
        if (digitalRead(pins[pin]) == LOW) {
          current_pin = pin;
          break;
        }
      }
      
     
    }
    //Serial.println(current_pin);

    int base_read = digitalRead(base);
    int second_read = digitalRead(second);
    int fx_base_read = mozziAnalogRead(fx_base);
    int fx_second_read = mozziAnalogRead(fx_second);

    if ((current_pin != 0)  ) {
      if ((base_read == LOW) && (second_read == LOW) && (master_gain == 0)) {
        //Serial.println('doing it now');
        settingChanger();
        digitalWrite(ledPin, HIGH);
      }
      else if ((fx_base_read < 100) && (fx_second_read < 100) && (master_gain == 0)) {
        fxSettingChanger();
        digitalWrite(ledPin, HIGH);
      }
      else {
        if (swell == 1) {
           envelope.setADLevels(240,200);
           envelope.setTimes(320,520,400000,40000);    
           envelope.noteOn();
           longHoldDelay.start(54000000);
        }
        else if (swell == 2) {
           envelope.setADLevels(240,200);
           envelope.setTimes(1820,1620,4000000,40000);    
           envelope.noteOn();
           longHoldDelay.start(540000000);          
        }
        else {
           envelope.setADLevels(240,200);
           envelope.setTimes(80,100,400000,40000);    
           envelope.noteOn();
           longHoldDelay.start(540000000);
        }
        envelope.update();
        long_hold_gain = envelope.next();
        Serial.println(long_hold_gain);


        int note = ((notes[current_pin][octave]));
        key_pressed = 1;
        aSin1.setFreq(note); // set the frequency
        aSaw1.setFreq(note); // set the frequency
        aTri1.setFreq(note); // set the frequency
        aSin2.setFreq((note - 2)); // set the frequency
        aSin3.setFreq((notes[current_pin][2])); // set the frequency        
        aSin4.setFreq((notes[current_pin][3])); // set the frequency
        aSin5.setFreq((notes[current_pin][4])); // set the frequency
        aSin6.setFreq((notes[current_pin][5])); // set the frequency
        master_gain = 1;
        long_hold_active = 1;
        tremStart();

      }
      if ((base_read == LOW) && (second_read == LOW)) {
        silenceMaker();
        miscDelay.start(100);
      }
      if ((fx_base_read < 100) && (fx_second_read < 100)) {
        silenceMaker();
        miscDelay.start(100);
      }
    }
    else {
      if ((long_hold_active > 0)) {
        // long_hold_active goes 1 at start, 2 in progress
        if (long_hold_active == 1 ) {

          envelope.setADLevels(200,long_hold_gain);
          if (long_hold == 1) {

           envelope.setTimes(20,20,160,220);    
           envelope.noteOn();
           longHoldDelay.start(440);
          }          
          else if (long_hold == 2) {

           envelope.setTimes(20,20,320,900);    
           envelope.noteOn();
           longHoldDelay.start(1260);
          }
          else {

           envelope.setTimes(20,20,20,20);    
           envelope.noteOn();
           longHoldDelay.start(80);
          }
         }

        envelope.update();
        long_hold_gain = envelope.next(); // this is where it's different to an audio rate envelope
        long_hold_active = 2;
        if (envelope.playing() == false) {
          silenceMaker();
        }
      }
      else {
        silenceMaker();
        current_pin = 0;
      }
    }
}

int updateAudio(){
  long returner = 0.000000000;
  int total = 0;
  int secondary_ins_gain = 180;
  if (sin_active == 1) {
    returner += (int) ((long)((long) 240 * aSin1.next()) >> 8);
  }
  if (saw_active == 1) {
    returner += (int) ((long)((long) 240 * aSaw1.next()) >> 8);
  }
  if (tri_active == 1) {
    returner += (int) ((long)((long) 240 * aTri1.next()) >> 8);
  }

  if (waveshaper_active == 1) {
  
    byte asig1 = (byte)128+((returner*((byte)128+aGain1.next()))>>8);
    byte asig2 = (byte)128+((returner*((byte)128+aGain2.next()))>>8);
    // get the waveshaped signals
    char awaveshaped1 = aCheby3rd.next(asig1);
    char awaveshaped2 = aCheby6th.next(asig2);
    // use a waveshaping table to squeeze 2 summed 8 bit signals into the range -244 to 243
    returner = aCompress.next(256u + awaveshaped1 + awaveshaped2);
    secondary_ins_gain = 120;
  }
  if (sin2_active == 1) {
    returner += (int) ((long)((long) secondary_ins_gain * aSin2.next()) >> 8);     
  }
  secondary_ins_gain = 140;
  if (sin3_active == 1) {
    returner += (int) ((long)((long) secondary_ins_gain * aSin3.next()) >> 8);
  }
  secondary_ins_gain = 60;
  if (sin4_active == 1) {
    returner += (int) ((long)((long) secondary_ins_gain * aSin4.next()) >> 8);
  }
  if (sin5_active == 1) {
    returner += (int) ((long)((long) secondary_ins_gain * aSin5.next()) >> 8);
  }
  if (sin6_active == 1) {
    returner += (int) ((long)((long) secondary_ins_gain * aSin6.next()) >> 8);
  }
  if (sin7_active == 1) {
    returner += (int) ((long)((long) secondary_ins_gain * aSin7.next()) >> 8);
  }
  if (sin8_active == 1) {
    returner += ((int)aSin8.next());
  }
  

   returner = (int) ((long)((long) long_hold_gain * returner) >> 8);


  if (trem_active == 1) {
    returner = (int)((long)((long) returner * tremGain.next()) >> 16);
  }
returner = aCompress.next(256u + returner);
return returner * master_gain ;// total;

}

void silenceMaker() {
  aSin1.setFreq(0);
  aSaw1.setFreq(0);
  aTri1.setFreq(0);
  aSin2.setFreq(0);
  aSin3.setFreq(0);
  aSin4.setFreq(0);
  aSin5.setFreq(0);
  aSin6.setFreq(0);
  aSin7.setFreq(0);
  aSin8.setFreq(0);
  aSin9.setFreq(0);        
  long_hold_active = 0;
  key_pressed = 0;
  long_hold_gain = 0;
  master_gain = 0;
  longHoldDelay.start(100);
}

void tremStart() {
  if (trem_active == 1) {
    unsigned int gain = (128u+kTremolo.next())<<8;
    tremGain.set(gain, AUDIO_RATE / CONTROL_RATE); // divide of literals should get optimised away
  }
}

void settingChanger() {
   silenceMaker();
   int base_read = digitalRead(base);
   int second_read = digitalRead(second);
   int count = 0;
   if (master_gain != 0) {
    return;
   }
   

     while ((digitalRead(base) == LOW) && (digitalRead(second) == LOW)) {
        master_gain = 0;
        if (count < 10) {
          digitalWrite(ledPin, LOW);
        }
        else {
          digitalWrite(ledPin, HIGH);
        }
        count++;
        if (count > 20) {
          count = 0;
        }
        delay(20);

      if ((digitalRead(pins[2]) == LOW)) {

        if (octave > 0) {
          
          EEPROM.write(octave_addr, octave - 1);
          octave = octave - 1;
          //Serial.println('Octave down');
        }
        settingChanged();
        
 
      }
      else if ((digitalRead(pins[4]) == LOW)) {
          if (EEPROM.read(octave_addr) <= max_octaves) {
            EEPROM.write(octave_addr, octave + 1);
            octave = octave + 1;
           // Serial.println('Octave up');

        }
        settingChanged();
        
     
     }
     else if ((digitalRead(pins[7]) == LOW)) {
       if (EEPROM.read(sin_addr) == true) {
         EEPROM.write(sin_addr, false);
         sin_active = 0;
        // Serial.print(0);

       }
       else {
         EEPROM.write(sin_addr, true);
         sin_active = 1;
       //  Serial.print(1);

       }
       settingChanged();
      

      
    }
    else if ((digitalRead(pins[9]) == LOW)) {
       if (EEPROM.read(saw_addr) == true) {
         EEPROM.write(saw_addr, false);
         saw_active = 0;
       //  Serial.print(0);

       }
       else {
         EEPROM.write(saw_addr, true);
         saw_active = 1;
       //  Serial.print(1);

       }
       settingChanged();
      
    
    }
    else if ((analogRead(pins[11]) < 100)) {
       if (EEPROM.read(tri_addr) == true) {
         EEPROM.write(tri_addr, false);
         tri_active = 0;
        // Serial.print(0);

       }
       else {
         EEPROM.write(tri_addr, true);
         tri_active = 1;
       //  Serial.print(1);

       }
       settingChanged();   

    }
    else if ((digitalRead(pins[5]) == LOW)) {
       if (EEPROM.read(sin2_addr) == true) {
         EEPROM.write(sin2_addr, false);
         sin2_active = 0;
       //  Serial.print(0);

       }
       else {
         EEPROM.write(sin2_addr, true);
         sin2_active = 1;
       //  Serial.print(1);

       }
       settingChanged();
      
    }
    else if ((digitalRead(pins[6]) == LOW)) {
       if (EEPROM.read(sin3_addr) == true) {
         EEPROM.write(sin3_addr, false);
         sin3_active = 0;
        // Serial.print(0);

       }
       else {
         EEPROM.write(sin3_addr, true);
         sin3_active = 1;
        // Serial.print(1);

       }
       settingChanged();
      
    }
    else if ((digitalRead(pins[8]) == LOW)) {
       if (EEPROM.read(sin4_addr) == true) {
         EEPROM.write(sin4_addr, false);
         sin4_active = 0;
        // Serial.print(0);

       }
       else {
         EEPROM.write(sin4_addr, true);
         sin4_active = 1;
        // Serial.print(1);

       }
       settingChanged();
      
    }
    else if ((digitalRead(pins[10]) == LOW)) {
       if (EEPROM.read(sin5_addr) == true) {
         EEPROM.write(sin5_addr, false);
         sin5_active = 0;
        // Serial.print(0);

       }
       else {
         EEPROM.write(sin5_addr, true);
         sin5_active = 1;
        // Serial.print(1);

       }
       settingChanged();
      
    }
    else if ((analogRead(pins[12]) < 100)) {
       if (EEPROM.read(sin6_addr) == true) {
         EEPROM.write(sin6_addr, false);
         sin6_active = 0;
        // Serial.print(0);

       }
       else {
         EEPROM.write(sin6_addr, true);
         sin6_active = 1;
        // Serial.print(1);

       }
       settingChanged();
      
    }    
    
    else if ((analogRead(pins[13]) < 100)) {
      settingReset();
     // Serial.print(1);
      settingChanged();
      

    }
  
    }
}

void fxSettingChanger() {

 int count = 0;
 silenceMaker();

 while ((analogRead(fx_base) < 100) && (analogRead(fx_second) < 100)) {
  master_gain = 0;
  //  Serial.println(mozziAnalogRead(fx_base));
  //  Serial.println(digitalRead(fx_base));
    if (count < 10) {
      digitalWrite(ledPin, LOW);
    }
    else {
      digitalWrite(ledPin, HIGH);
    }
    count++;
    if (count > 20) {
      count = 0;
    }
    delay(20);
    if ((digitalRead(pins[1]) == LOW)) {
      fxReset();
      settingChanged();   
    }
    else if ((digitalRead(pins[7]) == LOW)) {
      EEPROM.write(long_hold_addr, 0);
      long_hold = 0;
      settingChanged();   
    }
    else if ((digitalRead(pins[9]) == LOW)) {
      EEPROM.write(long_hold_addr, 1);
      long_hold = 1;
      settingChanged();   
    }    
    else if ((analogRead(pins[11]) < 100)) {
      EEPROM.write(long_hold_addr, 2);
      long_hold = 2;
      settingChanged();   
    }
    
    // SWELL
    else if ((digitalRead(pins[6]) == LOW)) {
      EEPROM.write(swell_addr, 0);
      swell = 0;
      settingChanged();   
    }
    else if ((digitalRead(pins[8]) == LOW)) {
      EEPROM.write(swell_addr, 1);
      swell = 1;
      settingChanged();   
    }    
    else if ((digitalRead(pins[10]) == LOW)) {
      EEPROM.write(swell_addr, 2);
      swell = 2;
      settingChanged();   
    }    
    else if ((digitalRead(pins[3]) == LOW)) {
      if (trem_active == 1) {
        EEPROM.write(trem_addr, 0);
        trem_active = 0;        
      }
      else {
        EEPROM.write(trem_addr, 1);
        trem_active = 1;
      }
      settingChanged();   
    }    
    else if ((digitalRead(pins[2]) == LOW)) {
      trem_speed = trem_speed - 1;
      EEPROM.write(trem_speed_addr, trem_speed);
      kTremolo.setFreq(trem_speed);
      settingChanged();   
    }
    else if ((digitalRead(pins[4]) == LOW)) {
      trem_speed = trem_speed + 1;
      EEPROM.write(trem_speed_addr, trem_speed);
      kTremolo.setFreq(trem_speed);
      settingChanged();   
    }

    else if ((digitalRead(pins[5]) == LOW)) {
      if (waveshaper_active == 1) {
        waveshaper_active = 0;
        EEPROM.write(waveshaper_addr, 0);
      }
      else {
        waveshaper_active = 1;
        EEPROM.write(waveshaper_addr, 1);
      }
      settingChanged();
    }
 }
}

void firstRun() {
  settingReset();
  fxReset();
}

void settingReset() {
    EEPROM.write(octave_addr, 1);
    EEPROM.write(sin_addr, 1);
    EEPROM.write(saw_addr, 0);
    EEPROM.write(tri_addr, 0);
    EEPROM.write(sin2_addr, 0);
    EEPROM.write(sin3_addr, 0);
    EEPROM.write(sin4_addr, 0);
    EEPROM.write(sin5_addr, 0);
    EEPROM.write(sin6_addr, 0);
    EEPROM.write(sin7_addr, 0);
    EEPROM.write(sin8_addr, 0);                    

    sin_active = 1;
    sin2_active = 0;
    saw_active = 0;
    tri_active = 0;
    sin3_active = 0;
    sin4_active = 0;
    sin5_active = 0;
    sin6_active = 0;
    sin7_active = 0;
    sin8_active = 0;            

    octave = 1;
}

void fxReset() {
    EEPROM.write(long_hold_addr, 1);
    EEPROM.write(trem_addr, 0);
    EEPROM.write(trem_speed_addr, 5.5d);
    kTremolo.setFreq(trem_speed);
    EEPROM.write(waveshaper_addr, 0);
    EEPROM.write(waveshaper_speed1_addr, 2.f);
    EEPROM.write(waveshaper_speed2_addr, .4f);    
    aGain1.setFreq(2);
    aGain2.setFreq((int) 0.4);
    EEPROM.write(swell_addr, 0);
    swell = 0;
    waveshaper_active = 0;
    long_hold = 1;
    trem_active = 0;
    trem_speed = 5.5d;
}

void exitSettings() {
  delay(250);
  silenceMaker();
  unPauseMozzi();
  silenceMaker();
  digitalWrite(ledPin, HIGH);
}

void settingChanged() {
  int count = 0;
  while (count < 3) {
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin,LOW);
    delay(750 * 10);
    count++;
  }
}

void loop(){

    audioHook(); // required here

}


