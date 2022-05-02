///////////////////////////////////////////////////////////////////
/*  HandHeld4 SSB TRX                                            */
/*  Mikrocontroller:  ATMEL AVR ATmega328p 8 MHz                 */
///////////////////////////////////////////////////////////////////
/*  Compiler:         GCC (GNU AVR C-Compiler)                   */
/*  Autor:            Peter Rachow                               */
/*  Letzte Aenderung:                                            */
///////////////////////////////////////////////////////////////////
#define CPUCLK 8

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

//EEPROM
//Byte		  Function
// 0          Last VFO used
// 4..7, etc. 4 bytes each for every VFO freq

//INPUT
//Analog
//PWR-Meter: PC2
//S-Meter:   PC3
//VDD check: PC1

//Keys
//PD0, PD1, PD2

//Rotary encoder
//PB6, PB7

#define MAXVFO 16
#define INTERFREQUENCY 10000000

//Band options
#define BANDOPTION 20

#if (BANDOPTION == 20) //14MHz
    #define BAND_CENTER 14200000 
    #define BAND_0 14000000
    #define BAND_1 14350000
#endif  

#if (BANDOPTION == 17) //18MHz
    #define BAND_CENTER 18100000
    #define BAND_0 18068000
    #define BAND_1 18168000
#endif

//////////////////////////////////////
//   L   C   D   
//////////////////////////////////////
#define OLEDADDR 0x78
#define OLEDCMD 0x00   //Command follows
#define OLEDDATA 0x40  //Data follows

#define FONTW 6
#define FONTH 8

#define S_SETLOWCOLUMN           0x00
#define S_SETHIGHCOLUMN          0x10
#define S_PAGEADDR               0xB0
#define S_SEGREMAP               0xA0

#define S_LCDWIDTH               128
#define S_LCDHEIGHT              64 

// Font 6x8 for OLED
const char font[97][6] PROGMEM={
{0x00,0x00,0x00,0x00,0x00,0x00},	// 0x20
{0x00,0x00,0x06,0x5F,0x06,0x00},	// 0x21
{0x00,0x07,0x03,0x00,0x07,0x03},	// 0x22
{0x00,0x24,0x7E,0x24,0x7E,0x24},	// 0x23
{0x00,0x24,0x2B,0x6A,0x12,0x00},	// 0x24
{0x00,0x63,0x13,0x08,0x64,0x63},	// 0x25
{0x00,0x36,0x49,0x56,0x20,0x50},	// 0x26
{0x00,0x00,0x07,0x03,0x00,0x00},	// 0x27
{0x00,0x00,0x3E,0x41,0x00,0x00},	// 0x28
{0x00,0x00,0x41,0x3E,0x00,0x00},	// 0x29
{0x00,0x08,0x3E,0x1C,0x3E,0x08},	// 0x2A
{0x00,0x08,0x08,0x3E,0x08,0x08},	// 0x2B
{0x00,0x00,0xE0,0x60,0x00,0x00},	// 0x2C
{0x00,0x08,0x08,0x08,0x08,0x08},	// 0x2D
{0x00,0x00,0x60,0x60,0x00,0x00},	// 0x2E
{0x00,0x20,0x10,0x08,0x04,0x02},	// 0x2F
{0x00,0x3E,0x51,0x49,0x45,0x3E},	// 0x30
{0x00,0x00,0x42,0x7F,0x40,0x00},	// 0x31
{0x00,0x62,0x51,0x49,0x49,0x46},	// 0x32
{0x00,0x22,0x49,0x49,0x49,0x36},	// 0x33
{0x00,0x18,0x14,0x12,0x7F,0x10},	// 0x34
{0x00,0x2F,0x49,0x49,0x49,0x31},	// 0x35
{0x00,0x3C,0x4A,0x49,0x49,0x30},	// 0x36
{0x00,0x01,0x71,0x09,0x05,0x03},	// 0x37
{0x00,0x36,0x49,0x49,0x49,0x36},	// 0x38
{0x00,0x06,0x49,0x49,0x29,0x1E},	// 0x39
{0x00,0x00,0x6C,0x6C,0x00,0x00},	// 0x3A
{0x00,0x00,0xEC,0x6C,0x00,0x00},	// 0x3B
{0x00,0x08,0x14,0x22,0x41,0x00},	// 0x3C
{0x00,0x24,0x24,0x24,0x24,0x24},	// 0x3D
{0x00,0x00,0x41,0x22,0x14,0x08},	// 0x3E
{0x00,0x02,0x01,0x59,0x09,0x06},	// 0x3F
{0x00,0x3E,0x41,0x5D,0x55,0x1E},	// 0x40
{0x00,0x7E,0x11,0x11,0x11,0x7E},	// 0x41
{0x00,0x7F,0x49,0x49,0x49,0x36},	// 0x42
{0x00,0x3E,0x41,0x41,0x41,0x22},	// 0x43
{0x00,0x7F,0x41,0x41,0x41,0x3E},	// 0x44
{0x00,0x7F,0x49,0x49,0x49,0x41},	// 0x45
{0x00,0x7F,0x09,0x09,0x09,0x01},	// 0x46
{0x00,0x3E,0x41,0x49,0x49,0x7A},	// 0x47
{0x00,0x7F,0x08,0x08,0x08,0x7F},	// 0x48
{0x00,0x00,0x41,0x7F,0x41,0x00},	// 0x49
{0x00,0x30,0x40,0x40,0x40,0x3F},	// 0x4A
{0x00,0x7F,0x08,0x14,0x22,0x41},	// 0x4B
{0x00,0x7F,0x40,0x40,0x40,0x40},	// 0x4C
{0x00,0x7F,0x02,0x04,0x02,0x7F},	// 0x4D
{0x00,0x7F,0x02,0x04,0x08,0x7F},	// 0x4E
{0x00,0x3E,0x41,0x41,0x41,0x3E},	// 0x4F
{0x00,0x7F,0x09,0x09,0x09,0x06},	// 0x50
{0x00,0x3E,0x41,0x51,0x21,0x5E},	// 0x51
{0x00,0x7F,0x09,0x09,0x19,0x66},	// 0x52
{0x00,0x26,0x49,0x49,0x49,0x32},	// 0x53
{0x00,0x01,0x01,0x7F,0x01,0x01},	// 0x54
{0x00,0x3F,0x40,0x40,0x40,0x3F},	// 0x55
{0x00,0x1F,0x20,0x40,0x20,0x1F},	// 0x56
{0x00,0x3F,0x40,0x3C,0x40,0x3F},	// 0x57
{0x00,0x63,0x14,0x08,0x14,0x63},	// 0x58
{0x00,0x07,0x08,0x70,0x08,0x07},	// 0x59
{0x00,0x71,0x49,0x45,0x43,0x00},	// 0x5A
{0x00,0x00,0x7F,0x41,0x41,0x00},	// 0x5B
{0x00,0x02,0x04,0x08,0x10,0x20},	// 0x5C
{0x00,0x00,0x41,0x41,0x7F,0x00},	// 0x5D
{0x00,0x04,0x02,0x01,0x02,0x04},	// 0x5E
{0x80,0x80,0x80,0x80,0x80,0x80},	// 0x5F
{0x00,0x00,0x03,0x07,0x00,0x00},	// 0x60
{0x00,0x20,0x54,0x54,0x54,0x78},	// 0x61
{0x00,0x7F,0x44,0x44,0x44,0x38},	// 0x62
{0x00,0x38,0x44,0x44,0x44,0x28},	// 0x63
{0x00,0x38,0x44,0x44,0x44,0x7F},	// 0x64
{0x00,0x38,0x54,0x54,0x54,0x08},	// 0x65
{0x00,0x08,0x7E,0x09,0x09,0x00},	// 0x66
{0x00,0x18,0xA4,0xA4,0xA4,0x7C},	// 0x67
{0x00,0x7F,0x04,0x04,0x78,0x00},	// 0x68
{0x00,0x00,0x00,0x7D,0x40,0x00},	// 0x69
{0x00,0x40,0x80,0x84,0x7D,0x00},	// 0x6A
{0x00,0x7F,0x10,0x28,0x44,0x00},	// 0x6B
{0x00,0x00,0x00,0x7F,0x40,0x00},	// 0x6C
{0x00,0x7C,0x04,0x18,0x04,0x78},	// 0x6D
{0x00,0x7C,0x04,0x04,0x78,0x00},	// 0x6E
{0x00,0x38,0x44,0x44,0x44,0x38},	// 0x6F
{0x00,0xFC,0x44,0x44,0x44,0x38},	// 0x70
{0x00,0x38,0x44,0x44,0x44,0xFC},	// 0x71
{0x00,0x44,0x78,0x44,0x04,0x08},	// 0x72
{0x00,0x08,0x54,0x54,0x54,0x20},	// 0x73
{0x00,0x04,0x3E,0x44,0x24,0x00},	// 0x74
{0x00,0x3C,0x40,0x20,0x7C,0x00},	// 0x75
{0x00,0x1C,0x20,0x40,0x20,0x1C},	// 0x76
{0x00,0x3C,0x60,0x30,0x60,0x3C},	// 0x77
{0x00,0x6C,0x10,0x10,0x6C,0x00},	// 0x78
{0x00,0x9C,0xA0,0x60,0x3C,0x00},	// 0x79
{0x00,0x64,0x54,0x54,0x4C,0x00},	// 0x7A
{0x00,0x08,0x3E,0x41,0x41,0x00},	// 0x7B
{0x00,0x00,0x00,0x77,0x00,0x00},	// 0x7C
{0x00,0x00,0x41,0x41,0x3E,0x08},	// 0x7D
{0x00,0x02,0x01,0x02,0x01,0x00},	// 0x7E
{0x00,0x3C,0x26,0x23,0x26,0x3C},	// 0x7F
{0x00,0x1E,0xA1,0xE1,0x21,0x12}};	// 0x80
///////////////////////////
//     DECLARATIONS
///////////////////////////
//
//OLED
void oled_command(int value);
void oled_data(unsigned int*, unsigned int);
void oled_gotoxy(unsigned int, unsigned int);
void oled_cls(int);
void oled_init(void);
void oled_byte(unsigned char);
void oled_putchar1(unsigned int x, unsigned int y, unsigned char ch, int);
void oled_putchar2(unsigned int x, unsigned int y, unsigned char ch, int);
void oled_putnumber(int, int, long, int, int, int);
void oled_putstring(int, int, char*, char, int);
void oled_write_section(int, int, int, int);

//I²C
void twi_init(void);
void twi_start(void);
void twi_stop(void);
uint8_t twi_read_ack(void);
uint8_t twi_read_not_ack(void);
uint8_t twi_get_status(void);

//String
int int2asc(long num, int dec, char *buf, int buflen);
  
////////////////
// SPI for DDS
////////////////
#define DDS_DDR DDRB
#define DDSPORT PORTB
#define FSYNC 0 //PB0
#define SDATA 1 //PB1
#define SCLK  2 //PB2

//Radio display
void show_frequency(long);
void show_meter(int);
void show_voltage(int);
void show_vfo(int);
void show_vfo_freq(int);
void draw_meter_scale(void);

//DDS
void spi_send_word(unsigned int);
void set_frequency(long, int);
void wait_ms(int);

//KEYS
int get_keys(void);

//ADC
int get_adc(int);

//Tuning
int calc_tuningfactor(void);

//EEPROM
void store_frequency(long, int);
long load_frequency(int);
int is_mem_freq_ok(long);
void store_last_vfo(int);
int load_last_vfo(void);

//MISC
int main(void);

////////////////////
//    Variables   //
////////////////////
//Global variables
//Tuning
int tuningcount = 0;
int tuning = 0;
int laststate = 0; //Last state of rotary encoder

//Seconds counting
long runseconds10 =  0;
long runsecs10b = 0; //Timer S-Meter display

//METER
int sv_old = 0;

//Delaytime fck = 8.000 MHz 
void wait_ms(int ms)
{
    int t1, t2;

    for(t1 = 0; t1 < ms; t1++)
    {
        for(t2 = 0; t2 < 137 * CPUCLK; t2++)
        {
            asm volatile ("nop" ::);
        }   
     }    
}

///////////////////////////
//
//         TWI
//
///////////////////////////
void twi_init(void)
{
    //set SCL to 400kHz
    TWSR = 0x00;
    TWBR = 0x0C;
	
    //enable TWI
    TWCR = (1<<TWEN);
}

//Send start signal
void twi_start(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

//send stop signal
void twi_stop(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void twi_write(uint8_t u8data)
{
    TWDR = u8data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

uint8_t twi_get_status(void)
{
    uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
}

/////////////////////
//                 //  
//  OLED routines  //
//                 // 
/////////////////////
//Send comand to OLED
void oled_command(int value)
{
   twi_start();
   twi_write(OLEDADDR); //Device address
   twi_write(OLEDCMD);  //Command follows
   twi_write(value);    //Send value
   twi_stop();
} 

//Send a 'number' bytes of data to display - from RAM
void oled_data(unsigned int *data, unsigned int number)
{
   int t1;
   twi_start();
   twi_write(OLEDADDR); //Device address
   twi_write(OLEDDATA); //Data follows
   
   for(t1 = 0; t1 < number; t1++)
   {
      twi_write(data[t1]); //send the byte(s)
   }   
   twi_stop ();   
}

//Set "cursor" to current position to screen
void oled_gotoxy(unsigned int x, unsigned int y)
{
   //int x2 = x + 2; //SH1106
   int x2 = x;       //SSD1306
   twi_start();
   twi_write(OLEDADDR); //Select display  I2C address
   twi_write(OLEDCMD);  //Be ready for command
   twi_write(S_PAGEADDR + y); //Select display row
   twi_write(S_SETLOWCOLUMN + (x2 & 0x0F)); //Col addr lo byte
   twi_write(S_SETHIGHCOLUMN + ((x2 >> 4) & 0x0F)); //Col addr hi byte
   twi_stop();
}

void oled_cls(int invert)
{
    unsigned int row, col;

    //Just fill the memory with zeros
    for(row = 0; row < S_LCDHEIGHT / 8; row++)
    {
        oled_gotoxy(0, row); //Set OLED address
        twi_start();
        twi_write(OLEDADDR); //Select OLED
        twi_write(OLEDDATA); //Data follows
        for(col = 0; col < S_LCDWIDTH; col++)
        {
            if(!invert)
            {
                twi_write (0); //normal
            }   
            else
            {
                twi_write(255); //inverse
            } 
        }
        twi_stop();
    }
    oled_gotoxy(0, 0); //Return to 0, 0
}

//Write bitmap to one row of screen
void oled_write_section(int x1, int x2, int row, int number)
{
    int t1;
    oled_gotoxy(x1, row);
    	
    twi_start();
    twi_write(OLEDADDR); //Device address
    twi_write(OLEDDATA); //Data follows
   
    for(t1 = x1; t1 < x2; t1++)
    {
       twi_write(number); //send the byte(s)
    }    
    twi_stop ();   
}

//Initialize OLED
void oled_init(void)
{
    oled_command(0xAE); // Display OFF
	oled_command(0x20); // Set Memory Addressing Mode
    oled_command(0x00); // HOR
    
    oled_command(0xB0);    // Set Page Start Address for Page Addressing Mode, 0-7
    oled_command(0xC8);    // Set COM Output Scan Direction
    oled_command(0x00);    // --set low column address
    oled_command(0x10);    // --set high column address
    oled_command(0x40);    // --set start line address
    oled_command(0x81);
    oled_command(0xFF);    // Set contrast control register
    oled_command(0xA1);    // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
    oled_command(0xA6);    // Set display mode. A6=Normal; A7=Inverse
    oled_command(0xA8);
    oled_command(0x3F);    // Set multiplex ratio(1 to 64)
    oled_command(0xA4);    // Output RAM to Display
					       // 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
    oled_command(0xD3);
    oled_command(0x00);    // Set display offset. 00 = no offset
    oled_command(0xD5);    // --set display clock divide ratio/oscillator frequency
    oled_command(0xF0);    // --set divide ratio
    oled_command(0xD9); 
    oled_command(0x22);    // Set pre-charge period
    oled_command(0xDA);
    oled_command(0x12);    // Set com pins hardware configuration
    oled_command(0xDB);    // --set vcomh
    oled_command(0x20);    // 0x20,0.77xVcc
    oled_command(0x8D);
    oled_command(0x14);    // Set DC-DC enabl
    oled_command(0xAF);    //Display ON
   
} 

//Write 1 byte pattern to screen using vertical orientation 
void oled_byte(unsigned char value)
{
   twi_start();
   twi_write(OLEDADDR); //Device address
   twi_write(OLEDDATA); //Data follows
   twi_write(value);
   twi_stop ();   
}

//Write character to screen (normal size);
void oled_putchar1(unsigned int x, unsigned int y, unsigned char ch, int invert)
{
	int t0;
		
	oled_gotoxy(x, y);
	for(t0 = 0; t0 < FONTW; t0++)
	{
		if(!invert)
		{
            oled_byte(pgm_read_byte(&font[ch - 32][t0]));
        }
        else    
        {
            oled_byte(~pgm_read_byte(&font[ch - 32][t0]));
        }
        
	}
}		

//Write character to screen (DOUBLE size);
void oled_putchar2(unsigned int x, unsigned int y, unsigned char ch, int invert)
{
	int t0, t1;
	char c;
	int i[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	for(t0 = 0; t0 < FONTW; t0++)
	{
		for(t1 = 0; t1 < 8; t1++)
		{
			if(!invert)
			{
				c = pgm_read_byte(&font[ch - 32][t0]);
			}
			else
			{
				c = ~pgm_read_byte(&font[ch - 32][t0]);
			}
				
		    if(c & (1 << t1))
		    {
			    i[t0] += (1 << (t1 * 2));
			    i[t0] += (1 << (t1 * 2 + 1));
		    }	
	    }
	}
	
	oled_gotoxy(x, y);
	for(t0 = 0; t0 < FONTW; t0++)
	{		
	    oled_byte(i[t0] & 0xFF);
	    oled_byte(i[t0] & 0xFF);
	}
	
	oled_gotoxy(x, y + 1);
	for(t0 = 0; t0 < FONTW; t0++)
	{		
	    oled_byte((i[t0] & 0xFF00) >> 8);
	    oled_byte((i[t0] & 0xFF00) >> 8);
	}
}		

//Print string in given size
//lsize=0 => normal height, lsize=1 => double height
void oled_putstring(int col, int row, char *s, char lsize, int inv)
{
    int c = col;
	
	while(*s)
	{
	    if(!lsize)
		{
	        oled_putchar1(c, row, *s++, inv);
		}
        else
        {
            oled_putchar2(c, row, *s++, inv);
		}	
		c += (lsize + 1) * FONTW;
	}
}

//Print an integer/long to OLED
void oled_putnumber(int col, int row, long num, int dec, int lsize, int inv)
{
    char *s = malloc(16);
	if(s != NULL)
	{
	    int2asc(num, dec, s, 16);
	    oled_putstring(col, row, s, lsize, inv);
	    free(s);
	}	
}

///////////////////////
//                   //
// STRING FUNCTIONS  //
//                   //
///////////////////////
//INT 2 ASC
int int2asc(long num, int dec, char *buf, int buflen)
{
    int i, c, xp = 0, neg = 0;
    long n, dd = 1E09;

    if(!num)
	{
	    *buf++ = '0';
		*buf = 0;
		return 1;
	}	
		
    if(num < 0)
    {
     	neg = 1;
	    n = num * -1;
    }
    else
    {
	    n = num;
    }

    //Fill buffer with \0
    for(i = 0; i < 12; i++)
    {
	    *(buf + i) = 0;
    }

    c = 9; //Max. number of displayable digits
    while(dd)
    {
	    i = n / dd;
	    n = n - i * dd;
	
	    *(buf + 9 - c + xp) = i + 48;
	    dd /= 10;
	    if(c == dec && dec)
	    {
	        *(buf + 9 - c + ++xp) = '.';
	    }
	    c--;
    }

    //Search for 1st char different from '0'
    i = 0;
    while(*(buf + i) == 48)
    {
	    *(buf + i++) = 32;
    }

    //Add minus-sign if neccessary
    if(neg)
    {
	    *(buf + --i) = '-';
    }

    //Eleminate leading spaces
    c = 0;
    while(*(buf + i))
    {
	    *(buf + c++) = *(buf + i++);
    }
    *(buf + c) = 0;
	
	return c;
}

/////////////////////////////////
//
// DISPLAY FUNCTIONS
//
////////////////////////////////
//Current frequency (double letter height)
void show_frequency(long f)
{
	char *buf;
	int t1 = 0;
	int row = 2, col = 5;
	
	buf = malloc(16);
	
	//Init buffer string
	for(t1 = 0; t1 < 16; t1++)
	{
	    *(buf + t1) = 0;
	}
	
	int2asc(f / 10, 2, buf, 16);
	
	//Display buffer
	for(t1 = 0; *(buf + t1); t1++)
	{
		oled_putchar2(col + t1 * 12, row, *(buf + t1), 0);   
	}	
		
	free(buf);
}

//S-Meter bargraph (Page 6)
void show_meter(int sv0)
{
	int t1, sv, y0 = 6;
	
	sv = sv0;
	if(sv == sv_old)
	{
		return;
	}	
	
	//Clear bar
	if(sv == -1)
    {
	    //Clear s-meter
	    for(t1 = 0; t1 < S_LCDWIDTH; t1 += 2)
	    {
	         oled_write_section(t1, t1 + 1, y0, 0);
	    }
	    return;
	}     
	
    if(sv > S_LCDWIDTH)
	{
	    sv = S_LCDWIDTH;
	}
		
	//Clear bar graph partly, when new s-val smaller than previous
    if(sv < sv_old)
    {
	     //Clear s-meter
	     for(t1 = sv; t1 < sv_old - 2; t1 += 2)
	     {
	          oled_write_section(t1, t1 + 1, y0, 0);
	     }	
    }
    else
    {
		sv_old = sv;
		runsecs10b = runseconds10;
	}	
   
    //Draw bar graph
	for(t1 = 0; t1 < sv; t1 += 2)//
	{
	    oled_write_section(t1, t1 + 1, y0, 0x1E);
	}	  
}

void show_voltage(int v1)
{
    char *buffer;
	int t1, p;
	int xpos = 15, ypos = 0;
		
	buffer= malloc(0x10);
	//Init buffer string
	for(t1 = 0; t1 < 0x10; t1++)
	{
	    *(buffer + t1) = 0;
	}
	
    p = int2asc(v1, 1, buffer, 6) * 6;
    oled_putstring(xpos * 6, ypos, buffer, 0, 0);
	oled_putstring(p + xpos * 6, ypos, "V ", 0, 0);
	free(buffer);
}

void show_vfo(int vfo)
{
	int xpos = 0, ypos = 0;
	
	oled_putstring(xpos * 6, ypos, "VFO:", 0, 0);
	oled_putchar1((xpos + 4) * 6, ypos, vfo + 65, 0);
}	

void show_vfo_freq(int vfo)
{

    long f = load_frequency(vfo);
    
    if(is_mem_freq_ok(f))
    {
		oled_putnumber(6 * FONTW, 0, f / 10, 2, 0, 0);
	}
	else
	{
		oled_putstring(6 * FONTW, 0, "--------", 0, 0);
	}		
}	

void draw_meter_scale(void)
{
	oled_putstring(0, 5, "S1 3 5 7 9 +10 +20dB", 0, 0);
	oled_putstring(0, 7, "0 1W 2W  3W  4W   5W", 0, 0);
}

  /////////////////////////
 //   E  E  P  R  O  M  //
/////////////////////////
void store_frequency(long f, int vfo)
{
    long hiword, loword;
    unsigned char hmsb, lmsb, hlsb, llsb;
	
    int start_adr = (vfo + 1) * 4;
    
	cli();
    hiword = f >> 16;
    loword = f - (hiword << 16);
    hmsb = hiword >> 8;
    hlsb = hiword - (hmsb << 8);
    lmsb = loword >> 8;
    llsb = loword - (lmsb << 8);

    while(!eeprom_is_ready());
    eeprom_write_byte((uint8_t*)start_adr, hmsb);

    while(!eeprom_is_ready());
    eeprom_write_byte((uint8_t*)start_adr + 1, hlsb);

    while(!eeprom_is_ready());
    eeprom_write_byte((uint8_t*)start_adr + 2, lmsb);

    while(!eeprom_is_ready());
    eeprom_write_byte((uint8_t*)start_adr + 3, llsb);
    
    sei();	
}

//Load a frequency from memory by memplace
long load_frequency(int vfo)
{
    long rf;
    unsigned char hmsb, lmsb, hlsb, llsb;
    int start_adr = (vfo + 1) * 4;
		
    cli();
    hmsb = eeprom_read_byte((uint8_t*)start_adr);
    hlsb = eeprom_read_byte((uint8_t*)start_adr + 1);
    lmsb = eeprom_read_byte((uint8_t*)start_adr + 2);
    llsb = eeprom_read_byte((uint8_t*)start_adr + 3);
	sei();
	
    rf = (long) 16777216 * hmsb + (long) 65536 * hlsb + (unsigned int) 256 * lmsb + llsb;
		
	return rf;
}

//Check if freq is in 20m-band
int is_mem_freq_ok(long f)
{
	if(f >= BAND_0 && f <= BAND_1)
	{
		return 1;
	}	
	else
	{
		return 0;
	}		
}	

//Store last VFO used
void store_last_vfo(int vfonum)
{
    int start_adr = 0;
    
	cli();
    
    while(!eeprom_is_ready());
    eeprom_write_byte((uint8_t*)start_adr, vfonum);

    sei();	
}

//Store last VFO stored
int load_last_vfo(void)
{
    int start_adr = 0;
    int vfonum;
    
    cli();
    
	vfonum = eeprom_read_byte((uint8_t*)start_adr);
    
    sei();
	
	if(vfonum >= 0 && vfonum < MAXVFO)
	{
		return vfonum;
	}
	else
	{
		return -1;
	}	
}

///////////////////////
//                   //
//   A   D   C       //     
//                   //
///////////////////////
//Read ADC value
int get_adc(int adc_channel)
{
	
	int adc_val = 0;
	
	ADMUX = (1<<REFS0) + adc_channel;     // Kanal adcmode aktivieren
    wait_ms(3);
	
    ADCSRA |= (1<<ADSC);
    //while(ADCSRA & (1<<ADSC));
	wait_ms(3);
	
	adc_val = ADCL;
    adc_val += ADCH * 256;   
	
	return adc_val;
	
}	

/////////////////
// SPI AD9835  //
/////////////////
void spi_send_word(unsigned int sbyte)
{
    unsigned int t1, x = 32768;
    DDSPORT &= ~(1 << FSYNC); //FSYNC lo
    for(t1 = 0; t1 < 16; t1++)
    {
        if(sbyte & x)
        {
            DDSPORT |= (1 << SDATA);  //SDATA Bit PB1 set
        }
        else
        {
            DDSPORT &= ~(1 << SDATA);  //SDATA Bit PB1 erase
        }
        
        DDSPORT |= (1 << SCLK);  //SCLK hi
        DDSPORT &= ~(1 << SCLK); //SCLK lo
        
        x >>= 1;
    }
    DDSPORT |= (1 << FSYNC); // FSYNC hi
}

void set_frequency(long fin, int initad9835)
{
	long f = fin - INTERFREQUENCY;
    long fxtal = 50000000;  //fCrystal in MHz
    double fword0;
    long fword1;
    int t1;
    
    unsigned char xbyte[] = {0x33, 0x22, 0x31, 0x20};    
    fword0 = (double) f / fxtal;
    fword1 = (unsigned long) (fword0 * 0xFFFFFFFF);

    if(initad9835)
    {
        //Init, set AD9835 to sleepmode
        spi_send_word(0xF800);
        
    }

    //Send 4 * 16 Bit to DDS
    for(t1 = 0; t1 < 4; t1++)
    {
        spi_send_word((xbyte[t1] << 8) + (((fword1 >> ((3 - t1) * 8))) & 0xFF));
    }

    //End of sequence
    if(initad9835)
    {
        //AD9835 wake up from sleep
        spi_send_word(0xC000);
    }
    else
    {
        //AD9835 freq data update, no full init
        spi_send_word(0x8000);
    }
}

  ///////////////////////////
 //    Key detection      // 
///////////////////////////
//PD0 = 1; PD1 = 2; PD2 = 3 short time key press < 2 secs
//PD0 = 34 PD1 = 5; PD2 = 6 long time key press >= 2 secs
int get_keys(void)
{
	long t0;
	int t1;
	
	for(t1 = 0; t1 < 3; t1++)
	{
		if(!(PIND & (1 << t1)))
		{
			t0 = runseconds10;
			while(!(PIND & (1 << t1)))
			{
				oled_putnumber(19 * FONTW, 1, (runseconds10 - t0) / 10, -1, 0, 0);
			}
			if((runseconds10 - t0) < 20)
			{	
			    return t1 + 1;
			}
			else    
			{	
			    return t1 + 4;
			}
		}
	}	
	
	return 0;	
}	

  ///////////////////////////
 //         ISRs          // 
///////////////////////////
ISR(TIMER1_COMPA_vect)
{
    runseconds10++; 
    tuningcount = 0;
}

//Rotary encoder
ISR(PCINT0_vect)
{ 
	int gray = (PINB & 0xC0) >> 6;         //Read PB6 and PB7
    	
    int state = (gray >> 1) ^ gray;        //Convert from Gray code to binary

    if (state != laststate)                //Compare states
    {        
        tuning += ((laststate - state) & 0x03) - 2; // Results in -1 or +1
        laststate = state;
        tuningcount += 2;
    } 
	PCIFR |=  (1 << PCIF0);               //Clear pin change interrupt flag.
}

//Calc increment/decrement rate from tuning speed
int calc_tuningfactor(void)
{
	return (tuningcount * tuningcount); //-1 reverses tuning direction
}	  

int main(void)
{
	//Key
	int k;
	int xit0, xit1;
	
	//VFO
	int sel_vfo;
	int vfo_tmp;
	long f_vfo, f_temp0, f_temp1;
	long runsecs10c = 0;
	
	//Voltage check
	double v1;
	int adc_v;
	long runsecs10a = 0;
		
	//INPUT and TWI pull up
	PORTB = (1 << PB6)| (1 << PB7); //Pullup Rs for rotary encoder
    PORTC = 0x30; //I²C-Bus lines: PC4=SDA, PC5=SCL
	PORTD = 0x07; //Pull-up Rs for Keys 0..2
	
	//Output	             
    DDS_DDR = (1 << FSYNC) | (1 << SDATA) | (1 << SCLK);     //SPI for DDS on PB0..PB2
    	    		
	//Interrupt definitions for rotary encoder PB6 and PB7 (PCINT6 and PCINT7)
	PCMSK0 |= (1<<PCINT6)|(1<<PCINT7);    //enable encoder pins as interrupt source
	PCICR |= (1<<PCIE0);                      // enable pin change interupts 
	
	//Timer 1 as counter for 10th seconds
    TCCR1A = 0;             // normal mode, no PWM
    TCCR1B = (1 << CS10) | (1 << CS12) | (1<<WGM12);   // Prescaler = 1/1024 based on system clock 16 MHz
                                                       // 15625 incs/sec
                                                       // and enable reset of counter register
	OCR1AH = (781 >> 8);                             //Load compare values to registers
    OCR1AL = (781 & 0x00FF);
	TIMSK1 |= (1<<OCIE1A);
	
	//ADC config and ADC init
    ADCSRA = (1<<ADPS0) | (1<<ADPS1) | (1<<ADEN); //Prescaler 64 and ADC on
	get_adc(0); //One dummy conversion
	    		
	//Display
	//TWI
	twi_init();
	wait_ms(20);
	//OLED
	oled_init();
	wait_ms(20);
	oled_cls(0);	
	  
    sel_vfo = load_last_vfo();
    if((sel_vfo < 0) || (sel_vfo > 15))
    {
		sel_vfo = 0;
	}	
    show_vfo(sel_vfo);
    f_vfo = load_frequency(sel_vfo);    
    if(!is_mem_freq_ok(f_vfo))
    {
		f_vfo = BAND_CENTER;
	}	
    set_frequency(f_vfo, 1);
    set_frequency(f_vfo, 0);
    show_frequency(f_vfo);
    show_vfo_freq(sel_vfo);
    
    sei();

    //Display current values for 1st time    
    //VOLTAGE
	v1 = (double) get_adc(1) * .171; //5 / 1024 * 4,1327 * 10;
	adc_v = (int) v1;
   	show_voltage(adc_v);
	runsecs10a = runseconds10;
	
	//Current VFO
	show_vfo(sel_vfo);
	
    draw_meter_scale();	
    
    for(;;)
    {
		//TUNING		
		if(tuning > 1)  
		{    
		    f_vfo += calc_tuningfactor();  
		    set_frequency(f_vfo, 0);
		    show_frequency(f_vfo);
			tuning = 0;
		}
		
		if(tuning < -1)
		{
		    f_vfo -= calc_tuningfactor();  
		    set_frequency(f_vfo, 0);
		    show_frequency(f_vfo);
			tuning = 0;
		}
		
		if(runseconds10 > runsecs10a + 50)
		{
        	//CHECK VOLTAGE
		    v1 = (double) get_adc(1) * .171; //5 / 1024 * 12,57 / 3,589 * 10;
		    adc_v = (int) v1;
   		    show_voltage(adc_v);
	        runsecs10a = runseconds10;
	    }	
		
		if(runseconds10 > runsecs10b + 10)
		{
			show_meter(-1);
			sv_old = 0;
	        runsecs10b = runseconds10;
	    }	
	    
	    show_meter(((get_adc(3) << 1) + get_adc(3)) >> 1); //RX

        k = get_keys();
        if(k > 0)	    	    
        {
			while(get_keys());
			switch(k)
			{
				case 1: vfo_tmp = sel_vfo;
				        f_temp0 = f_vfo;
				        
				        k = get_keys();
				        xit0 = 0;
				        
				        while(!xit0)
				        {
							//Increase VFO index by one
				            if(vfo_tmp < MAXVFO) //Next VFO
				            {
							    vfo_tmp++;
						    }
						    else
						    {
							    vfo_tmp = 0;
						    }	
						    
						    //Load new frequency
						    f_temp1 = load_frequency(vfo_tmp);
						    
						    //Set frequency if OK
						    if(is_mem_freq_ok(f_temp1))	
						    {
				                show_vfo(vfo_tmp);
				                show_vfo_freq(vfo_tmp);
				                set_frequency(f_temp1, 0);
                                show_frequency(f_temp1);
                            }    
                            
                            //Wait for 5 seconds or key pressed
                            runsecs10c = runseconds10;
                            xit1 = 0;
                            while((runseconds10 < runsecs10c + 50) && !xit1)
                            {
								show_meter(((get_adc(1) << 1) + get_adc(1)) >> 1); //RX
                                oled_putnumber(19 * FONTW, 1, (runseconds10 - runsecs10c) / 10, -1, 0, 0);
                                k = get_keys();
                                if(k)
                                { 
									oled_putstring(19 * FONTW, 1, "-", 0, 0);						
								}	
								
								switch(k)
								{
									case 1: //Reset VFO indicator
								            sel_vfo = vfo_tmp;
						                    show_vfo(sel_vfo);
				                            show_vfo_freq(sel_vfo);
				                
				                            //Reset frequency
                                            f_vfo = f_temp1;
							                set_frequency(f_vfo, 0);
                                            show_frequency(f_vfo);
                                            xit0 = 1;
                                            xit1 = 1;
                                            break;
                                            
                                    case 2: f_temp0 = load_frequency(vfo_tmp);
                                            if(is_mem_freq_ok(f_temp0))
                                            {
												f_vfo = f_temp0;
												sel_vfo = vfo_tmp;
												show_vfo(sel_vfo);
				                                show_vfo_freq(sel_vfo);
				                                set_frequency(f_vfo, 0);
                                                show_frequency(f_vfo);
                                                store_last_vfo(sel_vfo);
                                                xit0 = 1;
                                                xit1 = 1;
                                            }    
                                            break;
                                            
                                    case 3: runsecs10c = runseconds10;
                                            xit1 = 1;
                                            break;       
                                 }                  
                            }
                        }
                        xit0 = 0;   
                        break; 
				         		
				case 5: store_frequency(f_vfo, sel_vfo);  //Store frequency to selelcted VFO
				        store_last_vfo(sel_vfo);
				        show_vfo(sel_vfo);
				        show_vfo_freq(sel_vfo);
				        set_frequency(f_vfo, 0);
				        break;
			}	
            oled_putstring(19 * FONTW, 1, "-", 0, 0);					
	    }    
    }
     
	return 0;
}
