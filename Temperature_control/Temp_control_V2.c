#include <msp430.h>
volatile int CONVER=0, CONVER1=0, CONVER2=0, CONVER3=0;
volatile int a = 0;
volatile int b = 0;
volatile int on = 0;

/**
 *
 */
void main(void)
{

    WDTCTL = WDTPW | WDTHOLD; // Detiene el watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Desactiva el modo de bajo consumo de energía
    __enable_interrupt();


    //CONFIGURACION DE BOTONES P1.1 Y P1.2
    P1DIR &= ~(BIT1 | BIT2);  //P1.1 y P1.2 ENTRADAS
    P1REN |= (BIT1 | BIT2);   //HABILITAR RESISTENCIAS INT.
    P1OUT |= (BIT1 | BIT2);   //PULL UP

    //CONFIGURACION LED ENCENDIDO  P9.1
    P9DIR |= BIT1;    //P2.0 SALIDA
    P9OUT &= ~BIT1; //LED APAGADO (LOW)

    //CONFIGURACION LED ENCENDIDO BOMBA P9.5
    P9DIR |= BIT5;    //P2.0 SALIDA
    P9OUT &= ~BIT5;  //LED APAGADO (LOW)

    //CONFIGURACION LED ENCENDIDO GATE  P9.6
    P9DIR |= BIT6;    //P2.0 SALIDA
    P9OUT &= ~BIT6;  //LED APAGADO (LOW)


    //SALIDAS PARA 100%
    //P3DIR = (BIT0|BIT1);
    //P3OUT &= ~(BIT0|BIT1);

    //CONFIGURACION DE BOTON EXTERNO (BOTON 3) P1.4

    P1DIR &= ~BIT4;     //  P1.4 ENTRADA
    P1REN |= BIT4;      // //HABILITAR RESISTENCIAS INT.
    P1OUT |= BIT4;      // PULL UP

     //TIMER
     P2DIR |= (BIT5|BIT4); //PRIMER FUNCION
     P2SEL0 |= (BIT5|BIT4);

     P1DIR |= (BIT6|BIT7); //TERCER FUNCION
     P1SEL0 |= (BIT6|BIT7);
     P1SEL1 |= (BIT6|BIT7);


     TA0CTL |= (BIT9|BIT4); //1MHZ Y CONTEO ARRIBA
     TA0CCTL1 |= (BIT7|BIT6|BIT5); //SET-RESET
     TA0CCTL2 |= (BIT7|BIT6|BIT5); //SET RESET



    // ADC
    while(REFCTL0&BITA); //Revisa que REFGENBUSY este en 0
    REFCTL0|=BIT0; //Activa la referencia interna


    ADC12CTL0|=(BITA|BIT4); //BITA: 64 CICLOS DE RELOJ, BIT4: ENCENDIDO
    ADC12CTL1|=BIT1; //SECUENCIA DE CANALES
    ADC12CTL2|=BIT5; //12 BITS (14 CICLOS DE RELOJ)
    ADC12CTL3=0;

    ADC12MCTL0|=(BIT8|BIT3); //P9.0
    ADC12MCTL1|=(BIT8|BIT3|BIT0); //P9.1
    ADC12MCTL2|=(BIT8|BIT3|BIT1); //P9.2
    ADC12MCTL3|=(BIT8|BIT3|BIT1|BIT0); //P9.3

    ADC12IER0|=(BIT0|BIT1|BIT2|BIT3); //INTERRUPCIONES


    while(!(REFCTL0 & BITC));
    //ADC12RDYIFG ESTA LISTA LA REFERENCIA LOCAL PARA USARSE
    //ADC12IFG0  INTERRUPCION CONVERSION LISTA

    while(1)
    {
        //BOTON 1 ENCENDIDO
            if((P1IN&BIT1) == 0)     //PRESION PARA ENCENDER
            {
                a = 1;
            }
            if((P1IN&BIT1) == BIT1 && (a == 1))  //EVITAR REBOTES
            {
                    a = 0;
                    on = 1;
                    P9OUT |= BIT1;    //LED ENCENDIDO


            }
            //BOTON 2
            if((P1IN&BIT2) == 0)
             {
                b = 1;
             }
             if((P1IN&BIT2) == BIT2 && (b == 1) && (on==1))
             {
                P9OUT |= BIT5; // ENCENDER LED BOMBBAS
                b = 0;
                TA0CCR0 = 5000;
                TA0CCR1 = 5000;
                TA0CCR2 = 5000;
                //P3OUT |= (BIT0|BIT1);

                __delay_cycles(5000000);
                //P3OUT &= ~(BIT6|BIT7);

             }

            __delay_cycles(300);
            ADC12CTL0 |= (BIT1|BIT0); //BIT1: HABILITAR CONVERSIÓN, BIT0: INICIAR CONVERSIÓN
            __delay_cycles(300);
            ADC12CTL0&=~BIT0;
    }

}


#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  switch (__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
  {
    case ADC12IV_NONE:        break;        // Vector  0:  No interrupt
    case ADC12IV_ADC12OVIFG:  break;        // Vector  2:  ADC12MEMx Overflow
    case ADC12IV_ADC12TOVIFG: break;        // Vector  4:  Conversion time overflow
    case ADC12IV_ADC12HIIFG:  break;        // Vector  6:  ADC12BHI
    case ADC12IV_ADC12LOIFG:  break;        // Vector  8:  ADC12BLO
    case ADC12IV_ADC12INIFG:  break;        // Vector 10:  ADC12BIN

    case ADC12IV_ADC12IFG0:    // Vector 12:  ADC12MEM0 Interrupt
    CONVER=(ADC12MEM0*0.48)+1200;

    if (CONVER >= 2000 && CONVER <= 3000){
               TA0CCR1 = 2500;
               TA0CCR2 = 2500;
               P9OUT |= BIT6;
           }
           if (CONVER < 2000 ){
               TA0CCR1 = CONVER;
               TA0CCR2 = 50;
               P9OUT &= ~BIT6;
           }
           if (CONVER > 3000 ){
               TA0CCR1 = 50;
               TA0CCR2 = CONVER;
               P9OUT &= ~BIT6;
           }
         ADC12IFGR1 &= ~BIT0;
         break;

    case ADC12IV_ADC12IFG1:   // Vector 14:  ADC12MEM1 Interrupt
    CONVER1=(ADC12MEM1*0.48)+500;
    TA0CCR2 = CONVER1;

    break;

    case ADC12IV_ADC12IFG2:   // Vector 16:  ADC12MEM2 Interrupt
    CONVER2=(ADC12MEM2*0.48)+500;
    TB0CCR3 = CONVER2;
    break;

    case ADC12IV_ADC12IFG3:  // Vector 18:  ADC12MEM3 Interrupt
    CONVER3=(ADC12MEM3*0.48)+500;
    TB0CCR4 = CONVER3;
    break;

    case ADC12IV_ADC12IFG4:   break;        // Vector 20:  ADC12MEM4
    case ADC12IV_ADC12IFG5:   break;        // Vector 22:  ADC12MEM5
    case ADC12IV_ADC12IFG6:   break;        // Vector 24:  ADC12MEM6
    case ADC12IV_ADC12IFG7:   break;        // Vector 26:  ADC12MEM7
    case ADC12IV_ADC12IFG8:   break;        // Vector 28:  ADC12MEM8
    case ADC12IV_ADC12IFG9:   break;        // Vector 30:  ADC12MEM9
    case ADC12IV_ADC12IFG10:  break;        // Vector 32:  ADC12MEM10
    case ADC12IV_ADC12IFG11:  break;        // Vector 34:  ADC12MEM11
    case ADC12IV_ADC12IFG12:  break;        // Vector 36:  ADC12MEM12
    case ADC12IV_ADC12IFG13:  break;        // Vector 38:  ADC12MEM13
    case ADC12IV_ADC12IFG14:  break;        // Vector 40:  ADC12MEM14
    case ADC12IV_ADC12IFG15:  break;        // Vector 42:  ADC12MEM15
    case ADC12IV_ADC12IFG16:  break;        // Vector 44:  ADC12MEM16
    case ADC12IV_ADC12IFG17:  break;        // Vector 46:  ADC12MEM17
    case ADC12IV_ADC12IFG18:  break;        // Vector 48:  ADC12MEM18
    case ADC12IV_ADC12IFG19:  break;        // Vector 50:  ADC12MEM19
    case ADC12IV_ADC12IFG20:  break;        // Vector 52:  ADC12MEM20
    case ADC12IV_ADC12IFG21:  break;        // Vector 54:  ADC12MEM21
    case ADC12IV_ADC12IFG22:  break;        // Vector 56:  ADC12MEM22
    case ADC12IV_ADC12IFG23:  break;        // Vector 58:  ADC12MEM23
    case ADC12IV_ADC12IFG24:  break;        // Vector 60:  ADC12MEM24
    case ADC12IV_ADC12IFG25:  break;        // Vector 62:  ADC12MEM25
    case ADC12IV_ADC12IFG26:  break;        // Vector 64:  ADC12MEM26
    case ADC12IV_ADC12IFG27:  break;        // Vector 66:  ADC12MEM27
    case ADC12IV_ADC12IFG28:  break;        // Vector 68:  ADC12MEM28
    case ADC12IV_ADC12IFG29:  break;        // Vector 70:  ADC12MEM29
    case ADC12IV_ADC12IFG30:  break;        // Vector 72:  ADC12MEM30
    case ADC12IV_ADC12IFG31:  break;        // Vector 74:  ADC12MEM31
    case ADC12IV_ADC12RDYIFG: break;        // Vector 76:  ADC12RDY
    default: break;
  }
}
