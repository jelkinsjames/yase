#include <iostream>
#include <math.h>
#include "fader.hh"
#include "yase.hh"

namespace yase {

  Fader::Fader(double min, double max) : min_val(min), max_val(max) {
    target = add_input("target");
    value = add_output("value");
  }

  void Fader::init() {
    outputs[value] = adjusted_target();
  }

  double Fader::adjusted_target() {
    return min_val + ( max_val-min_val ) * inputs[target] / 127.0;    
  }

  void Fader::update() {

    outputs[value] = outputs[value] - TS * FADER_GAIN * (outputs[value] - adjusted_target());

  }    

}

