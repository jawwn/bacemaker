# bacemaker


The BaceMaker Foot Synth

Uses the Mozzi Library: http://sensorium.github.io/Mozzi/

Output on Pin 9, be sure to connect your preferred resistors so it's not ridiculously loud! 10K to ground should do it.

The Input pins use INPUT_PULLUP, so the other end of your switch will go to ground, as we're detecting a LOW signal.

This project assumes you are using a foot pedal that has 13 pedals, starting and ending with C.

Foot combinations:

Bottom C and D held together, enter into Oscillator Settings

While holding C1 and D:

Db - Main Octave Down

Eb - Main Octave Up
  ( Default is Octave 2 )
  
E - Toggle Octave 2 - Detuned

F - Toggle Octave 3

G - Toggle Octave 4

A - Toggle Octave 5

B - Toggle Octave 6

Gb - Toggle Sine Wave in Main Octave

Ab - Toggle Square Wave in Main Octave

Bb - Toggle Saw Wave in Main Octave

C2 - Reset Oscillator Settings



B and C2 Held together, enter into FX settings

While holding B and C2:

D - Toggle Tremolo

Db - Tremolo Speed Down

Eb - Tremolo Speed Up

E - Waveshaper (experimental, doesn't sound that great)

F - Swell Off

G - Short Swell

A - Long Swell

Gb - Sustain Off

Ab - Short Swell

Bb - Long Swell

C1 - Reset FX settings


Settings are saved in the EEPROM when changed, so they'll restore when you turn on the Bacemaer
