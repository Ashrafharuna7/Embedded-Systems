    // Connect a POT betweek 0 and 3.3V with the wiper connected to PA0
    // NOTE!!: PA0 actually connects to IN5 of ADC1.
    // The POT voltage controls the duty cycle of a PWM output on PA3
    // Timer 2 Channel 4     is used to generate the PWM output.
    #include <stm32l432xx.h>
    #include "eeng1030_lib.h"
    #include <stdio.h>
    #include <stdint.h>
    #include <sys/unistd.h>
    #include <errno.h>
    #define enable_interrupts() asm(" cpsie i ")
    #define diable_interrupt() asm (" cpsid i")

    void setup(void);
    void delay(volatile uint32_t dly);
    void initADC(void);
    int readADC(int chan);
    void initTimer2(void);
    void setTimer2Duty(int duty);
    int captureFanSpeed();
    void initSerial(uint32_t baudrate);
    void delay(volatile uint32_t dly);

    int vin;
    int currentSpeed = 0;
    volatile uint32_t pulseTime = 0;
    volatile uint32_t rpm = 0;
    int count;
    int duration = 0;
    int oldSpeed = 0;
    int period = 0;

    volatile int data_ready = 0;

    int main()
    {
        setup();
    
    while (1)
    {
        vin = readADC(5);
        //printf("ADC Value: %d\r\n", vin);

        GPIOB->ODR ^= (1 << 3); // Toggle LED 
        // printf("USART2 is working!\r\n");
        //delay(500000); // Slow down output

        TIM2->CCR4 = (vin/4096.0)*999;
       // printf("Value:%d\r\n",TIM2->CCR4);
        rpm = captureFanSpeed();
         int duration = 40000;
         int pulses_per_rev = 2;
         int rpm= (60*8000000) / (duration * pulses_per_rev);
        printf("RPM: %d\n",rpm);
        printf("RPM:%d\r\n",rpm);
        delay(100000);

        // captureFanSpeed();  // Capture the fan speed and calculate RPM
        // printf("Fan RPM: %d\r\n", rpm);  // Print the RPM value
    }
    }
    void setup(void)
    {
        RCC->AHB2ENR |= (1 << 0) | (1 << 1); // turn on GPIOA and GPIOB

        pinMode(GPIOB,3,1); // digital output
        pinMode(GPIOB,4,0); // digital input
        enablePullUp(GPIOB,4); // pull-up for button
        pinMode(GPIOA,0,3);  // analog input
        pinMode(GPIOA,1,2);  // alternative funtion for PA1
        pinMode(GPIOA,2,2); //alternative function for PA2
        pinMode(GPIOA,3,2);  // alternative function mode for PA3
        pinMode(GPIOA,15,2); // alternate function mode for PA15
       
        selectAlternateFunction(GPIOA,15,3); // AF3 = USART2 TX
        selectAlternateFunction(GPIOA,2,7); // AF7 = USART2 RX
        selectAlternateFunction(GPIOA,1,1); // AF1 = Timer2 PA1 
        selectAlternateFunction(GPIOA,3,1); // AF1 = PWM PA3 

        initADC();
        initTimer2();
        initSerial(9600);
    }

    void initSerial(uint32_t baudrate)
    {
        RCC->APB1ENR1 |= (1 << 0); // make sure GPIOA is turned on
        const uint32_t CLOCK_SPEED= SystemCoreClock;    
        uint32_t BaudRateDivisor;
        BaudRateDivisor = CLOCK_SPEED/baudrate;	
        RCC->APB1ENR1 |= (1 << 17); // turn on USART2 (14 for USART )
        
        USART2->CR1 = 0;
        USART2->CR2 = 0;
        USART2->CR3 = (1 << 12); // disable over-run errors
        USART2->BRR = BaudRateDivisor;
        USART2->CR1 =  (1 << 3) | (1 << 2);  // enable the transmitter and receiver
        USART2->CR1 |= (1 << 5); // enable receiver interrupts
        USART2->CR1 |= (1 << 0); // enable the UART
        USART2->ICR = (1 << 1); // clear any old framing errors
        
        
    }

    void initADC()
    {
        // initialize the ADC
        RCC->AHB2ENR |= (1 << 13); // enable the ADC
        RCC->CCIPR |= (1 << 29) | (1 << 28); // select system clock for ADC
        ADC1_COMMON->CCR = ((0b01) << 16) + (1 << 22) ; // set ADC clock = HCLK and turn on the voltage reference
        // start ADC calibration    
        ADC1->CR=(1 << 28); // turn on the ADC voltage regulator and disable the ADC
        delay(100); // wait for voltage regulator to stabilize (20 microseconds according to the datasheet).  This gives about 180microseconds
        ADC1->CR |= (1<< 31);
        while(ADC1->CR & (1 << 31)); // wait for calibration to finish.
        ADC1->CFGR = (1 << 31); // disable injection
        ADC1_COMMON->CCR |= (0x0f << 18);
    }
    int readADC(int chan)
    {

        ADC1->SQR1 |= (chan << 6);
        ADC1->ISR = (1 << 3); // clear EOS flag
        ADC1->CR |= (1 << 0); // enable the ADC
        while ( (ADC1->ISR & (1 <<0))==0); // wait for ADC to be ready
        ADC1->CR |= (1 << 2); // start conversion
        while ( (ADC1->ISR & (1 <<3))==0); // wait for conversion to finish
        return ADC1->DR; // return the result
        ADC1->CR = 0;
    }

    void delay(volatile uint32_t dly)
    {
        while(dly--);
    }

    
    
    void initTimer2(void)
    {
        // Initialize Timer 2 for both PWM and input capture
        RCC->APB1ENR1 |= (1 << 0);  // Enable Timer 2 clock
        TIM2->CR1 = 0;  // Disable Timer 2
        
        // Configure Timer 2 for PWM output (channel 4 - PA3)
        // Configure TIM2 Ch2 as an input
        TIM2->CCMR2 = (0b110 << 12) + (1 << 11) + (1 << 10);  // Output compare mode (PWM)
        TIM2->CCER |= (1 << 12);  // Enable Channel 4 for PWM
        
        // Configure Timer 2 for input capture (channel 3 - PA1 for FG signal)
        TIM2->CCMR1 = (1 << 0) | (1 << 8);  // Capture on both rising and falling edges
        TIM2->CCER |= (1 << 4);  // Enable input capture on channel 2 (PA1)
        
        TIM2->ARR = 1000 - 1;  // Set period for PWM (adjust for desired frequency)
        TIM2->CCR4 = 500;  // Set initial PWM duty cycle (50%)
        

        //
        // Configure PA1 as alternate function (AF1 for TIM2_CH2)
    GPIOA->MODER &= ~(GPIO_MODER_MODE1);        // Clear mode bits
    GPIOA->MODER |= (GPIO_MODER_MODE1_1);       // Set alternate function mode
    GPIOA->AFR[0] |= (1 << GPIO_AFRL_AFSEL1_Pos); // AF1 for TIM2
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD1);       // No pull-up/down

    // Enable TIM2 clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    // Configure TIM2 CH2 as input capture on TI2
    TIM2->CCMR1 &= ~TIM_CCMR1_CC2S;    // Clear CC2S
    TIM2->CCMR1 |= TIM_CCMR1_CC2S_0;   // CC2S = 01: IC2 mapped to TI2

    // Set input capture prescaler (optional)
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2PSC;  // No prescaler, capture every event

    // Set input filter (optional)
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2F;    // No filter

    // Configure active edge detection
    TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP); // Rising edge

    // Enable capture
    TIM2->CCER |= TIM_CCER_CC2E; // Enable capture on channel 2

    // Enable Timer counter
    TIM2->CR1 |= TIM_CR1_CEN;
        //



        TIM2->EGR |= (1 << 0);  // Update event
        TIM2->CR1 |= (1 << 7);  // Enable auto-reload
        TIM2->CR1 |= (1 << 0);  // Enable counter
    }

    void setTimer2Duty(int duty)
    {
        int arrvalue=(duty*TIM2->ARR)/4095;
        TIM2->CCR4=arrvalue;
    }

    int captureFanSpeed()
    {
        // Calculate fan speed based on captured pulse time from FG signal (PA1)
        if (TIM2->SR & (1 << 2))  // Check if capture event occurred (CC1IF flag)
        {
            currentSpeed = TIM2->CCR2;
            duration = currentSpeed - oldSpeed;

            if (duration < 0) {
                duration += TIM2->ARR; // To get rid of them 0s
            }
            oldSpeed = currentSpeed;
            rpm = (6000000*duration/SystemCoreClock);
           //printf("Millesecond rotations: %f\r\n",rpm);

            return rpm;

            
        }
    }
    void USART2_IRQHandler()
    {
    
    }

    int _write(int file, char *data, int len)
    {
        if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
        {
            errno = EBADF;
            return -1;
        }
        while(len--)
        {
            eputc(*data);  
            data++;
        }    
        return 0;
    }

    void eputc(char c)
    {
        while((USART2->ISR & (1 << 6))==0);
        USART2->TDR=c;
    }

    

