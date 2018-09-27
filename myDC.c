// Proj : PWM Controlled DC Motor
// Doc  : myDC.c
// Name : Kenosha Vaz
// Date : 10 April, 2018

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"

static TIM_HandleTypeDef tim1;

/* Initialise the GPIO pins */

ParserReturnVal_t CmdDC_Init(int action)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  uint16_t rc,value;

  rc = fetch_uint16_arg(&value);
  if(rc)
    {
      printf("\nMust supply a Period Value!\n\n");
      return CmdReturnBadParameter1;
    }  
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = 0;
  
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Initialise the Timer Delay*/

  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Alternate = 6;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  

  __HAL_RCC_TIM1_CLK_ENABLE();

  /* Initialize PWM */

  tim1.Instance = TIM1;
  tim1.Init.Prescaler     = HAL_RCC_GetPCLK2Freq() / 1000000;
  tim1.Init.CounterMode   = TIM_COUNTERMODE_UP;
  tim1.Init.Period        = value;
  tim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  tim1.Init.RepetitionCounter = 0;

  HAL_TIM_Base_Init(&tim1);

  HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

  HAL_TIM_Base_Start(&tim1);


  TIM1->DIER |= 0x01;

  /* Configure Output */

  TIM_OC_InitTypeDef sConfig;

  sConfig.OCMode       = TIM_OCMODE_PWM1;
  sConfig.Pulse        = 0;
  sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfig.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  /* For Channel 1 */

  HAL_TIM_PWM_ConfigChannel(&tim1,&sConfig, TIM_CHANNEL_1);

  HAL_TIM_PWM_Start(&tim1,TIM_CHANNEL_1); /* Start PWM Output */

  /* For Channel 2 */

  HAL_TIM_PWM_ConfigChannel(&tim1,&sConfig, TIM_CHANNEL_2);

  HAL_TIM_PWM_Start(&tim1,TIM_CHANNEL_2); /* Start PWM Output */

  return CmdReturnOk;
}
ADD_CMD("dcinit",CmdDC_Init,"                Initialise DC Motor")

void TIM1_UP_TIM16_IRQHandler(void){

  TIM1->SR &= ~0x01;
  
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
}

ParserReturnVal_t CmdDC(int action)
{
  if(action!=CMD_INTERACTIVE) return CmdReturnOk;

  uint16_t rc, dir, spd;

  rc = fetch_uint16_arg(&dir);
  if(rc)
    {
      printf("\nMust supply a Direction! (0/1/2)\n\n");
      return CmdReturnBadParameter1;
    }

  if(dir==0){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_0, GPIO_PIN_RESET);
    spd=0;
  }else if(dir==1){
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

	rc = fetch_uint16_arg(&spd);
	if(rc)
	  {
	    printf("\nMust supply a Speed!\n\n");
	    return CmdReturnBadParameter1;
	  }

	TIM1->CCR1=spd;

	
  }else if(dir==2){
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

	rc = fetch_uint16_arg(&spd);
	if(rc)
	  {
	    printf("\nMust supply a Speed!\n\n");
	    return CmdReturnBadParameter1;
	  }

	TIM1->CCR2=spd;
	
  }else{
    printf("\nPlease Enter 0|1|2 to Set direction\n\n\t0 :\tBrake\n1 :\tForward\n2 :\tReverse\n");
    return CmdReturnBadParameter1;
  }

  return CmdReturnOk;
}
ADD_CMD("dc",CmdDC,"<0|1|2> <SPEED> Set Direction of DC Motor")
