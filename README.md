# YASE: Yet Another Synthesizer Engine

YASE is a modular digital audio synthesis and processing library. You can use it to make sounds, sequences, and effects. Literally any digital synthesis is available, from frequency modulation, wavetables, physical modeling, sampling, and so on. External audio devices, such as modular synthesizers and effects, can easily be incorporated with a based audio interface. With it's MIDI capabilities, you can control your creations with various MIDI controllers.

While there are literally dozens of similar libraries available, we believe YASE hits a sweet spot because it is
- Written in modern C++
- Fast and scalable
- Easy to use and extend
- Very, very modular
- Extensible
- Open source

Have fun and share what you make!

## Example

Here's an example of a super basic YASE program. 

```cpp
#include "yase.hh"

using namespace yase;

int main(int argc, char * argv[]) {

    Sine sine1, sine2;
    Audio audio;
    Container synth;
    
    sine1.set_input("frequency", 440);
    sine2.set_input("frequency", 441);

    synth.connect(sine1,"signal",audio,"left")
         .connect(sine2,"signal",audio,"right");

    synth.run(UNTIL_INTERRUPTED);

    return 0; 

}
```

The first three lines of the main function declare some modules: two sine wave sources, an audio interface, and a container to put everything into. The next block sets the frequency inputs of the two sine waves to be 440 Hz and 441 Hz respectively. The `connect` statements connect the output signals of the sine waves to the left and right inputs of the audio interface. Finally, the `run` statement runs the program until the user presses `Ctrl-C`. The result is a pleasing sound. Try it!

One thing to notice about modules is that their inputs and outputs are named by strings during set up. This makes working with I/O easy and straightforward. When you make your own modules, you can add as many inputs and outputs as you would like with whatever names make sense for your use case. If you use the wrong name, YASE gives you an easy to understand error message. Once all the connections are made and the container (named `synth` in this case) is run, YASE uses integer indices to the I/O so there is no string matching at run time. These two *setup* and *run* phases are common to all YASE programs. 

## Building

To date, Yase has been developed on MacOS 11.6.1. The following are required:
- 'git', 'g++', and 'make', part of the MacOS Development Tools.
- Brew, for installing things: https://brew.sh/
- PortAudio, which can be installed with 'brew install portaudio'
- RtMidi:, which can be installed with 'brew install rtmidi'
- Google Test: which can be installed with 'brew install googletest'
- Niels Lohman's JSON library, which should be unpacked and put somewhere on your computer. You should then edit the Makefiles to point to the right directory. https://github.com/nlohmann/json
- FFTW, which can be downloaded here: https://www.fftw.org/download.html. Put this in yase/third-party, compile it, and install it.

On my Mac, things are installed in strange places. Doing the following helps the linker work right.
```
export PATH="/usr/bin:$PATH"
```

Once all the prerequisites are installed, get the code and compile it

```bash
git clone https://github.com/klavins/yase.git
cd yase
make
```

Assuming no errors, you should be able to hear sound if you run
```bash
./examples/bin/sine_wave
```
***Note: We recommend that when you work on audio projects you keep your volume very low until you are sure your code works and isn't producing ear-splitting, speaker cone damaging signals.***
