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
#use delay(clock = 100MHz, crystal=40MHz)
#use spi(SLAVE, SPI2, BITS = 8, MODE = 1, ENABLE = PIN_G9, stream = SPI_2)


/******************************************************************************/
// Moving Average FIlter Parameters
// Alpha = 0.1611328125 = 660/4096 ; 660 is maximum temperature value; 5mV/ 1C change
#define Sample_Length  5
float Alpha = 0.1611328125;
float Cofficent = (1/(float)Sample_Length) * Alpha;
/******************************************************************************/
// PID Parameters//

/******************************************************************************/

int8 SPI_Flag = 0, Byte_Count = 0, Rx, Tx, Cmand, ProbeID = 2,count = 0;
int8 Version = 7,SP = 0, Temp, LTMR, MV;
int Value, Duty;
float  Kp = 0.5, C_out = 0, Set_Point, M_Variable, Error;

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
                    spi_prewrite(MV);
                else if(Cmand == 3)
                        spi_prewrite(ProbeID);
                break;
            
            case 3: 
                 
                if(Cmand == 1)
                    spi_prewrite(0);
                else if(Cmand == 3)
                   spi_prewrite(0);
                break;      
            
            
            case 4: 
                Byte_Count = 0;            
                if(Cmand == 1)
                    SP = Rx;
                else if(Cmand == 3)
                    spi_prewrite(0);
                break;
                
            default:
                Byte_Count = 0;
                break;
            }

}



LP_Filter(int ch)
  {
    float Filter_Out;
    unsigned int16 i,sum, Sample[Sample_Length];

    set_adc_channel(ch);
    for(i = 0; i < Sample_Length; i++)
        {
            Sample[i] = read_adc();
            sum += Sample[i];
        }    
        //Filter_Out = ((float)sum * Cofficent) + 12;
        M_Variable = ((float)sum * Cofficent) + 12;
        MV  = (int8)M_Variable;
        sum = 0;
    return(Filter_Out);    
  }

/*#INT_TIMER1
void  timer1_isr(void) 
{
    LTMR = 1;

}*/


void main()
{    
   output_float(PIN_G9); // SS as an input
   setup_adc_ports(sAN0, VSS_VDD);
   setup_adc(ADC_CLOCK_INTERNAL);
   // Timer 1 for 100ms INT
   //setup_timer1(TMR_INTERNAL | TMR_DIV_BY_256, 7812); 

    //Frequency 258 Hz set up for PWM 2,3   
   setup_timer2(TMR_INTERNAL | TMR_DIV_BY_256, 100);
   setup_compare(2, COMPARE_PWM | COMPARE_TIMER2);
   set_pwm_duty(2,0);

   //enable_interrupts(INT_TIMER1); 
   enable_interrupts(INT_SPI2);
   enable_interrupts(INTR_GLOBAL);
   while(1)
    {

       LP_Filter(0);
       Set_Point = (float)SP;
       Error = Set_Point - M_Variable;
            if(Error < 0)
                Error = 0;
        C_out = Kp * Error;
        Duty = (int)C_out;
        set_pwm_duty(2,Duty);
    }
}   