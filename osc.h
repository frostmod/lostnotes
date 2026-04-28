#pragma once
/*
    BSD 3-Clause License

    Copyright (c) 2023, KORG INC.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//*/

/*
 *  File: osc.h
 *
 *  Dummy oscillator template instance.
 *
 */

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <climits>
#include <vector>
#include "unit_osc.h"   // Note: Include base definitions for osc units
#include "oscillator.h"

class Osc {
 public:
  /*===========================================================================*/
  /* Public Data Structures/Types/Enums. */
  /*===========================================================================*/
  bool noteGates[128];
  float notePhase[128];
  float notePitch[128];
  float notePhaseInc[128];
  int activeNoteCount = 0;

  /*===========================================================================*/
  /* Lifecycle Methods. */
  /*===========================================================================*/

  Osc(void) {}
  ~Osc(void) {} // Note: will never actually be called for statically allocated instances
  void buildPhaseIncTable() {
    for (uint8_t note = 0; note < 128; note++) {
      notePitch[note] = osc_notehzf(note);
      notePhaseInc[note] = notePitch[note] / 48000.f;
    }
  }

  inline int8_t Init(const unit_runtime_desc_t * desc) {
    if (!desc)
      return k_unit_err_undef;
    
    // Note: make sure the unit is being loaded to the correct platform/module target
    if (desc->target != unit_header.target)
      return k_unit_err_target;
    
    // Note: check API compatibility with the one this unit was built against
    if (!UNIT_API_IS_COMPAT(desc->api))
      return k_unit_err_api_version;

    // Check compatibility of samplerate with unit, for NTS-1 MKII should be 48000
    if (desc->samplerate != 48000)
      return k_unit_err_samplerate;

    // Check compatibility of frame geometry
    // Note: NTS-1 mkII oscillators can make use of the audio input depending on the routing options in global settings, see product documentation for details.
    if (desc->input_channels != 2 || desc->output_channels != 1)  // should be stereo input / mono output
      return k_unit_err_geometry;

    // Note: SDRAM is not available from the oscillator runtime environment
    
    // Cache the runtime descriptor for later use
    runtime_desc_ = *desc;
    
    buildPhaseIncTable();


    return k_unit_err_none;
  }

  inline void Teardown() {
    // Note: cleanup and release resources if any
  }

  inline void Reset() {
    // Note: Reset effect state, excluding exposed parameter values.
  }

  inline void Resume() {
    // Note: Effect will resume and exit suspend state. Usually means the synth
    // was selected and the render callback will be called again
  }

  inline void Suspend() {
    // Note: Effect will enter suspend state. Usually means another effect was
    // selected and thus the render callback will not be called
  }

  /*===========================================================================*/
  /* Other Public Methods. */
  /*===========================================================================*/

  fast_inline void Process(const float * in, float * out, size_t frames) {
    const unit_runtime_osc_context_t *ctxt = static_cast<const unit_runtime_osc_context_t *>(runtime_desc_.hooks.runtime_context);
    const float * __restrict in_p = in;
    float * __restrict out_p = out;
    const float * out_e = out_p + frames;  // assuming mono output

    activeNoteCount = 0;
    int activeNotes[128];
    for (int n = 0; n < 128; n++) {
      if (noteGates[n]) {
        activeNotes[activeNoteCount++] = n;
      }
    }
    float mult = 0.1f;
 
    for (; out_p != out_e; in_p += 2, out_p += 1) {
      // Process/generate samples here
      float sample = 0.0f;

      for (int idx = 0; idx < activeNoteCount; idx++) {
        int n = activeNotes[idx];
        notePhase[n] += notePhaseInc[n];
        notePhase[n] -= (uint32_t)notePhase[n]; // wrap to [0,1)
        sample += (saw.processSample(notePhase[n], notePhaseInc[n]) * mult);
      }

      *out_p = sample;
    }
  }

  inline void setParameter(uint8_t index, int32_t value) {
    params[index] = value;
  }

  inline int32_t getParameterValue(uint8_t index) const {
    return params[index];
  }

  inline const char * getParameterStrValue(uint8_t index, int32_t value) const {
    return nullptr;
  }

  inline void setTempo(uint32_t tempo) {
    // const float bpmf = (tempo >> 16) + (tempo & 0xFFFF) / static_cast<float>(0x10000);
    (void)tempo;
  }

  inline void tempo4ppqnTick(uint32_t counter) {
    (void)counter;
  }

  inline void NoteOn(uint8_t note, uint8_t velo) {
    (uint8_t)velo;
    noteGates[note] = true;
  }


  inline void NoteOff(uint8_t note) {
    noteGates[note] = false;
  }

  inline void AllNoteOff() {
    for (int n = 0; n < 128; n++) {
      noteGates[n] = false;
    }
  }

  inline void PitchBend(uint8_t bend) {
    (uint8_t)bend;
  }

  inline void ChannelPressure(uint8_t press) {
    (uint8_t)press;
  }

  inline void AfterTouch(uint8_t note, uint8_t press) {
    (uint8_t)note;
    (uint8_t)press;
  }

  
  /*===========================================================================*/
  /* Static Members. */
  /*===========================================================================*/
  
 private:
  /*===========================================================================*/
  /* Private Member Variables. */
  /*===========================================================================*/

  std::atomic_uint_fast32_t flags_;

  unit_runtime_desc_t runtime_desc_;
  int32_t params[10];

  PolyBLEPSaw saw;



  /*===========================================================================*/
  /* Private Methods. */
  /*===========================================================================*/

  /*===========================================================================*/
  /* Constants. */
  /*===========================================================================*/
};
