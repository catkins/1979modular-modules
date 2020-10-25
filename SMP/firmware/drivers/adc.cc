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
// Driver for ADC.

#include "Buchla_Clouds/firmware/drivers/adc.h"

#include <stm32f4xx_conf.h>

namespace clouds {
  
void Adc::Init() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  
  DMA_InitTypeDef dma_init;
  ADC_CommonInitTypeDef adc_common_init;
  ADC_InitTypeDef adc_init;
  GPIO_InitTypeDef gpio_init;
  
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  gpio_init.GPIO_Pin |= GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOA, &gpio_init);
  
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOC, &gpio_init);

  // Use DMA to automatically copy ADC data register to values_ buffer.
  dma_init.DMA_Channel = DMA_Channel_0;
  dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[0];
  dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dma_init.DMA_BufferSize = ADC_CHANNEL_LAST;
  dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init.DMA_Mode = DMA_Mode_Circular;
  dma_init.DMA_Priority = DMA_Priority_High;
  dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &dma_init);
  DMA_Cmd(DMA2_Stream0, ENABLE);
  
  adc_common_init.ADC_Mode = ADC_Mode_Independent;
  adc_common_init.ADC_Prescaler = ADC_Prescaler_Div8;
  adc_common_init.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  adc_common_init.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
  ADC_CommonInit(&adc_common_init);
  
  adc_init.ADC_Resolution = ADC_Resolution_12b;
  adc_init.ADC_ScanConvMode = ENABLE;
  adc_init.ADC_ContinuousConvMode = DISABLE;
  adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  adc_init.ADC_DataAlign = ADC_DataAlign_Left;
  adc_init.ADC_NbrOfConversion = ADC_CHANNEL_LAST;
  ADC_Init(ADC1, &adc_init);
  
  /*// 168M / 2 / 8 / (10 x (480 + 20)) = 2.1kHz.
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_480Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_480Cycles); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_480Cycles); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 4, ADC_SampleTime_480Cycles); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, ADC_SampleTime_480Cycles); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 6, ADC_SampleTime_480Cycles); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 7, ADC_SampleTime_480Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 8, ADC_SampleTime_480Cycles); 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 9, ADC_SampleTime_480Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 10, ADC_SampleTime_480Cycles); 
  */
  // 168M / 2 / 8 / (10 x (480 + 20)) = 2.1kHz.
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_480Cycles);   //position CV
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_480Cycles);   //density CV
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_480Cycles);  //grain size POT
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 4, ADC_SampleTime_480Cycles);  //grain size CV
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 5, ADC_SampleTime_480Cycles);   //pitch CV
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_480Cycles);   //spread CV 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_480Cycles);   //feedback CV
  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_480Cycles);   //reverb CV
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 9, ADC_SampleTime_480Cycles);   //balance CV
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 10, ADC_SampleTime_480Cycles);  //texture CV


  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);
  Convert();
}

void Adc::Convert() {
  ADC_SoftwareStartConv(ADC1);
}

void Adc::DeInit() {
  ADC_DeInit();
}

}  // namespace clouds
