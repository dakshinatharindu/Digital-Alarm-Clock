

using namespace std;
#include "stdlib.h"
#include "Alarm.h"
#include "Display.h"
#include "util/delay.h"
#include <avr/io.h>
#include "ds1307.h"
#include "Buzzer.h" //This is used to play the tone with the speaker
#define BUTTON_DELAY 500
#ifndef F_CPU
#define F_CPU 1000000UL
#endif

// default constructor
Alarm::Alarm()
{
} //Alarm

// default destructor
Alarm::~Alarm()
{
} //~Alarm
int numberOfAlarms = 0;
int alarmArray[10];
int toneArray[10];
int monthsDays[12]={31,28,31,30,31,30,31,31,30,31,30,31};
uint8_t year = 0;
uint8_t month = 0;
uint8_t day = 0;
uint8_t dayofweek = 0;
uint8_t hour = 0;
uint8_t minute = 0;
uint8_t second = 0;
int powerOf(int base, int power){
	int result = 1;
	while(power!=0){
		result*=base;
		power--;
	}
	return result;
}
bool isPress(uint8_t prt){
	if (PIND == 1<< prt){
		return true;
	}
	else{
		return false;
	}
}
void ringAlarm(int tone){
	LCD_Clear();
	LCD_String("RING!!");
	play(tone);				//Trigger buzzer. Can be interrupted by the int1 button.
	//_delay_ms(2000);
}
void setAlarm(int alarm_time){
	if (alarm_time<2400){
		alarmArray[numberOfAlarms] = alarm_time;
		numberOfAlarms++;
	}
}
void setTone(int t){
	toneArray[numberOfAlarms] = t;
}
void updateAlarmArray(int removeAlarm){
	for (int i=0;i<numberOfAlarms;i++){
		if (i<removeAlarm){
			alarmArray[i] = alarmArray[i];
			toneArray[i] = toneArray[i];
		}
		else{
			alarmArray[i] = alarmArray[i+1];
			toneArray[i] = toneArray[i+1];
		}
	}
}
void checkAlarm(){
	if (numberOfAlarms != 0){
		ds1307_getdate(&year, &month, &day, &dayofweek, &hour, &minute, &second);
		for (int i = 0; i <= numberOfAlarms;i++){
			if (alarmArray[i]/100 == hour && alarmArray[i]%100 == minute){
				ringAlarm(toneArray[i]);
				updateAlarmArray(i);
				numberOfAlarms--;
			}
		}
		
	}
}
void setClockTime(int clockTime[6]){
	int y = clockTime[0];
	int m = clockTime[1];
	int d = clockTime[2];
	int D = getDay(y,m,d);
	int h = clockTime[3];
	int M = clockTime[4];
	int s = clockTime[5];
	if ((0<y) & (y<99) & (0<m)&(m<13) & (0<d)&(d<32) & (0<=h)&(h<24) & (0<=M)&(M<60) & (0<=s)&(s<60)){
		ds1307_setdate(y, m, d, D, h, M, s);
	}
}
int getDay(int y, int m, int d){
	int days = 0;
	for (int i = 1;i<=y;i++){
		if (i%4 == 0){
			days+= 366;
		}
		else{
			days+= 365;
		}
		
	}
	for (int j = 0;j<m-1;j++){
		if ((y%4==0) & (j == 1)){
			days+=monthsDays[j];
			days+=1;
		}
		else{
			days +=monthsDays[j];
		}
	}
	return (days+d+5)%7;
}
void displayValue(int val){
	char valS[10];
	itoa(val,valS,10);
	LCD_String(valS);
}
void displayZero(uint8_t digit){
	if (digit<10){
		LCD_String("0");
	}
}
void displayTime(){
	ds1307_getdate(&year, &month, &day, &dayofweek, &hour, &minute, &second);
// 	char secondS[10];char minuteS[10];char hourS[10];char yearS[10];char monthS[10];char dayS[10];
// 	itoa(second,secondS,10);itoa(minute,minuteS,10);itoa(hour,hourS,10);itoa(year,yearS,10);itoa(month,monthS,10);itoa(day,dayS,10);
	LCD_Home(0);LCD_String("   20");
	displayZero(year);displayValue(year);LCD_String("-");
	displayZero(month);displayValue(month);LCD_String("-");
	displayZero(day);displayValue(day);
	LCD_Home(1);LCD_String("    ");
	displayZero(hour);
	displayValue(hour);
	LCD_String(":");
	displayZero(minute);
	displayValue(minute);
	LCD_String(":");
	displayZero(second);
	displayValue(second);
	//_delay_ms(1);

}

int alarm_var = 0;
char alarms[10];
void showAlarms(char key,int state){
	LCD_Home(0);
	if ((key=='8') & (state==2)){
		alarm_var++;
		//LCD_Clear();
		_delay_ms(BUTTON_DELAY);
	}
	if ((key=='2') & (state ==2)){
		alarm_var--;
		//LCD_Clear();
		_delay_ms(BUTTON_DELAY);
	}
	if ((key=='5') & (numberOfAlarms!=0)){
		LCD_Clear();
		displayTyping(alarmArray[alarm_var]/100,3);LCD_String(":");displayTyping(alarmArray[alarm_var]%100,3);
		LCD_String(" DELETED");LCD_Home(1);LCD_String("SUCCESSFULLY");
		_delay_ms(3000);
		LCD_Clear();
		updateAlarmArray(alarm_var);
		numberOfAlarms--;
		_delay_ms(BUTTON_DELAY);
		
		
	}
	if (alarm_var>=numberOfAlarms){
		alarm_var=0;
	}
	if (alarm_var<0){
		alarm_var=numberOfAlarms-1;
	}
	if (state==2){
		LCD_Home(0);
		if (numberOfAlarms==0){
			LCD_String("   NO ALARMS");
			LCD_Home(1);
			LCD_String("    ARE SET");
		}
		else{
			LCD_String("PRESS 5 TO DEL");
			LCD_Home(1);
			LCD_String(">>");
			displayTyping(alarmArray[alarm_var]/100,3);LCD_String(":");displayTyping(alarmArray[alarm_var]%100,3);
			
// 			displayTyping(alarmArray[(alarm_var+1)%numberOfAlarms]/100,3);LCD_String(":");
// 			displayTyping(alarmArray[(alarm_var+1)%numberOfAlarms]%100,3);
		}
	}
}
unsigned int timerTime[3];
int timerState = 0;
int temp1 = 0;
int data1 = 0;
int start=0;
void timer(char key, int state){
	
	LCD_Home(1);
	LCD_String("   ");
	for (int i=0;i<3;i++){
		if ((i!=3) & (i!=0)){
			LCD_String(":");
		}
		if (i==timerState){
			displayTyping(timerTime[i],temp1);
		}
		else{
			displayTyping(timerTime[i],3);
		}
	}
	
	if (timerState==3){
		LCD_Home(0);
		
		if (key=='5'){
			start=1;
		}
		if (start==1){
			LCD_String("PRESS # TO STOP ");
			timerCountDown();
		}
		if (start==0){
			LCD_String("PRESS 5 TO START");
		}
		if ((timerTime[2]==0) & (timerTime[1]==0) & (timerTime[0]==0) ){
			start=0;
			timerState=0;
			LCD_Clear();
			LCD_String("TIMES UP!!!");
			play(1);
		}
	}
	else{
		LCD_Home(0);
		LCD_String("ENTER THE TIME");
	}
	if ((int(key)>47) & (int(key)<58) & (state ==2) & (timerState!=3)){
		data1+=(int(key)-48);
		_delay_ms(BUTTON_DELAY);
		if (temp1 == 0){
			data1*=10;
			temp1 = 1;
			timerTime[timerState] = data1;
		}
		else{
			timerTime[timerState] = data1;
			timerState++;
			data1=0;
			temp1 = 0;
		}
	}
	
}

int tempSec=60;
void timerCountDown(){
	ds1307_getdate(&year, &month, &day, &dayofweek, &hour, &minute, &second);
	
	if (tempSec!=second){
		timerTime[2]-=1;
		tempSec=second;
	}
	if (timerTime[2]==-1){
		if (timerTime[1]==0){
			if (timerTime[0]!=0){
				timerTime[1]=59;timerTime[0]-=1;
			}
		}
		else{
		timerTime[2]=59;timerTime[1]-=1;	
	}
}
}
void resetAlarmVariables(){
	tempSec=60;
	timerTime[0]=0;timerTime[1]=0;timerTime[2]=0;
	timerState = 0;
	temp1 = 0;
	data1 = 0;
	start=0;
}