// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Calibration settings.

#include "Buchla_Clouds/firmware/cv_scaler.h"

#include <algorithm>
#include <cmath>

#include "stmlib/dsp/dsp.h"

#include "Buchla_Clouds/firmware/resources.h"

namespace clouds {

using namespace std;

/* static */
CvTransformation CvScaler::transformations_[ADC_CHANNELS_TOTAL] = {
  // ADC_POSITION_CV,
  { false, true, 0.05f },
  // ADC_DENSITY_CV,
  { false, true, 0.01f },
  // ADC_SIZE_GRAIN_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_SIZE_GRAIN_CV,
  { false, true, 0.1f },
  // ADC_PITCH_CV,
  //{ true, true, 1.00f },
  { false, false, 1.00f },
  // ADC_SPREAD_CV,
  { false, true, 0.2f },
  // ADC_FEEDBACK_CV,
  { false, true, 0.2f },
  // ADC_REVERB_CV,
  { false, true, 0.2f },
  // ADC_BALANCE_CV,
  { false, true, 0.2f },
  // ADC_TEXTURE_CV,
  { false, true, 0.01f },
  // ADC_POSITION_POTENTIOMETER,
  { false, false, 0.05f },
  // ADC_PITCH_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_DENSITY_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_TEXTURE_POTENTIOMETER,
  { false, false, 0.01f },
  // ADC_BALANCE_POTENTIOMETER,
  { false, false, 1.00f },
  // ADC_SPREAD_POTENTIOMETER,
  { false, false, 0.05f },
  // ADC_FEEDBACK_POTENTIOMETER,
  { false, false, 0.05f },
  // ADC_REVERB_POTENTIOMETER,
  { false, false, 0.05f },
};

void CvScaler::Init(CalibrationData* calibration_data) {
  adc_.Init();
  pots_adc_.Init();
  gate_input_.Init();
  calibration_data_ = calibration_data;
  fill(&smoothed_adc_value_[0], &smoothed_adc_value_[ADC_CHANNELS_TOTAL], 0.0f);
  note_ = 0.0f;
  
  fill(&previous_capture_[0], &previous_capture_[kAdcLatency], false);
  fill(&previous_gate_[0], &previous_gate_[kAdcLatency], false);
}

void CvScaler::Read(Parameters* parameters) {
  pots_adc_.Scan();
  for (int8_t i = 0; i < ADC_CHANNELS_TOTAL; ++i) {
    const CvTransformation& transformation = transformations_[i];
    float value; 
    if (i < 10) {
        value = adc_.float_value(i);
    } else {
        value = pots_adc_.float_value(i - ADC_CHANNEL_LAST);
    }
    if (transformation.flip) {
      value = 1.0f - value;
    }
    if (transformation.remove_offset) {
      value -= calibration_data_->offset[i];
    }
    smoothed_adc_value_[i] += transformation.filter_coefficient * \
        (value - smoothed_adc_value_[i]);
  }
  
  float position = smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_POSITION_POTENTIOMETER];
  position += smoothed_adc_value_[ADC_POSITION_CV] * 2.0f;
  CONSTRAIN(position, 0.0f, 1.0f);
  parameters->position = position;

  float texture = smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_TEXTURE_POTENTIOMETER];
  texture += smoothed_adc_value_[ADC_TEXTURE_CV] * 2.0f;
  CONSTRAIN(texture, 0.0f, 1.0f);
  parameters->texture = texture;

  float density = smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_DENSITY_POTENTIOMETER];
  density += smoothed_adc_value_[ADC_DENSITY_CV] * 2.0f;
  CONSTRAIN(density, 0.0f, 1.0f);
  parameters->density = density;

  float size = smoothed_adc_value_[ADC_GRAIN_POTENTIOMETER];
  size += smoothed_adc_value_[ADC_GRAIN_CV] * 2.0f;
  CONSTRAIN(size, 0.0f, 1.0f);
  parameters->size = size;

  float dry_wet = smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_BALANCE_POTENTIOMETER];
  dry_wet += smoothed_adc_value_[ADC_BALANCE_CV] * 2.0f;
  dry_wet = dry_wet * 1.05f - 0.025f;
  CONSTRAIN(dry_wet, 0.0f, 1.0f);
  parameters->dry_wet = dry_wet;

  float reverb_amount = smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_REVERB_POTENTIOMETER];
  reverb_amount += smoothed_adc_value_[ADC_REVERB_CV] * 2.0f;
  CONSTRAIN(reverb_amount, 0.0f, 1.0f);
  parameters->reverb = reverb_amount;

  float feedback = smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_FEEDBACK_POTENTIOMETER];
  feedback += smoothed_adc_value_[ADC_FEEDBACK_CV] * 2.0f;
  CONSTRAIN(feedback, 0.0f, 1.0f);
  parameters->feedback = feedback;

  float stereo_spread = smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_SPREAD_POTENTIOMETER];
  stereo_spread += smoothed_adc_value_[ADC_SPREAD_CV] * 2.0f;
  CONSTRAIN(stereo_spread, 0.0f, 1.0f);
  parameters->stereo_spread = stereo_spread;
  
  parameters->pitch = stmlib::Interpolate(
      lut_quantized_pitch,
      smoothed_adc_value_[ADC_CHANNEL_LAST + ADC_PITCH_POTENTIOMETER],
      1024.0f);
  
  float note = calibration_data_->pitch_offset;
  note += smoothed_adc_value_[ADC_PITCH_CV] * calibration_data_->pitch_scale;
  if (fabs(note - note_) > 0.5f) {
    note_ = note;
  } else {
    ONE_POLE(note_, note, 0.2f)
  } 
  parameters->pitch += note_;
  CONSTRAIN(parameters->pitch, -48.0f, 48.0f);
  
  gate_input_.Read();
  if (gate_input_.freeze_rising_edge()) {
    parameters->freeze = true;
  } else if (gate_input_.freeze_falling_edge()) {
    parameters->freeze = false;
  }

  /*
  parameters->trigger = previous_trigger_[0];
  parameters->gate = previous_gate_[0];
  for (int i = 0; i < kAdcLatency - 1; ++i) {
    previous_trigger_[i] = previous_trigger_[i + 1];
    previous_gate_[i] = previous_gate_[i + 1];
  }
  previous_trigger_[kAdcLatency - 1] = gate_input_.trigger_rising_edge();
  previous_gate_[kAdcLatency - 1] = gate_input_.gate();
  */

  parameters->capture = previous_capture_[0] | capture_button_flag_;
  if (capture_button_flag_ == true) {
    capture_button_flag_ = false;
  }
  parameters->gate = previous_gate_[0];
  for (int i = 0; i < kAdcLatency - 1; ++i) {
    previous_capture_[i] = previous_capture_[i + 1];
    previous_gate_[i] = previous_gate_[i + 1];
  }
  previous_capture_[kAdcLatency - 1] = gate_input_.capture_rising_edge();
  previous_gate_[kAdcLatency - 1] = gate_input_.gate();
  
  adc_.Convert();
//}
}
}  // namespace clouds
