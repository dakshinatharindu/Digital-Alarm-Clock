/*
 * Audio_Output_Avr.cpp
 *
 * Created: 15/05/2021 06:37:06
 * Author : Nuwan Udara
 */ 

#define F_CPU  1000000UL //problem with proteus simulation, so currently using 1MHz F_CPU //#define F_CPU  16000000UL
#include <avr/io.h>
#define __DELAY_BACKWARD_COMPATIBLE__    // This is requaired to make _delay_ms() take vairables
#include <util/delay.h>					// for delay functions
#include <avr/interrupt.h>			   // for clocks and interrupts 
#include <avr/pgmspace.h>			  //for PROGMEM 
#include "pitches.h"
#include "melodies.h"
 
 //Buzzer declaration
#define buzzer_DDR DDRB
#define buzzer_PORT PORTB
#define buzzer_PIN PORTB0

// interrupt button declaration
#define interruptDDR DDRD

 
 
//input pins
int playbutton=PORTB5;
int changenextsong=PORTB4;
int interruptPin = PORTD2;

// Square Wave generating methods
void feq_ctc(int step); // the CTC implementation *buggy
void Delay_note_method(float freq , float dur);// Using oscillator with CPU cycles delay. A raw method

void play(int number);

//currently testing multiple methods to generate signals



//Local variables
int songnumber=0;
volatile int sp = 0;

// Interrupt declaration ISR functions
/** The Interrupt function we use with CTC mode code

**/

ISR(TIMER1_COMPA_vect)          // 16 bit Timer interrupt handler
{
	PORTB ^= (1 << PORTB0); // generate square wave (toggle)
}

/** External interrupt for INT0 (PD2) pin ISR function
We will use this to stop currently playing melody **/

ISR (INT0_vect)          //External interrupt Stop button ISR
{
	sp=0;
				
	_delay_ms(100);
}


void changesong(){
	_delay_ms(500);
	if (songnumber==3){songnumber=0;}
	else{songnumber++;}
}


int tempochoose(int number){
	if (number==0){return tempos[0];}
	else if (number==1){return tempos[1];}
	else if (number==2){return tempos[2];}
	else{return tempos[3];}
}



int main(void)
{
	// pins ports declaration
	buzzer_DDR |= _BV(buzzer_PIN) ;// make buzzer pin output
	//set button pins as inputs
	buzzer_DDR &= ~_BV(playbutton);
	buzzer_DDR &= ~_BV(changenextsong);
	
	
	EICRA |= _BV(ISC01) | _BV(ISC00);	 // set rising edge method
	EIMSK |= _BV(INT0);					// enable pin PD1 External interrupt  
	
    sei();							  //  this Enables the Global Interrupt Enable (I-bit) in the Status Register (SREG)
	
    while (1) 
    {
		if (PINB & _BV(playbutton)){
			sp=1;
			play(songnumber);
		}
		
		if (PINB & _BV(changenextsong))
		{
			changesong();
			
		}
    _delay_ms(100);
	}
}



/**
Use 16bit timer to generate square pulses
You need to give the comparison bit which is used in CTC mode
Using 1024 prescaler so the we can produce all the frequencies we desire
Currently bit unstable
The built in clock pin is using here
**/


void feq_ctc(int step){                          // using built in 16bit timer with prescaler 1024 CTC mode.
	TCCR1B = (1 << WGM12); // enable CTC mode
	OCR1A =step;                              // step is the comparison step number
	TIMSK1 = (1 << OCIE1A);					  // set for CTC Mode
	
	sei();									// Call interrupt
	
	TCCR1B |= (1 << CS12) | (1 << CS10);     // Prescaler 1024
	//{Problem with pulse width and low frequancies not working well}
}

/**
Use delays and CPU cycles to generate square waves
Take frequency and the duration as inputs
get period in miliseconds by T=1/feqruancy *1000 ms
and Calculate how many times the signal has to repeat by cycles=Duration/Period
We will treat it as an integer. 
The 50% square wave is generated by 50% of the period time pin High (5V)
50% of the period_time Low (0V)

**/

void Delay_note_method(float freq,float dur){
	
	// variables
		long int i,cycles;     
		float half_period,period;
		
		if (freq != 0){
			period=(1/freq)*1000;
			cycles=dur/period;
			half_period = period/2;

			for (i=0;i<cycles;i++)
			{
				//50% duty cycle
				
				_delay_ms(half_period);
				
				buzzer_PORT |= _BV(buzzer_PIN);
				
				_delay_ms(half_period);
				
				buzzer_PORT &= ~ _BV(buzzer_PIN);
			}
		}
		else {
			_delay_ms(dur);   // rest
		}

		return;
		
		
}
/** 
play function
This will take one input. The song number, starting from 0
It will take tempo and the chords and duration data from melodies.h accordingly
you can change the tone generating method between declared functions
Tempo is the 
**/
void play(int number){
	int tempo=tempochoose(number); // choose the right tempo from the list using the function
	int notes = sizeof(melody0[number])/sizeof(melody0[number][0])/2;
	int wholenote = (60000 * 2.5) / tempo;
	int divider = 0;
	int noteDuration = 0;
	
	for (int thisNote = 0; thisNote < notes *2 ; thisNote = thisNote + 2) {
		if (sp==1){
		divider = pgm_read_word_near(melody0[number]+thisNote + 1);
		
		if (divider > 0) { //positive divider ( positive duration)
			// it is a regular note
		noteDuration = (wholenote) / divider;}
		
		else if (divider<0){
			noteDuration=(wholenote)/(divider);// negative durations need to be increased and made positive
			noteDuration=noteDuration*1.5;
			noteDuration=-noteDuration;
		}
		else if(divider==0){return;} // this will be used to stop playing
		
		Delay_note_method((pgm_read_word_near(melody0[number]+thisNote)), noteDuration*0.9 ); // play the selected pitch
		_delay_ms(noteDuration*0.5);
		
		}
		else{
		sp=0;
		break;
		}
	}
	
		
	
}
