// 
// YASE Gain Module Implementation
// 
// Copyright (C) 2022 Eric Klavins
// This file is part of YASE
// 
// YASE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// YASE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
// 
// You should have received a copy of the GNU General Public License along
// with YASE. If not, see <https://www.gnu.org/licenses/>.
// 

#include "gain.hh"
#include "yase.hh"

namespace yase {

  int q = 0;

  Gain::Gain() {
    signal = add_input("signal");
    signal = add_output("signal");
    amplitude = add_input("amplitude");
  }

  void Gain::init() {
    set_input(amplitude, 0.5);
  }

  void Gain::update() {
    outputs[signal] = inputs[amplitude] * inputs[signal];      
  }    

}

