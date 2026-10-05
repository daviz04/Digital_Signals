#include <msp430.h>

void init_timer(void) {
    // Retaining your requested 20 Hz sample rate
    TA0CCR0 = 50000;                  // 1MHz / 50000 = 20 Hz sample rate
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL_2 + MC_1 + TACLR;
}
void init_adc(void) {
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;
    // Example ADC10 initialization - make sure ADC10IE is included!
    ADC10CTL1 = INCH_4 + SHS_0 + ADC10SSEL_3; // A4 Input (P1.4)

    ADC10AE0 |= BIT4;
}

volatile unsigned int adc_raw = 0;
volatile unsigned char pcm_4bit = 0;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Configure Clock to 1 MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // Configure Outputs: P1.0 (LSB), P1.1, P1.2, P1.3 (MSB)
    P1DIR |= (BIT0 | BIT1 | BIT2 | BIT3);
    // Disable internal pull-up/pull-down resistors on Pins 0, 1, 2, 3
    P1REN &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P1OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    init_adc();
    init_timer();

    __enable_interrupt();       // Enable global interrupts

    while(1) {
        __bis_SR_register(LPM0_bits); // Sleep in low power mode
    }
}

// 1. TIMER ISR (Only triggers or does nothing if hardware-linked)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {
    // If you don't use hardware-triggering, you only start the conversion here
    // and IMMEDIATELY exit. No while loop!
    ADC10CTL0 |= ENC + ADC10SC;
}

// 2. ADC ISR (Executes automatically ONLY when data is ready)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    // Read the result immediately without waiting
    adc_raw = ADC10MEM;

    // Scale and write to your 4-bit output port
    pcm_4bit = (adc_raw >> 6) & 0x0F;
    P1OUT = (P1OUT & 0xF0) | pcm_4bit;
}

