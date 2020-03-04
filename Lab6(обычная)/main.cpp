#include "adc1registers.hpp" //for ADC1
#include "adccommonregisters.hpp" //for ADCCommon
#include "gpioaregisters.hpp"  //for Gpioa
#include "gpiocregisters.hpp"  //for Gpioc
#include "rccregisters.hpp"    //for RCC
#include "tim2registers.hpp"   //for TIM2
#include <iostream>


extern "C"
{
int __low_level_init(void)
{

  //Switch on external 8 MHz oscillator
 
  RCC::CR::HSEON::On::Set(); //podkluchili vneshnii istrochnik
  while (!RCC::CR::HSERDY::Ready::IsSet() )
  {
  };
  
  //Switch system clock on external oscillator
  
    RCC::CFGR::SW::Hse::Set();
    while ( !RCC::CFGR::SWS::Hse::IsSet( ))
    { 
    };
    
    //Switch on clock on PortA a
    RCC::AHB1ENR::GPIOAEN::Enable::Set(); // 
    
     
    // ************** Setup TIM2 ***********
    // Set Devider PSC to count every 1 ms
   
    // Set ARR to 5 seconds  overflow
   
    // Clear Overdlow event flag  
   
    // Reset counter
   
    // Enable TIM2 to count
   
    
    //********* ADC1 setup
    
    //Switch on clock on ADC1
    RCC::APB2ENR::ADC1EN::Enable::Set(); //acp k istochiky tactirovaniya
    RCC::AHB1ENR::GPIOAEN::Enable::Set() ; //vkl port A
    RCC::AHB1ENR::GPIOCEN::Enable::Set() ; //vkl port C
    
    //Switch On internal tempearture sensor
    ADC_Common::CCR::TSVREFE::Enable::Set(); //temperat. senser
    
    //pport A
    
    GPIOA::MODER::MODER0::Analog::Set();    // acp vxod
    
    GPIOA::MODER::MODER5::Output::Set();  //vkluchenie portov
    GPIOC::MODER::MODER5::Output::Set();  //vkluchenie portov
    GPIOC::MODER::MODER8::Output::Set();  //vkluchenie portov
    GPIOC::MODER::MODER9::Output::Set();  //vkluchenie portov
    
      
    //Set single conversion mode
    ADC1::CR2::CONT::SingleConversion::Set(); // cont porkl
    ADC1::CR2::EOCS::SingleConversion::Set(); //eosk vklichen
     
    // Set 84 cycles sample rate for channel 18
    ADC1::SQR1::L::Conversions2::Set(); // kolvo izmerenii
    
    ADC1::SQR3::SQ2::Channel0::Set(); //podkluchili datchik na kanal 18
    ADC1::SQR3::SQ1::Channel18::Set(); //podkluchili datchik na kanal 18
   
    ADC1::SMPR2::SMP0::Cycles480::Set(); //speed diskretizacii
    ADC1::SMPR1::SMP18::Cycles480::Set(); //speed diskretizacii
    
    ADC1::CR1::SCAN::Enable::Set();
    
    ADC1::CR1::RES::Bits12::Set(); //ystanovka 12bitov

    // Enable ADC1
    ADC1::CR2::ADON::Enable::Set(); 
  
  return 1;
}
}

constexpr float B1 = 0; // see datacheet (page 226) and calculate B coeficient here ; - napryagenie
constexpr float K1 = 3.3F/4096.0F; // see datcheet ((page 226)) and calculate K coeficient here ;  - napryagenie

constexpr float B2 = (25.0F-0.76F/0.0025F); // see datacheet (page 226) and calculate B coeficient here ; -tempa
constexpr float K2 = ((3.3F/4096.0F)/0.0025F); // see datcheet ((page 226)) and calculate K coeficient here ; -tempa

int main()
{
  std::uint32_t data1 = 0U ;
  std::uint32_t data2 = 0U ;
  float temperature = 0.0F ;
  float voltage = 0.0F ;
  
  for(;;)    
  {
    //**************ADC*****************
    
    //Start conversion
    ADC1::CR2::SWSTART::On::Set();
    
     // wait until Conversion is not complete 
     while (ADC1::SR::EOC::ConversionNotComplete::IsSet( )) 
     { 
     };
    
    //Get data from ADC
    data1 = ADC1::DR::Get();//Get data from ADC;
    temperature = data1* K2+B2; // tempa
    
    // wait until Conversion is not complete 
     while (ADC1::SR::EOC::ConversionNotComplete::IsSet( )) 
     { 
     };
    data2 = ADC1::DR::Get();//Get data from ADC; 
    voltage = data2 * K1 + B1 ; //Convert ADC counts to temperature
    
    std::cout << "Count: " << data1 << " Temperature: " << temperature << std::endl ;
    std::cout << "Count: " << data2 << " voltage: " << voltage << std::endl ;
    if(voltage > 0.7)
    {
      GPIOÑ::ODR::ODR5::High::Set();
        if(voltage > 1.3)
        {
        GPIOC::ODR::ODR8::High::Set();
          if(voltage > 2)
          {
          GPIOC::ODR::ODR9::High::Set(); 
            if(voltage > 2.7)
            {
            GPIOA::ODR::ODR5::High::Set(); 
            } 
            else
            GPIOA::ODR::ODR5::Low::Set();
            }
          else
          GPIOC::ODR::ODR9::Low::Set();
          }
        else
        GPIOC::ODR::ODR8::Low::Set();
        }
      else
      GPIOC::ODR::ODR5::Low::Set();
    }
}
  
