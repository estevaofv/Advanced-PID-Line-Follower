#include <pololu/PololuQTRSensors.h>
#include <pololu/OrangutanDigital.h>
#include <pololu/OrangutanPushbuttons.h>
#include <pololu/OrangutanTime.h>
#include <pololu/OrangutanMotors.h>

// Initializes the 3pi, displays a welcome message, calibrates, and
// plays the initial music.
void initialize()
{
	unsigned char qtr_rc_pins[] = {18,19,2,4,7,8,9,10};
	unsigned int counter; 
	qtr_rc_init(qtr_rc_pins, 8, 2000, 11);
	while(!button_is_pressed(BUTTON_B))
	{		
		set_digital_output(13,0xff);
		delay_ms(200);
	}
	wait_for_button_release(BUTTON_B);
	set_digital_output(13,LOW);
	delay_ms(1000);
	
	for(counter=0;counter<80;counter++)
	{
		if(counter < 20 || counter >= 60)
			set_motors(90,-90);         //**************************************************
		else
			set_motors(-90,90);
		
		set_digital_output(13,HIGH);
		qtr_calibrate(QTR_EMITTERS_ON);   //**************************************************
		delay_ms(20);
	}
	set_digital_output(13,LOW);
	set_motors(0,0);

	while(!button_is_pressed(BUTTON_B))
	{
		set_digital_output(13,0xff);
		delay_ms(200);
	}

	wait_for_button_release(BUTTON_B);
	set_digital_output(13,HIGH);
}

char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right)
{
	if(found_left)
	return 'L';
	else if(found_right)
	return 'R';
	else
	return 'S';
	
}

void turn(char dir)
{
	unsigned int sensors[8];
	unsigned int position = qtr_read_line(sensors,QTR_EMITTERS_ON);
	switch(dir)
	{
	case 'L':
		// Turn left.
		set_motors(-70,70);
		while(!(position > 3000 && position < 4000))
		{
			position = qtr_read_line(sensors,QTR_EMITTERS_ON);
		}
		break;
	case 'R':
		// Turn right.
		set_motors(70,-70);
		while(!(position > 3000 && position < 4000))
		{
			position = qtr_read_line(sensors,QTR_EMITTERS_ON);
		}
		break;
	case 'S':
		// Don't do anything!
		break;
	}
}


int main()
{
	unsigned int sensors[8]; 
	unsigned int last_proportional=0;
	long integral=0;
	

	initialize();
	

	while(1)
	{   
				
		set_digital_output(13,HIGH);
	
		unsigned int position = qtr_read_line(sensors,QTR_EMITTERS_ON);
		
		int proportional = ((int)position) - 3500; 
		int derivative = proportional - last_proportional;
		integral += proportional;
		last_proportional = proportional;
		int power_difference = proportional/20 + integral/10000 + derivative*3/2;
			
		unsigned char found_left=0;
		unsigned char found_straight=0;
		unsigned char found_right=0;

		position = qtr_read_line(sensors,QTR_EMITTERS_ON);

		// Check for left and right exits.
		if(sensors[0] > 500)
		{
			found_left = 1;
		}
		if(sensors[7] > 500)
		{
			found_right = 1;
		}
		// Check for a straight exit.
		if (found_left || found_right)
		{
			set_motors(40,40);
			delay_ms(100);
		}
		position = qtr_read_line(sensors,QTR_EMITTERS_ON);
		if(sensors[0] > 600 && sensors[7] > 600)
		{
			found_straight = 1;
			found_left=0;
			found_right=0;
		}

		unsigned char dir = select_turn(found_left, found_straight, found_right);

		turn(dir);
		
		const int max = 230;
		if(power_difference > max)
			power_difference = max;
		if(power_difference < -max)
			power_difference = -max;

		if(power_difference < 0)
			set_motors(max+power_difference, max);
		else
			set_motors(max, max-power_difference);
	}
	
}

