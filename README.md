# ncPlayMidi
ncurses userland MIDI file player &amp; decoder

This was planned to be a part of a bigger project ...it's at the "well it works" stage - but don't hold your breath for a polished end-product

plug in your USB->MIDI cable and `./midp song.mid /dev/mididevice`

```
# Open port "/dev/null"
# Play "mid/TubularBells1XG.mid" (format 1)
 TubularBells1XG.mid     Time: 7/8 (12 clok/clik, 8 32nd/qn)    Key: C
 160.05BPM (  781 uS/dt,  480 dt/qn)    100% -> 160.05BPM ( 781 dt/qn)

 1              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Bright Piano    │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 2              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Music Box       │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 3              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Fingered Bass   │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 4              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Perc. Organ     │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 5              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Pad: Choir      │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 6              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Steel Guitar    │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 7              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
OverDrive Gtr   │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 8              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Tubular Bells   │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
 9              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
                │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
10              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Celesta         │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
11              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
Pad: Choir      │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
12              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
                │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
13              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
                │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
14              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
                │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
15              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
                │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
16              │1█ █ │ █ █ █ │2█ █ │ █ █ █ │3█ █ │ █ █ █ │4█ █ │ █ █ █ │5█ █ │ █ █ █ │6█ █ │ █ █ █ │7█ █ │ █ █ █ │8█ █ │ █ █ █ │
                │c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│c│d│e│f│g│a│b│
                ┕─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
# Send MIDI "Panic"
# Close port "/dev/null"
# Unload Slot[0] - "mid/TubularBells1XG.mid"
# Destroyed SMF slots
```
