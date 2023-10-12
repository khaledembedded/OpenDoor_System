/*
 * main.c
 *
 *  Created on: Jul 7, 2020
 *      Author: khaled
 */

#include "../Lib/Std_types.h"
#include "../Lib/Bit_math.h"


#include "I2c_int.h"
#include "Eeprom_int.h"
#include "Keypad_int.h"
#include "Lcd_int.h"
#include "Servo_int.h"

#undef F_CPU
#define F_CPU 8000000
#include <util/delay.h>

#define PASSWORD_ARR_LENGTH      4
#define PASSWORDCONF_ARR_LENGTH  4

#define PASSWORD_FLAG_LOC      4
#define SYSTEM_FLAG_LOC        5


#define PASSWORD_EXIST    1+0x30
#define SYSTEM_NOTBLOCKED 0+0x30
#define SYSTEM_BLOCKED    1+0x30

#define CLOSE_DOOR_ANGLE   0
#define OPEN_DOOR_ANGLE   90

u8 Password[4];
u8 Password_Confirm[4];


int main(void)
{
	/* flag to check password existence in Eeprom MEM  */
	u8 Password_flag;
	/* flag to check system availability */
	u8 System_flag ;
	/* iterator to loop in PW arrays */
	u8 j ;
	/* iterator to try only 3 times to open the system */
	u8 NO_Itiration ;

	/* Initialize EEPROM module */
	Eeprom_vidInit();
	/* Initialize LCD module */
	Lcd_vidInit();
	/* Initialize Keypad module */
	Keypad_vidInit();
	/* Initialize SERVO motor module */
	ServoMotor_vidInt(CHANNEL_A);


	/* display system open MSG  */
   //Lcd_vidSendCommand(LCD_u8CMD_CLR_DISPLAY);
   //_delay_ms(2);
   //Lcd_vidWriteString("SYS Opening");
   //_delay_ms(500);
	

	while(1)
	{
		SYS_BEGIN :
		/* check system block flag (read it from EEPROM) */
		Eeprom_u8ReadByte(EEPROM_u8DEVICE_0 , SYSTEM_FLAG_LOC, &System_flag);
		/* if the system is available (the flag is cleared) */
		if(System_flag == SYSTEM_NOTBLOCKED)
		{
			/* check password existence flag (read it from EEPROM) */
			Eeprom_u8ReadByte(EEPROM_u8DEVICE_0 , PASSWORD_FLAG_LOC, &Password_flag);

			/* if there is a password saved in EEPROM (the flag is set) */
			if(Password_flag == PASSWORD_EXIST)
			{
				for(NO_Itiration=0;NO_Itiration<3;NO_Itiration++)
				{
					PW_ENTER :
					/* display a require password from the user MSG  */
					Lcd_vidSendCommand(LCD_u8CMD_CLR_DISPLAY);
					_delay_ms(2);
					Lcd_vidWriteString("enter the PW:");
					Lcd_vidSendCommand(LCD_u8CMD_BEGIN_CURS_2ND_RAW);
					_delay_ms(2);
					/* take the password from the user */
					for(;;)
					{
						Key_u8RecievedData = Keypad_u8GetKey();
						if (Key_u8RecievedData != KEYPAD_u8NO_KEY)
							break;
					}
					for(j=0;j<PASSWORDCONF_ARR_LENGTH;j++)
					{
						Key_u8RecievedData = Keypad_u8GetKey();
						/* save the password that the user entered */
						Password_Confirm[j] = Key_u8RecievedData+0x30 ;
						/* display the password encrypted */
						Lcd_vidWriteChar('*');
						_delay_ms(1200);
					}
					/* Read the saved password from EEPROM */
					for(j=0 ; j < PASSWORD_ARR_LENGTH ; j++)
					{
						Eeprom_u8ReadByte(EEPROM_u8DEVICE_0 , j, &Password[j]);
						_delay_ms(10);
					}
					/*compare 2 passwords*/
					/* if the password is correct */
					if((Password[0]==Password_Confirm[0])&&(Password[1]==Password_Confirm[1])&&(Password[2]==Password_Confirm[2])&&(Password[3]==Password_Confirm[3]))
					{
						/* display welcome MSG */
						Lcd_vidSendCommand(LCD_u8CMD_CLR_DISPLAY);
						_delay_ms(2);
						Lcd_vidWriteString("Welcome");
						/* open the door */
						ServoMotor_vidTurOn(CHANNEL_A, OPEN_DOOR_ANGLE);
						_delay_ms(2000);
						ServoMotor_vidTurOn(CHANNEL_A, CLOSE_DOOR_ANGLE);
						_delay_ms(2000);
						goto SYS_BEGIN;

					}
					/* if the password is not correct */
					else if(NO_Itiration<2)
					{
						/* display try again MSG to the user  */
						Lcd_vidSendCommand(LCD_u8CMD_CLR_DISPLAY);
						_delay_ms(2);
						Lcd_vidWriteString("TRY AGAIN!");
						_delay_ms(500);
						continue  ;
					}
					/* after last iteration */
					else
					{
						/* set the flag of system block in EEPROM */
						Eeprom_u8WriteByte(EEPROM_u8DEVICE_0, SYSTEM_FLAG_LOC, SYSTEM_BLOCKED);
						_delay_ms(10);
						break ;
					}

					
				}
			}
			/* if there is no password saved in EEPROM (the flag is cleared) */
			else
			{
				/* guide the user to the password updater application */
				Lcd_vidWriteString("1ST_time SignUp");
				Lcd_vidSendCommand(LCD_u8CMD_BEGIN_CURS_2ND_RAW);
				_delay_ms(2);
				Lcd_vidWriteString("Go to PW updater");
			}
		}

		/* if the system is blocked (the flag is set) */
		else
		{
			/* Block the system(do not open it until go to password updater application and modify the PW) */
			Lcd_vidSendCommand(LCD_u8CMD_CLR_DISPLAY);
			_delay_ms(2);
			Lcd_vidWriteString("SYS is blocked");
			_delay_ms(20);
			break ;
		}
		
	}
	return 0 ;
}
