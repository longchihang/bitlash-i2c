

#include <Wire.h>

#include <bitlash.h>
//#include "C:\Program Files (x86)\Arduino\libraries\bitlash\bitlash.h"
//#include "C:\Program Files (x86)\Arduino\libraries\bitlash\src\bitlash.h"
#include "src/bitlash.h"  // for sp() and printInteger()



byte ishex(char);

#define EEPROM_I2C_ADDRESS 80
uint8_t i2c_id = EEPROM_I2C_ADDRESS;

//24C01    -> 1024 bit    -> 128 byte
//24C02    -> 2048 bit    -> 256 byte
//24C04    -> 4096 bit    -> 512 byte
//24C08    -> 8192 bit    -> 1024 byte
//24C16    -> 16384 bit   -> 2048 byte
//24C32    -> 32768 bit   -> 4096 byte
//24C64    -> 65536 bit   -> 8192 byte
//24C128   -> 131072 bit  -> 16384 byte
//24C256   -> 262144 bit  -> 32768 byte
//24C512   -> 524288 bit  -> 65536 byte
/*
#define MAX_ADDRESS_24C01    128-1
#define MAX_ADDRESS_24C02    256-1 
#define MAX_ADDRESS_24C04    512-1
#define MAX_ADDRESS_24C08   1024-1
#define MAX_ADDRESS_24C16   2048-1
#define MAX_ADDRESS_24C32   4096-1
#define MAX_ADDRESS_24C64   8192-1
#define MAX_ADDRESS_24C128 16384-1
#define MAX_ADDRESS_24C256 32768-1
#define MAX_ADDRESS_24C512 65536-1
*/

#define I2C_24C_DEFAULT 2

uint16_t i2c_24c_max_address(uint16_t);

uint16_t max_address = i2c_24c_max_address(I2C_24C_DEFAULT); // i2c_24c * 128 - 1;//MAX_ADDRESS_24C02;

#define HOW_MANY_VALUES_A_ROW 16


uint16_t i2c_24c_max_address(uint16_t n){
	switch (n)
	{
		case 1:
		case 2:
		case 4:
		case 8:
		case 16:
		case 32:
		case 64:
		case 128:
		case 256:
			return n * 128 - 1;
			break;
		case 512:
			return UINT16_MAX;
			break;
		default:
			break;
	}
	return 0; 
}


numvar func_i2c_24c(void) {
	static uint16_t i2c_24c = I2C_24C_DEFAULT;
	
	uint16_t n = i2c_24c;
	uint16_t m = max_address;
	
	if (getarg(0))
		n = getarg(1);

	m = i2c_24c_max_address(n);

	if(m > 0){
		i2c_24c = n;
		max_address = m;
	} else {
		sp("Invalid 24C model, please input 1, 2, 4, 8, 16, 32, 64, 128 or 256.");
		speol();
	}
	
	return i2c_24c;
}

numvar func_i2c_id(void) {
	if (getarg(0)) 
		i2c_id = getarg(1);
	//else
	//  sp("0x");printHex(i2c_id);speol();
	return i2c_id; 
}


numvar func_i2c_write(void){
	uint16_t startaddress=0;
	uint16_t endaddress=max_address;
	//uint16_t len=max_address+1;
	uint16_t address;
	uint8_t c;
	uint16_t counter;
	if(getarg(0)>1){
		counter = getarg(0);
		c = getarg(1);
		if(c >= 0){
			startaddress = c;
			if (startaddress < 0 || startaddress > endaddress)
				startaddress = 0;

			//len = counter - 1;
			//if(len < endaddress - startaddress + 1)
			//	endaddress = startaddress + len - 1;
			
			int i=2;
			if (counter>1) {
				address=startaddress;
				if ( isstringarg(i) ) { // getarg(1) is a string, e.g. i2c_write(0x00,"f3 12", "aabbcc"...)
					char * line;
					char h1, h2;
					int j;
					
					do {
						if ( !isstringarg(i) ) break; // below arguments must be a string
						line = (char *) getstringarg(i);
						j=0;
						/*
						sp("line:");
						sp(line);
						speol();
						*/
						while(1){
							do {
								h1 = *(line + j++);
								/*
								sp("h1:");
								printInteger(h1, 0, ' ');
								spb(h1);
								speol();
								*/
							} while(h1=='\t' || h1==' '); //skip spaces
							if(h1=='\0') break;
							if(!ishex(h1)) break;
							
							h2 = *(line + j++);
							/*
							sp("h2:");
							printInteger(h2, 0, ' ');
							spb(h2);
							speol();
							*/
							if(!ishex(h2)) break;
							/*
							sp("h1,h2:");
							printInteger(h1, 0, ' ');
							spb(',');
							printInteger(h2, 0, ' ');
							speol();
							*/
							
							c = (uint8_t)hexval(h1);
							c <<= 4;
							c |= (uint8_t)hexval(h2);
							/*
							sp("c:");
							printInteger(c, 0, ' ');
							speol();
							*/
							
							writeI2CByte(i2c_id, address, c );
							delay(50);
							address++;
						}
						i++;
					} while(i<=counter);
				}
				else { // getarg(2) is not a string, e.g. i2c_write(0x00,125,155...) 
					for (address=startaddress; address<=endaddress && i<=counter; address++, i++) {
						c = (uint8_t) getarg(i);
						
						//writeEEPROM(i2c_id, address, c );
						writeI2CByte(i2c_id, address, c );
						delay(50);
					}
				}
				return 0;
			}
		}
	}

	sp("Invalid from address or no data.");
	speol();
	return 0;

}

numvar func_i2c_read(void){
	uint16_t startaddress=0;
	uint16_t endaddress=max_address;
	uint32_t len=max_address+1;
	uint16_t address;
	uint8_t c;
	if(!getarg(0)){
		sp("Start to read all ...");
		speol();
		//startaddress = 0;
		
	} else if(getarg(0)==1){
		c = getarg(1);
		if(c >= 0){
			startaddress = c;
			if (startaddress < 0 || startaddress > endaddress)
				startaddress = 0;
		
			sp("Start to read from 0x");
			//printHex(startaddress);
			printIntegerInBase(startaddress, 16, 4, '0');
			sp(" ...");
			speol();
		}
		else{
			sp("Invalid from address.");
			speol();
		}
	} else if(getarg(0)==2){
		c = getarg(1);
		len = getarg(2);
		if(c >= 0 && len > 0){
			startaddress = c;
			if(startaddress < 0 || startaddress > endaddress)
				startaddress = 0;
				
			if(len < endaddress - startaddress + 1)
				endaddress = startaddress + len - 1;
	
			sp("Start to read from 0x");
			//printHex(startaddress);
			printIntegerInBase(startaddress, 16, 4, '0');
			
			sp(" and len is ");
			printInteger(len, 0, ' ');
			sp(" ...");
			speol();
		}
		else{
			sp("Invalid from address or len.");
			speol();
			return 0;
		}
	}
	else{
		return 0;
	}

	// printer header
	sp("      ");
	for (c=0; c< HOW_MANY_VALUES_A_ROW; c++){
		// printHex(c);
		printIntegerInBase(c, 16, 2, '0');
		spb(' ');
	}
	speol();
	sp("      ");
	for (c=0; c< HOW_MANY_VALUES_A_ROW; c++){
		sp("---");
	}
	// speol();
	
	if ( startaddress % HOW_MANY_VALUES_A_ROW != 0) {
		speol();
		//Serial.print(( startaddress / HOW_MANY_VALUES_A_ROW ) * 8, HEX);
		address=( startaddress / HOW_MANY_VALUES_A_ROW ) * HOW_MANY_VALUES_A_ROW;
		// printHex(address);
		printIntegerInBase(address, 16, 4, '0');
		sp(": ");
	
		for (c=0; c<(startaddress % HOW_MANY_VALUES_A_ROW); c++){
			sp("   ");
		}
	}
	
	for (address=startaddress ; address<=endaddress ; address++) {
		if (address % HOW_MANY_VALUES_A_ROW == 0 ) {
			speol();
			//Serial.print(address, HEX);
			// printHex(address);
			printIntegerInBase(address, 16, 4, '0');
			sp(": ");
		}
		 //Serial.print(readEEPROM(EEPROM_I2C_ADDRESS, address), HEX);
		 //c = readEEPROM(i2c_id, address);
		 c = readI2CByte(i2c_id, address);
		 // printHex(c);
		 printIntegerInBase(c, 16, 2, '0');
		 spb(' ');
		 //if ( (address + 1) % HOW_MANY_VALUES_A_ROW == 0 ) {
		 //  speol();
		 //}
	}
	speol();
 
	return 0;
}

void writeI2CByte(int deviceaddress, uint16_t data_addr, byte data){
	Wire.beginTransmission(deviceaddress);
	if(max_address > UINT8_MAX) Wire.write((byte)(data_addr >> 8)); //writes the MSB
	Wire.write((byte)(data_addr & 0xFF)); //writes the LSB
	//Wire.write(data_addr);
	Wire.write(data);
	Wire.endTransmission();
}

byte readI2CByte(int deviceaddress, uint16_t data_addr){
	byte data = NULL;
	Wire.beginTransmission(deviceaddress);
	if(max_address > UINT8_MAX) Wire.write((byte)(data_addr >> 8)); //writes the MSB
	Wire.write((byte)(data_addr & 0xFF)); //writes the LSB
	//Wire.write(data_addr);
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress, 1); //retrieve 1 returned byte
	delay(1);
	if(Wire.available()){
		data = Wire.read();
	}
	return data;
}

void setup(void) {
	// initialize bitlash and set primary serial port baud
	// print startup banner and run the startup macro
	initBitlash(57600);
	Wire.begin(); 
	// you can execute commands here to set up initial state
	// bear in mind these execute after the startup macro
	// doCommand("print(1+1)");
	addBitlashFunction("i2c_id", (bitlash_function) func_i2c_id);
	addBitlashFunction("i2c_24c", (bitlash_function) func_i2c_24c);
	addBitlashFunction("i2c_read", (bitlash_function) func_i2c_read);
	addBitlashFunction("i2c_write", (bitlash_function) func_i2c_write);
	//e.g. 
	// i2c_read()                         // start read from 0x00, read all data by model.
	// i2c_read(0x00, 16)                 // start read from 0x00, len 16.
	// i2c_write(0x00, 0x01, 0x02)        // start write from 0x00, data 0x01 and then 0x02.
	//   or i2c_write(0x00, "01 02")
	//   or i2c_write(0x00, "0102") 
	// i2c_write(0x00, "01 02", "03 04")  // start write from 0x00, data 0x01 and then 0x02, 0x03, 0x04.
	// print i2c_id()                     // show i2c-id.
	// i2c_id(0x52)                       // set i2c-id to 0x52.
	// print i2c_24c()                    // show i2c-24c model, 2 means 24C02.
	// i2c_24c(2)                          // set i2c-24c model to 24C02.

}

void loop(void) {
	runBitlash();
}

/*
			00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 
			------------------------------------------------
0000: F6 08 08 0E 0A 61 40 00 05 25 40 00 82 08 00 00 
0010: 0C 08 38 01 02 00 03 3D 50 50 60 32 1E 32 2D 01 
0020: 18 25 05 13 3C 1E 1E 00 36 39 7F 80 14 1E 00 00 
0030: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12 71 
0040: 7F 7F 7F 7F CB 00 00 00 00 20 44 51 56 45 31 42 
0050: 31 36 20 20 20 20 20 20 20 20 20 00 00 08 28 00 
0060: 00 00 00 52 4D 54 2D 38 37 30 30 39 33 00 00 00 
0070: 00 55 32 36 48 33 4A 34 31 5A 31 00 00 00 00 00 
0080: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0090: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00A0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00B0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00C0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00D0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00E0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00F0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
*/

// http://vascoferraz.com/projects/24c-eeprom-check/
// https://www.hackster.io/techmirtz/reading-and-writing-data-to-external-eeprom-using-arduino-670dd8
// https://www.norwegiancreations.com/2018/02/creating-a-command-line-interface-in-arduinos-serial-monitor/
// https://medium.com/@kslooi/print-formatted-data-in-arduino-serial-aaea9ca840e3

// https://create.arduino.cc/projecthub/gatoninja236/how-to-use-i2c-eeprom-30767d 
// readI2CByte() writeI2CByte()



