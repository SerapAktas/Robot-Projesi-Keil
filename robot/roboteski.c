#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "string.h"
#include "driverlib/adc.h"
#include "uartstdio.h"
#include "lm4f120h5qr.h"
#define sol_kat 0.38
#define sag_kat 0.25
#define sol_adim 0.0061
#define sag_adim 0.0075
#define sensor1									GPIO_PIN_4
#define sensor2									GPIO_PIN_5
#define sensor3									GPIO_PIN_4
#define sensor4									GPIO_PIN_6
#define sensor5									GPIO_PIN_7
#define sensor6									GPIO_PIN_1
#define sensor7									GPIO_PIN_2
#define sensor8									GPIO_PIN_3
#define GPIO_PA0_U0RX           0x00000001
#define GPIO_PA1_U0TX           0x00000401
#define trig										GPIO_PIN_5
#define echo										GPIO_PIN_2
#define GPIO_PF2_T1CCP0         0x00050807
#define GPIO_PF3_T1CCP1         0x00050C07
#define GPIO_PF1_T0CCP1         0x00050407
#define GPIO_PB6_T0CCP0         0x00011807
#define GPIO_PB7_T0CCP1         0x00011C07
#define GPIO_PB0_T2CCP0         0x00010007
#define GPIO_PB1_T2CCP1         0x00010407
#define GPIO_PD7_CCP1           0x00031C03
#define GPIO_PB2_CCP3           0x00010804
#define GPIO_O_LOCK             0x00000520  // GPIO Lock
#define GPIO_LOCK_KEY_DD        0x4C4F434B 
#define GPIO_O_CR               0x00000524

unsigned long ulPeriod, sag_motor, sol_motor,sagMotorHizi,solMotorHizi;
unsigned char a[10],i,i1=0,u,p,run,le=0;
unsigned int t,t1,seri,u1,Period,say,den,k[100],y=0,kal=0;
unsigned char ch[20];
signed int right,left;
unsigned long uzak,ulADC0_Value[5],pil,mot_oran1,mot_oran2;
unsigned long ulADC1_Value[5];
void __error__(char *pcFilename, unsigned long ulLine)
{
}
void delay (unsigned long int id)
{
		while(id--);
}
void UARTIntHandler(void)
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, ulStatus);
		i=UARTCharGetNonBlocking(UART0_BASE);
		if(i=='c')
		{	for(y=0;y<100;y++)
				{
					if(k[y]!=0)
						UARTprintf("k[%d]= %d\n" ,y , k[y]);
				}
				//pil=ulADC0_Value[0];
				//UARTprintf("pil %d   sag %d sol %d  uzak   %d\n", pil,ulADC0_Value[1],ulADC0_Value[2],uzak);
				//UARTprintf("pil %d   sag %d sol %d  uzak   %d\n", pil,ulADC1_Value[1],ulADC1_Value[2],uzak);
				
		}
		if(i=='h')
		{
			
		}
}

void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    while(ulCount--)
    {
        UARTCharPutNonBlocking(UART0_BASE, *pucBuffer++);
	  }
}

void  sonic()
{
		GPIOPinWrite(GPIO_PORTC_BASE,trig, trig);// ultrasonic sensor� baslat
		SysCtlDelay(250);//10 micro saniye bekle
		GPIOPinWrite(GPIO_PORTC_BASE,trig, 0);
		t=0;
		while(!GPIOPinRead(GPIO_PORTB_BASE,echo)&&t<50000)
		{
				t++;
		}

		t=0;
		while(GPIOPinRead(GPIO_PORTB_BASE,echo)&&t<50000)
		{
				t++;	
		}
		uzak/=13;			
		ADCProcessorTrigger(ADC0_BASE, 1);
		while(!ADCIntStatus(ADC0_BASE, 1, false))
		{
		}
		ADCIntClear(ADC0_BASE, 1);
		ADCSequenceDataGet(ADC0_BASE, 1, ulADC0_Value);
//		UARTprintf("pil %d   bos %d    %d\n", ulADC0_Value[0],ulADC0_Value[1],ulADC0_Value[2]);///////////////////////////////******************************/////////

		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0,GPIO_PIN_0 ); //ir ledi yakar
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3,GPIO_PIN_3 ); //ir ledi yakar
		SysCtlDelay(1500); //transist�r�n iletime ge�mesi i�in bekle

		ADCProcessorTrigger(ADC0_BASE, 1); //ADC pil ve iki sens�redeki gerilimleri okur
		while(!ADCIntStatus(ADC0_BASE, 1, false))//ADC' nin gerilimleri okumumasini bekliyor
		{
		}
	
		ADCIntClear(ADC0_BASE, 1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0,0 );//ir ledler son�k
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3,0 );//ir ledler son�k
		ADCSequenceDataGet(ADC0_BASE, 1, ulADC1_Value);//sequence_1 e yazilan sonuclari okur
		TimerMatchSet(TIMER0_BASE, TIMER_B, 64000-16*ulADC1_Value[1]);//kirmizi ledin parlakligi sag ir sens�r�n�n engeline bagli
		TimerMatchSet(TIMER1_BASE, TIMER_B, 64000-16*ulADC1_Value[2]);//yesil ledin parlakligi sol ir sens�r�n�n engeline bagli
		right=ulADC0_Value[1]-ulADC1_Value[1];
		left=ulADC0_Value[2]-ulADC1_Value[2];	
}

/*************************************************************************************************/
void dur()
{
		sol_motor = (ulPeriod-1)*sol_kat;
		sag_motor = (ulPeriod-1)*sag_kat;
		TimerMatchSet(TIMER2_BASE, TIMER_A,sag_motor); // Timer 0 Match set
		TimerMatchSet(TIMER2_BASE, TIMER_B,sol_motor); // Timer 0 Match set
		TimerEnable(TIMER2_BASE, TIMER_BOTH);
}
/*************************************************************************************************/
void ileri(long a,long b)
{
		sol_motor = (ulPeriod-1)*(sol_kat+(sol_adim)*b);
		sag_motor = (ulPeriod-1)*(sag_kat+(sag_adim)*a);
		TimerMatchSet(TIMER2_BASE, TIMER_A,sag_motor); // Timer 0 Match set
		TimerMatchSet(TIMER2_BASE, TIMER_B,sol_motor); // Timer 0 Match set
		TimerEnable(TIMER2_BASE, TIMER_BOTH); 
}
/*************************************************************************************************/
void sag()
{
		sol_motor = (ulPeriod-1)*(0.99);
		sag_motor = (ulPeriod-1)*(0.01);
		TimerMatchSet(TIMER2_BASE, TIMER_A,  sag_motor); // Timer 0 Match set
		TimerMatchSet(TIMER2_BASE, TIMER_B, sol_motor); // Timer 0 Match set
		TimerEnable(TIMER2_BASE, TIMER_BOTH); 
}
/*************************************************************************************************/
void sol()
{
		sol_motor = (ulPeriod-1)*(0.01);
		sag_motor = (ulPeriod-1)*(0.99);
		TimerMatchSet(TIMER2_BASE, TIMER_A,  sag_motor); // Timer 0 Match set
		TimerMatchSet(TIMER2_BASE, TIMER_B, sol_motor); // Timer 0 Match set
		TimerEnable(TIMER2_BASE, TIMER_BOTH); 
}
/*************************************************************************************************/
void bos()
{
		TimerDisable(TIMER2_BASE, TIMER_BOTH); 
}
/*************************************************************************************************/
void IntGPIOF(void)
{
	if (GPIOPinIntStatus(GPIO_PORTF_BASE, GPIO_PIN_0) & GPIO_PIN_0)
	{
			GPIOPinIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
	   	TimerDisable(TIMER2_BASE, TIMER_BOTH); 
			run=0;
 	}
	else
	{
	    GPIOPinIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
	   	TimerEnable(TIMER2_BASE, TIMER_BOTH); 
			run=1;
	}
}
/*************************************************************************************************/
void Timer4IntHandler(void)
{
		TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
		if(le==0)
		{
				TimerMatchSet(TIMER1_BASE, TIMER_A, 63998);
				le=1;
		}
		else
		{
				TimerMatchSet(TIMER1_BASE, TIMER_A, 0);
				le=0;
		}
}
/*************************************************************************************************/
void Timer3IntHandler(void)
{
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    sonic();
		UARTprintf("pil %d   dol %d    %d\n", ulADC0_Value[0],ulADC0_Value[1],ulADC0_Value[2]);//////*************************////////

}
/*************************************************************************************************/
void duz_ileri2(void)
{
	if( (!GPIOPinRead(GPIO_PORTE_BASE,sensor1)))
			{
				
				sagMotorHizi+=10;
			  solMotorHizi-=10;
				
			}
			 else  if((!GPIOPinRead(GPIO_PORTE_BASE,sensor2)))
			{
				
					sagMotorHizi+=5;
			    solMotorHizi-=5;
				
			}
			 else if( (!GPIOPinRead(GPIO_PORTC_BASE,sensor3)))
			{
				
			   sagMotorHizi+=3;
			   solMotorHizi-=3;
				
			}
			
			else if( (!GPIOPinRead(GPIO_PORTE_BASE,sensor6)) )
			{
		
			          sagMotorHizi-=3;
				        solMotorHizi+=3;
			   
				
			}
			else if( (!GPIOPinRead(GPIO_PORTE_BASE,sensor7)))
			{
				
							   sagMotorHizi-=5;
								solMotorHizi+=5;
			          
				
			}
			else if( (!GPIOPinRead(GPIO_PORTE_BASE,sensor8)))
			{
				
						     sagMotorHizi-=10;
									solMotorHizi+=10;
			       
				
			}
			else if( (!GPIOPinRead(GPIO_PORTC_BASE,sensor4))|| (!GPIOPinRead(GPIO_PORTC_BASE,sensor5)))
			{
			
			sagMotorHizi=30;
			solMotorHizi=70;

			}
	if (sagMotorHizi < 30 ) sagMotorHizi = 30; 
  if (solMotorHizi < 40 ) solMotorHizi = 40;
	if (sagMotorHizi >= 40) sagMotorHizi = 40; 
  if (solMotorHizi >= 70) solMotorHizi = 70; 
	ileri(sagMotorHizi,solMotorHizi);
}
int main(void)
{	
		SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	
		GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);
		GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);
	  GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, trig); //ultrasonic sensor tetikleme
	  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,echo);// ultrasonic sensor dinleme
		
	  HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;//sw2 serbest
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;	
		GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4);//sw1 basla sw2 dur
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); 
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_FALLING_EDGE); 
    GPIOPinIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); 
    /* Configure interrupt for buttons */
    IntEnable(INT_GPIOF);
    IntMasterEnable();
	  GPIOPadConfigSet(GPIO_PORTB_BASE, echo, GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);	
	  GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);	      
   
    GPIOPinConfigure(GPIO_PB0_T2CCP0); //motor pwm    
    GPIOPinConfigure(GPIO_PB1_T2CCP1); //motor pwm
	  GPIOPinConfigure(GPIO_PF1_T0CCP1); //kirmizi pwm
    GPIOPinConfigure(GPIO_PF2_T1CCP0); //mavi pwm
	  GPIOPinConfigure(GPIO_PF3_T1CCP1); //yesil pwm
	
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_0 ); 
	  GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_1 );
		GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_1 );
		GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_2 );
		GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_3 );
		
    ulPeriod = 64000;
		
 		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2); 
		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); 
		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); 
		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3); 
		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);

    TimerConfigure(TIMER2_BASE, (TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM)); 
		TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM)); 
    TimerConfigure(TIMER1_BASE, (TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM));
		TimerConfigure(TIMER3_BASE, TIMER_CFG_32_BIT_PER); // sensorleri belirli araliklarla oku
		TimerConfigure(TIMER4_BASE, TIMER_CFG_32_BIT_PER); // flash led

    TimerLoadSet(TIMER3_BASE, TIMER_A, SysCtlClockGet()/60); //saniyede 20 kez sensorleri oku
    TimerLoadSet(TIMER4_BASE, TIMER_A, SysCtlClockGet()/4); //saniyede 2 kez flash

    IntEnable(INT_TIMER3A);
	  IntEnable(INT_TIMER4A);

		TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
		TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);

		TimerControlLevel(TIMER2_BASE, TIMER_BOTH, 0); 
		TimerControlLevel(TIMER0_BASE, TIMER_BOTH, 0); 
    TimerControlLevel(TIMER1_BASE, TIMER_BOTH, 0);

    TimerLoadSet(TIMER2_BASE, TIMER_B,ulPeriod -1 ); 
    TimerLoadSet(TIMER2_BASE, TIMER_A,ulPeriod -1);
		TimerLoadSet(TIMER0_BASE, TIMER_B,ulPeriod -1);
    TimerLoadSet(TIMER1_BASE, TIMER_B,ulPeriod -1 );
		TimerLoadSet(TIMER1_BASE, TIMER_A,ulPeriod -1 ); 

		sag_motor = (unsigned long)(ulPeriod-1)*0.01;//0.29;
    sol_motor = (unsigned long)(ulPeriod-1)*0.01;//0.39;
		TimerMatchSet(TIMER2_BASE, TIMER_A,  sag_motor); 
		TimerMatchSet(TIMER2_BASE, TIMER_B, sol_motor); 
		TimerMatchSet(TIMER0_BASE, TIMER_B, sol_motor); 

		TimerPrescaleSet(TIMER2_BASE,TIMER_BOTH,19);
		TimerPrescaleMatchSet(TIMER2_BASE,TIMER_BOTH,18);
		TimerEnable(TIMER0_BASE, TIMER_BOTH); 
		TimerEnable(TIMER1_BASE, TIMER_BOTH); 

		// TimerEnable(TIMER2_BASE, TIMER_BOTH); 
    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
		GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
		UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));  
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    UARTStdioInit(0);

		SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
		SysCtlADCSpeedSet(SYSCTL_ADCSPEED_1MSPS);
		GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);
		GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);
		GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

		ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
		ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH11);
		ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH6);
		ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH7 | ADC_CTL_IE |ADC_CTL_END);

		ADCSequenceEnable(ADC0_BASE, 1);
		ADCIntClear(ADC0_BASE, 1);

		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0,0 ); //ir ledler son�k
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3,0 ); // ir ledler s�n�k
		TimerDisable(TIMER2_BASE, TIMER_BOTH); 

	  IntMasterEnable();
	  TimerEnable(TIMER3_BASE, TIMER_A); // sensor okuma
	
	
	
	
	GPIOPinTypeGPIOInput(GPIO_PORTC_BASE,sensor3|sensor4|sensor5);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,sensor1|sensor2|sensor6|sensor7|sensor8);
		
	GPIOPadConfigSet(GPIO_PORTC_BASE,sensor3|sensor4|sensor5,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTE_BASE,sensor1|sensor2|sensor6|sensor7|sensor8,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
	
		
	
		while(1)
		{
				if(run==1)
				{				
						bos();
				}
				else
				{
		
						duz_ileri2();
			
				}
		}	
}

				
			