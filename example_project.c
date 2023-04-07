#include <xmc_common.h>
#include <xmc_gpio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define nullptr ((void*)0)

#define TICKS_PER_SECOND 1000
#define DOT_TIME_DELAY 100
#define STRING_TRANSMIT_DELAY 4900

#define LED1 P1_1

#define BUTTON1 P1_14
#define BUTTON2 P1_15

uint32_t ticks = 0;
uint32_t ticksBootButton1Press = 0;

uint32_t timeBootButton1Presses;

uint32_t countButtonOnePress = 0;

char intToCharPtr[100];

void SysTick_Handler(void)
{
  ticks++;
  ticksBootButton1Press++;
}

void delayGenerator(uint32_t timeDelay)
{
  while(1)
  {
    if(ticks > timeDelay)
    {
      ticks = 0;
      break;
    }
  }
}

void dotTransmit(void)
{
  XMC_GPIO_SetOutputHigh(LED1);
  delayGenerator(DOT_TIME_DELAY);
  XMC_GPIO_SetOutputLow(LED1);
  delayGenerator(DOT_TIME_DELAY);
}

void dashTransmit(void)
{
  XMC_GPIO_SetOutputHigh(LED1);
  delayGenerator(3*DOT_TIME_DELAY);
  XMC_GPIO_SetOutputLow(LED1);
  delayGenerator(DOT_TIME_DELAY);
}

void transmitMorseCharacter(char* letterOrNum)
{
  int i=0;
  while(letterOrNum[i] != nullptr)
  {
    if(letterOrNum[i] ==  '.')
    {
      dotTransmit();
    }
    else if(letterOrNum[i] == '-')
    {
      dashTransmit();
    }
    i++;
  }
}

void transmitMorseString(char* morseMessage, char** letters, char** numbers)
{
  for(int i=0; i<strlen(morseMessage); i++)
  { 
    // check for letters
    if(morseMessage[i] >= 'A' && morseMessage[i] <= 'Z')
    {
      transmitMorseCharacter(letters[morseMessage[i] - 'A']);
      delayGenerator(2*DOT_TIME_DELAY); // Letters distance (3 dots delay) = 1 dot from LED_OFF cycle + 2 dots delay here
    }
    // check for numbers
    else if(morseMessage[i] >= '0' && morseMessage[i] <= '9')
    {
      transmitMorseCharacter(numbers[morseMessage[i] - '0']);
      delayGenerator(2*DOT_TIME_DELAY);
    }
    else if(morseMessage[i] == ' ')
    {
      delayGenerator(4*DOT_TIME_DELAY); // 100ms from LED_OFF delay + 200ms delay from after a letter transmission + 400 ms = 700ms
    }
  }
}

int main(void)
{
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);

    XMC_GPIO_CONFIG_t out_config = \
    {
      .mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL, \
      .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW, \
      .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE
    };

    XMC_GPIO_CONFIG_t in_config = \
    {
      .mode=XMC_GPIO_MODE_INPUT_TRISTATE,\
      .output_level=XMC_GPIO_OUTPUT_LEVEL_LOW,\
      .output_strength=XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE
    };

    // LED configuration using out_config
    XMC_GPIO_Init(LED1, &out_config);

    // Buttons 1 and Button 2 configuration using in_config
    XMC_GPIO_Init(BUTTON1, &in_config);
    XMC_GPIO_Init(BUTTON2, &in_config);

    char morseMessage[] = "I CAN MORSE";

    char* letters[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."
    };

    char* numbers[] = {
    "-----", ".----", "..---", "...--", "....-", ".....",
    "-....", "--...", "---..", "----."
    };

    //transmitMorseString(morseMessage,letters,numbers);

  while(1)
  {
    if(XMC_GPIO_GetInput(BUTTON1) == 0) //Button is pressed and held, 0 is press and 1 is release
    {
      timeBootButton1Presses = ticksBootButton1Press;
      ticksBootButton1Press = 0;

      if(countButtonOnePress < 3)
      {
        countButtonOnePress++;
      }
      delayGenerator(1); // Workaround for missing a LED blinking
      transmitMorseString(morseMessage,letters,numbers); // Sending "I CAN MORSE" on Button 1 press
    }
    else // Need to abort the ongoing transmission when released ? - Sequential logic not sure possible
    {

    }

    if(XMC_GPIO_GetInput(BUTTON2) == 0)
    {
      if(countButtonOnePress == 0)
      {
        delayGenerator(1);
        transmitMorseString("0",letters,numbers); // Button 1 is not pressed yet and Button 2 is pressed, then sending "0"
      }
      else
      {
        // Button 1 is pressed only once and then button2 is pressed, sending time between boot and button 1 press
        delayGenerator(1);
        sprintf(intToCharPtr, "%ld", timeBootButton1Presses);
        transmitMorseString(intToCharPtr,letters,numbers);
      }
    }
  }

  return 0;
}