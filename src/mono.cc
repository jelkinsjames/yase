#include "mono.hh"
#include "yase.hh"

namespace yase {

  Mono::Mono(Midi &midi_module, json &midi_map, string button_device_name, int kp, int cp) : 
                 Synthesizer(button_device_name), 
                 mixer(3),
                 mod_mixer1(2),
                 mod_mixer2(2),
                 filter_env_mixer(2),
                 midi(midi_module),
                 midi_map(midi_map),
                 keyboard_port(kp),
                 controller_port(cp)
  {   

    // OUTPUT
    signal = add_output("signal");

    // COMPONENTS
    add(osc[0]);
    add(osc[1]);      
    add(osc[2]);
    add(mixer);     
    add(mod_mixer1);
    add(mod_mixer2);  
    add(lfo);
    add(env);      
    add(filter);     
    add(gain);
    add(osc2_lfo_gain); 
    add(filter_env_mixer);
    add(filter_env);
    add(seq);    
    add(noop);

    // CONNECTIONS
    for ( int i=0; i<3; i++ ) 
        connect(osc[i], "signal", mixer, i);

    connect( mixer,  "signal", filter, "signal");
    connect( filter, "signal", env,    "signal");
    connect( env,    "signal", gain,   "signal");
    connect( lfo,    "signal", mod_mixer1, 0 );
    connect( osc[0], "signal", mod_mixer1, 1 );
    connect( mod_mixer1, "signal", osc[1], "modulation" );
    connect( lfo,    "signal", mod_mixer2, 0 );
    connect( osc[1], "signal", mod_mixer2, 1 );
    connect( mod_mixer2, "signal", osc[2], "modulation" );
    connect( lfo, "signal", osc2_lfo_gain, "signal" );
    connect( osc2_lfo_gain, "signal", osc[0], "modulation" );
    connect( filter_env, "signal", filter_env_mixer, 1 );
    connect( filter_env_mixer, "signal", filter, "frequency" );

    // DEFAULT INPUT
    filter_env_mixer.set_input(0, 1);    

    // MIDI KEYBOARD LISTENERS
    listen(MIDI_KEYDOWN, keyboard_port, [&] (const Event &e) { keydown(e); });
    listen(MIDI_KEYUP, keyboard_port, [&] (const Event &e) { keyup(e); });

    // OSCILLATOR TYPE SELECTIONS
    for ( int i=0; i<3; i++ ) {
        listen(MIDI_MOD, [&,i] (const Event &e) { select(e, i); });
    }    

    // LFO CONTROLS
    control(lfo, "frequency", 0.01, 10, midi_map["lfo_freq"]);
    control(lfo, "amplitude", 0, 10,    midi_map["lfo_amp"]);

    // OSCILLATOR MODULATION CONTROLS
    // CHECK: DOES EACH OSCILLATOR HAVE AN LFO GAIN?
    control(mod_mixer1, mod_mixer1.amplitude_index(0), 0, 5, midi_map["mod_mixer_lfo_ctrl"][0]); 
    control(mod_mixer1, mod_mixer1.amplitude_index(1), 0, 5, midi_map["mod_mixer_mod_ctrl"][0]);
    control(mod_mixer2, mod_mixer2.amplitude_index(0), 0, 5, midi_map["mod_mixer_lfo_ctrl"][1]);
    control(mod_mixer2, mod_mixer2.amplitude_index(1), 0, 5, midi_map["mod_mixer_mod_ctrl"][1]);
    control(osc2_lfo_gain, "amplitude", 0, 10, 50);

    // OSCILLATOR AMPLITUDES AND MIXING
    for (int i=0; i<3; i++) {
        control(mixer, i+3,          0, 1, midi_map["amplitudes"][i]);
        control(osc[i], "tuning",   -7, 8, midi_map["tunings"][i]);
        control(osc[i], "harmonic", -2, 3, midi_map["harmonics"][i]);
    }        

    // FILTER CONTROLS
    control(filter, "resonance", 0.1, 20, midi_map["filter_resonance"]);
    control(filter_env, "attack",  0.005, 1, midi_map["filter_env"]["A"]);
    control(filter_env, "decay",   0.005, 1, midi_map["filter_env"]["D"]);
    control(filter_env, "sustain", 0,     1, midi_map["filter_env"]["S"]);
    control(filter_env, "release", 0.005, 1, midi_map["filter_env"]["R"]);
    control(filter_env_mixer, filter_env_mixer.amplitude_index(0), 10, 6000, midi_map["filter_freq"] );
    control(filter_env_mixer, filter_env_mixer.amplitude_index(1), 10, 6000, midi_map["filter_eg_amt"] );

    // MAIN ENVELOPE
    control(env, "attack",  0.005, 1, midi_map["env"]["A"]);
    control(env, "decay",   0.005, 1, midi_map["env"]["D"]);
    control(env, "sustain", 0,     1, midi_map["env"]["S"]);
    control(env, "release", 0.005, 1, midi_map["env"]["R"]);

    // VOLUME
    control(gain, "amplitude", 0, 0.25, midi_map["volume"]);

    // BUTTONS FOR FILTER SELECTION
    mutex({midi_map["buttons"]["lpf"], midi_map["buttons"]["hpf"]}, {
          [&] (const Event &e) { filter.set_type("lpf"); }, 
          [&] (const Event &e) { filter.set_type("hpf"); }
          });

    toggle(midi_map["buttons"]["filter_toggle"], [&] (const Event &e) {
              filter.toggle();
          }, true);

    // KILLER RANDOMIZE BUTTON
    momentary(midi_map["buttons"]["randomize"], [&] (const Event &e) {
          randomize_faders();
          gain.set_input("amplitude", 0.1);
    });    

    // SEQUENCER KEYBOARD EVENTS
    listen(MIDI_KEYDOWN, keyboard_port, [&] (const Event &e) { seq.keydown(e); });
    listen(MIDI_KEYUP, keyboard_port, [&] (const Event &e) { seq.keyup(e); });

    // SEQUENCERS BUTTONS
    json button = midi_map["buttons"];

    mutex({
        button["stop"], 
        button["record"],
        button["play"] }, {
        [&] (const Event &e) { seq.stop(); },
        [&] (const Event &e) { seq.record(); },
        [&] (const Event &e) { seq.play(); }
    });

    momentary(button["rest"], [&] (const Event &e) { seq.insert_rest(); });
    momentary(button["reset"], [&] (const Event &e) { seq.reset(); });
    momentary(button["clear"], [&] (const Event &e) { seq.clear(); });
    momentary(button["decrease_tempo"], [&] (const Event &e) { seq.decrease_tempo(20); });
    momentary(button["increase_tempo"], [&] (const Event &e) { seq.increase_tempo(20); });
    momentary(button["decrease_duration"], [&] (const Event &e) { seq.decrease_duration(0.1); });
    momentary(button["increase_duration"], [&] (const Event &e) { seq.increase_duration(0.1); });      

  }

  void Mono::inject(Event e) {
      noop.emit(e);
  }

  void Mono::init() {
      Synthesizer::init();
  }

  void Mono::update() {
    Synthesizer::update();
    gain.copy_outputs(*this);
  }   

  void Mono::select(const Event &e, int i) {
    if ( e.id == midi_map["osc_selectors"][i] ) osc[i].select(e.value / 127.0);     
  } 

  void Mono::keydown(const Event &e) {

        for ( int i=0; i<3; i++ ) {
            osc[i].set_input("frequency", e.frequency());
        }

        filter.set_input("offset", e.frequency());
        filter.recalculate(); 
        env.set_input("velocity", e.value / 127.0);
        env.trigger();
        filter_env.trigger();
        keys.push_back(e.id);

  }

  void Mono::keyup(const Event &e) {

        keys.erase(std::remove(keys.begin(), keys.end(), e.id), keys.end());

        if ( keys.size() == 0 ) {
            env.release();
            filter_env.release();
        } else {
            Event temp(MIDI_KEYDOWN, keys.back(), 0, 0);
            for ( int i=0; i<3; i++ ) {
                osc[i].set_input("frequency", temp.frequency());
            }
            filter.set_input("offset", temp.frequency());
            filter.recalculate(); 
        }

    }

}