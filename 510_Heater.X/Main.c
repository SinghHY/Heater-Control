/**********************************************************************
* Temperature Control 
* � PerkinElmer Health Sciences Canada, Inc., 2017
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
// ADC Parameters
// Alpha = 0.1611328125 = 660/4096 ; 660 is maximum temperature value; 5mV/ 1C change
float Alpha = 0.1611328125;
/******************************************************************************/
// PID Parameters//
float C_out = 0, Set_Point, M_Variable = 95, Error = 0,  Previous_Error;
float dt = 0.01,  Kp = 0.5, Ki = 0.01, Kd = 0.1, Integral = 0, Derivative = 0;
/******************************************************************************/

int8 SPI_Flag = 0, Byte_Count = 0, Rx, Tx, Cmand, ProbeID = 2,count = 0;
int Version = 7,SP = 200, Temp, LTMR, MV;
unsigned int Value, Duty, Err_cnt = 0;

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



#INT_TIMER1 Level = 7
void  timer1_isr(void) 
{
    M_Variable= ((float)read_adc() * Alpha) + 12;
    
    Error = Set_Point - M_Variable;
        if(Error < 0)
           Error = 0;
    Integral = Integral + Error*dt;
    Derivative = (Error - Previous_Error)/dt;
    Previous_Error = Error;
}


void main()
{    
   output_float(PIN_G9); // SS as an input
   setup_adc_ports(sAN0, VSS_VDD);
   setup_adc(ADC_CLOCK_INTERNAL);
   set_adc_channel(0);
   // Timer 1 for 10 ms INT when clock is 100MHz
   setup_timer1(TMR_INTERNAL | TMR_DIV_BY_64, 7812); 

    //Frequency 258 Hz set up for PWM 2,3   
   setup_timer2(TMR_INTERNAL | TMR_DIV_BY_256, 500);
   setup_compare(2, COMPARE_PWM | COMPARE_TIMER2);
   set_pwm_duty(2,0);

   enable_interrupts(INT_TIMER1); 
   enable_interrupts(INTR_GLOBAL);
  
   while(1)
    {

      MV  = (int8)M_Variable;
      Set_Point = (float)SP;
              
      C_out = (Kp * Error) + (Ki * Integral);
      if(C_out > 500)
          C_out = 500;
      else if(C_out < 0)
          C_out = 0;
      else
      { 
        Duty = (unsigned int)C_out;
        set_pwm_duty(2,Duty);
      }
    }
}   
