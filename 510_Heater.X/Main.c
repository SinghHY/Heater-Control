/**********************************************************************
* Temperature Control 
* © PerkinElmer Health Sciences Canada, Inc., 2017
* This program is for Heater control U26 on old Board. 
* FileName:        Main.c
* Processor:       dsPIC33FJ256GP510A
* Compiler:        CCS v5.074 or Higher
* Version:     
************************************************************************/

#include <33FJ256GP510.h>
#fuses XT,NOWDT,NOPROTECT
#device ADC = 12 
#use delay(crystal = 40Mhz)
#use spi(SLAVE, SPI2, BITS = 8, MODE = 1, ENABLE = PIN_G9, stream = SPI_2)


#define Heater1 PIN_F2
#define Heater2 PIN_F3
#define Heater3 PIN_F4
#define Heater4 PIN_F5

/******************************************************************************/
// Moving Average FIlter Parameters
// Alpha = 0.1611328125 = 660/4096 ; 660 is maximum temperature value; 5mV/ 1C change
#define Sample_Length  20
float Alpha = 0.1611328125;
float Cofficent = (1/(float)Sample_Length) * Alpha;
/******************************************************************************/
// PID Parameters//

/******************************************************************************/

int8 SPI_Flag = 0, Byte_Count = 0, Rx, Tx, Cmand, ProbeID = 2,count = 0;
int8 Version = 7;
int8 PWM1 = 0, PWM2 = 0, PWM3 = 0, PWM4 = 0;
int8 AmpSPI1 = 15, AmpSPI2 = 25, AmpSPI3 = 35, AmpSPI4 = 45;
float Current1, Current2, Current3, Current4;  //
int Value, duty1, duty2, duty3, duty4;


/******************************************************************************/
// 8 bits SPI
#INT_SPI2

void spi2_slave_isr(void)
{
  Rx = spi_xfer_in(SPI_2, 8);
  Byte_Count++; 
 
  switch(Byte_Count)
            {
            case 1:
                spi_prewrite(Version);
                if(Cmand == 1)
                    spi_prewrite(0);
                else if(Cmand == 2)
                    spi_prewrite(0);
                break;
      
            case 2: 
                Cmand = Rx;
                if(Cmand == 1)
                    spi_prewrite(AmpSPI1);
                else if(Cmand == 2)
                        spi_prewrite(3);
                else if(Cmand == 3)
                        spi_prewrite(ProbeID);
                break;
            
            
            case 3: 
                 
                if(Cmand == 1)
                    spi_prewrite(0);
                else if(Cmand == 2)
                    {PWM1 = Rx;
                    spi_prewrite(0);}
                else if(Cmand == 3)
                   spi_prewrite(0);
                else if(Cmand == 7)
                    {PWM2 = Rx;
                    spi_prewrite(AmpSPI2);}
                break;      
            
            
            case 4: 
                Byte_Count = 0;            
                if(Cmand == 1)
                    spi_prewrite(0);
                else if(Cmand == 2)
                    {PWM3 = Rx;
                    spi_prewrite(AmpSPI3);}
                else if(Cmand == 3)
                    spi_prewrite(AmpSPI1);
                else if(Cmand == 7)
                    {PWM4 = Rx;
                    spi_prewrite(AmpSPI4);}
                break;
                
            default:
                Byte_Count = 0;
                break;
            }

}


float LP_Filter(int ch)
  {
    float Filter_Out;
    unsigned int16 i,sum, Sample[Sample_Length];

    set_adc_channel(ch);
    for(i = 0; i < Sample_Length; i++)
        {
            Sample[i] = read_adc();
            sum += Sample[i];
        }    
        Filter_Out = (float)sum * Cofficent;
        sum = 0;
    return(Filter_Out);    
  }


void main()
{    
   output_float(PIN_G9); // SS as an input
   setup_adc_ports(sAN0 | sAN2, VSS_VDD);
   setup_adc(ADC_CLOCK_INTERNAL);

   
    //Frequency 100Khz set up for PWM 1,2,3,4   
   setup_timer2(TMR_INTERNAL | TMR_DIV_BY_1, 39);
   setup_compare(2, COMPARE_PWM | COMPARE_TIMER2);
   set_pwm_duty(2,0);
   setup_compare(3, COMPARE_PWM | COMPARE_TIMER2);
   set_pwm_duty(3,0);

    
   enable_interrupts(INT_SPI2);
   enable_interrupts(INTR_GLOBAL);
   set_adc_channel(0);  
   while(1)
    {
       
       duty2 = 0 * 0.4;
       set_pwm_duty(2,duty2);
       
       duty3 = 0 * 0.4;
       set_pwm_duty(3,duty3);
       

       AmpSPI1  = read_adc();
       AmpSPI2  = (int8)LP_Filter(2);
    }
}   