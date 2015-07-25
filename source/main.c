#include "stm32f0xx.h"                  // Device header

/**
 * Set System Clock Speed to 48Mhz
 */
void setToMaxSpeed(void);

/**
 * Initialized the timer 2 as a down counter triggering from the ETR pin
 * Also enables the update interrupt for this timer
 * @param upToValue number of counts to issue and iterrupt
 */
void timer_2_capture_init();

/**
 * Initialize Pin B1 as output
 */
void led_init(void);

int main(void){
	setToMaxSpeed();
	led_init();
	timer_2_capture_init();
	while(1){
		//for(int i=0; i<0xFFFFF;i++);
		//GPIO_WriteBit(GPIOA,GPIO_Pin_1,!GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1));//toggles led
	}
}

void setToMaxSpeed(void){
	int internalClockCounter;
	RCC_PLLCmd(DISABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY));
	RCC_HSEConfig(RCC_HSE_OFF);
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2,RCC_PLLMul_12);
	RCC_PREDIV1Config(RCC_PREDIV1_Div1);
	RCC_PLLCmd(ENABLE);
	while(!RCC_GetFlagStatus(RCC_FLAG_PLLRDY));
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	for(internalClockCounter=0;internalClockCounter<1024;internalClockCounter++){
		if(RCC_GetSYSCLKSource()==RCC_SYSCLKSource_PLLCLK){
			SystemCoreClockUpdate();
			break;
		}
	}
}

void timer_2_capture_init(){
	//Configure Pin A2 as TIM2_CH3
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitTypeDef myGPIO;
	GPIO_StructInit(&myGPIO);
	myGPIO.GPIO_Pin=GPIO_Pin_2;
	myGPIO.GPIO_Mode=GPIO_Mode_AF;
	myGPIO.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&myGPIO);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_2);
	
	//time base configuration
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_TimeBaseInitTypeDef myTimer;
	TIM_TimeBaseStructInit(&myTimer);
	myTimer.TIM_CounterMode=TIM_CounterMode_Down;
	myTimer.TIM_Prescaler=(48);//microsecond count
	myTimer.TIM_ClockDivision=TIM_CKD_DIV1;
	myTimer.TIM_Period=(10000);//10 ms interrupt
	TIM_TimeBaseInit(TIM2,&myTimer);
	TIM_InternalClockConfig(TIM2);
	
	//capture channel 3  configuration
	TIM_ICInitTypeDef myCapture;
	TIM_ICStructInit(&myCapture);
	myCapture.TIM_Channel=TIM_Channel_3;
	myCapture.TIM_ICFilter=0xF;//no filter
	myCapture.TIM_ICPolarity=TIM_ICPolarity_Rising;
	myCapture.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	myCapture.TIM_ICSelection=TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM2,&myCapture);
	
	
	
	//Interrupt configuration
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);
	NVIC_EnableIRQ(TIM2_IRQn);
	
	//start timer
	TIM_Cmd(TIM2,ENABLE);
	//start capture
	TIM_CCxCmd(TIM2,TIM_Channel_3,ENABLE);
}

void led_init(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitTypeDef myGPIO;
	GPIO_StructInit(&myGPIO);
	myGPIO.GPIO_Pin=GPIO_Pin_1;
	myGPIO.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_Init(GPIOB,&myGPIO);
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);
	GPIO_Init(GPIOA,&myGPIO);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

int update_counter;
int capture_counter;
int capture_val;

void TIM2_IRQHandler(void){
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		//GPIO_WriteBit(GPIOB,GPIO_Pin_1,!GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_1));//toggles led
		update_counter++;
	}
	if(TIM_GetITStatus(TIM2,TIM_IT_CC3)){
		TIM_ClearITPendingBit(TIM2,TIM_IT_CC3);
		capture_val=TIM_GetCapture3(TIM2);
		capture_counter++;
	}
}

