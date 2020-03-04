#include "adc1registers.hpp" //for ADC1
#include "adccommonregisters.hpp" //for ADCCommon
#include "gpioaregisters.hpp"  //for Gpioa
#include "gpiocregisters.hpp"  //for Gpioc
#include "rccregisters.hpp"    //for RCC
#include "tim2registers.hpp"   //for TIM2
#include <iostream>

using namespace std;

extern "C"
{
int __low_level_init(void)
{
  //Switch on external 8 MHz oscillator
  RCC::CR::HSEON::On::Set();
  while(!RCC::CR::HSERDY::Ready::IsSet())
  {}; 
  //Switch system clock on external oscillator
  RCC::CFGR::SW::Hse::Set();
  while (!RCC::CFGR::SWS::Hse::IsSet())
  {};
    
   //---------------ADC1 setup--------------------------------------------------
  //Switch on clock on ADC1
  RCC::APB2ENR::ADC1EN::Enable::Set();
   //Switch On internal tempearture sensor  
  ADC_Common::CCR::TSVREFE::Enable::Set();
  //Resolution of the conversion.
  ADC1::CR1::RES::Bits12::Set();
   //Set single conversion mode
  ADC1::CR2::CONT::SingleConversion::Set();
  ADC1::CR2::EOCS::SingleConversion::Set();   
  // Set laentgh of conversion sequence
  ADC1::SQR1::L::Conversions2::Set();
  // Connect first conversion on Channel 17 ADC1_IN17 (VREFINT)
  ADC1::SQR3::SQ1::Channel17::Set();
  // Connect second conversion on Channel 18 ADC1_IN18 (temperature sensor) 
  ADC1::SQR3::SQ2::Channel18::Set();
  // Set 84 cycles sample rate for channels
  ADC1::SMPR1::SMP17::Cycles84::Set();
  ADC1::SMPR1::SMP18::Cycles84::Set();
  // Enable scan mod
  ADC1::CR1::SCAN::Enable::Set();
  // Enable ADC1
  ADC1::CR2::ADON::Enable::Set();
  
  return 1;
}
}

// Temperature sensor calibration values
constexpr size_t TsCal1Addr = 0x1FFF7A2C; // at temperature of 30 °C
constexpr size_t TsCal2Addr =  0x1FFF7A2E; //  at temperature of 110 °C
// Internal reference voltage calibration values
constexpr size_t VrefCalAddr =  0x1FFF7A2A; //Raw data acquired at temperature of 30 °C

// Temperature sensor calibration values
volatile uint16_t *TsCal1Pointer = reinterpret_cast<volatile uint16_t*>(TsCal1Addr) ;
volatile uint16_t *TsCal2Pointer = reinterpret_cast<volatile uint16_t*>(TsCal2Addr) ;
uint16_t TsCal12 = (*TsCal1Pointer);
uint16_t TsCal22 = (*TsCal2Pointer);
float TsCal1 = TsCal12; 
float TsCal2 = TsCal22;

// Internal reference voltage calibration values
volatile uint16_t *VrefCalPointer = reinterpret_cast<volatile uint16_t*>(VrefCalAddr) ;
uint16_t VrefCal2 = (*VrefCalPointer);
float VrefCal = VrefCal2;

constexpr float K = 3.3F/(4096.0F*0.0025F); 
constexpr float B = 25.0F - 0.76F/0.0025F ;

int main()
{
  uint32_t ADC_C = 0U ; //for getting reference voltage data from ADC
  uint32_t Vref = 0U ; //for getting reference voltage data from ADC
  float temperature = 0.0F ;
  float temperature1 = 0.0F ;
  float temperature2 = 0.0F ;
  float DEGREE_30 = 30.0F;
  
  for(;;)    
  {
    //Start conversion
    ADC1::CR2::SWSTART::On::Set();
    
    //----------Reference voltage-----------------------------------------------
    // wait until Conversion is not complete 
    while(!ADC1::SR::EOC::ConversionComplete::IsSet())
    {};
    Vref =  ADC1::DR::Get(); //Get data from ADC;
    
    //----------Temperature-----------------------------------------------------
    // wait until Conversion is not complete 
    while(!ADC1::SR::EOC::ConversionComplete::IsSet())
    {};
    ADC_C =  ADC1::DR::Get(); //Get data from ADC;
    //without calibration
    temperature = ADC_C * K + B ; //Convert ADC counts to temperature
    
    //temperature sensor calibration
    temperature1 = ADC_C * (110-30)/(TsCal2 - TsCal1) + (30 - (110-30)*TsCal1/(TsCal2- TsCal1) );
    
    //temperature sensor and internal reference voltage calibration
    temperature2 = ADC_C * (110-30)*VrefCal/(Vref*(TsCal2-TsCal1)) + DEGREE_30 - (110-30)*TsCal1/(TsCal2-TsCal1) ; //Convert ADC counts to temperature

    //----------I/O-------------------------------------------------------------
    cout << "\nTemperature \nWithoutcalibration: " << temperature << " \nWith temperature sensor : " << temperature1 << " \nWith temperature sensor and reference voltage calibration: " << temperature2 << endl ;
  }
return 0;  
}