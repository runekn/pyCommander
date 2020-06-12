/*
  Commander.cpp - Library for interfacing with ArbotiX Commander
  Copyright (c) 2009-2012 Michael E. Ferguson.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <windows.h>
#include <iostream>

/* bitmasks for buttons array */
unsigned char BUT_R1 = 0x01;
unsigned char BUT_R2 = 0x02;
unsigned char BUT_R3 = 0x04;
unsigned char BUT_L4 = 0x08;
unsigned char BUT_L5 = 0x10;
unsigned char BUT_L6 = 0x20;
unsigned char BUT_RT = 0x40;
unsigned char BUT_LT = 0x80;

// joystick values are -125 to 125
signed char walkV;      // vertical stick movement = forward speed
signed char walkH;      // horizontal stick movement = sideways or angular speed
signed char lookV;      // vertical stick movement = tilt    
signed char lookH;      // horizontal stick movement = pan (when we run out of pan, turn body?)
// 0-1023, use in extended mode    
int pan;
int tilt;

// buttons are 0 or 1 (PRESSED), and bitmapped
unsigned char buttons;  // 
unsigned char ext;      // Extended function set

// internal variables used for reading messages
unsigned char vals[7];  // temporary values, moved after we confirm checksum
int read_index = -1;              // -1 = waiting for new packet
int checksum;
unsigned char status = 0; 
HANDLE handler;
COMSTAT comstat;
DWORD errors;

extern "C" {
/* SouthPaw Support */
void UseSouthPaw(){
    status |= 0x01;
}

int serialAvailable() {
	ClearCommError(handler, &errors, &comstat);
	return comstat.cbInQue;
}

uint8_t serialRead() {
	DWORD bytesRead{};
	ClearCommError(handler, &errors, &comstat);
	uint8_t c;
	if (ReadFile(handler, &c, 1, &bytesRead, NULL)) {
		if(bytesRead != 0) {
			return c;
		}
	}
	return -1;
}

bool begin(char* port, unsigned long baud) {
	handler = CreateFileA(static_cast<LPCSTR>(port),
								GENERIC_READ | GENERIC_WRITE,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (handler == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			std::cerr << "ERROR: Handle was not attached.Reason : " << port << " not available\n";
		}
		else {
			std::cerr << "ERROR!!!\n";
		}
		return false;
	}
	else {
		DCB dcbSerialParameters = {0};
		if (!GetCommState(handler, &dcbSerialParameters)) {
			std::cerr << "Failed to get current serial parameters\n";
		}
		else {
			dcbSerialParameters.BaudRate = baud;
			dcbSerialParameters.ByteSize = 8;
			dcbSerialParameters.StopBits = ONESTOPBIT;
			dcbSerialParameters.Parity = NOPARITY;
			dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(handler, &dcbSerialParameters)) {
				std::cout << "ALERT: could not set serial port parameters\n";
			}
			else {
				PurgeComm(handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
				Sleep(1000);
			}
		}
		return true;
	}
}

/* process messages coming from Commander 
 *  format = 0xFF RIGHT_H RIGHT_V LEFT_H LEFT_V BUTTONS EXT CHECKSUM */
int ReadMsgs(){
    while(serialAvailable() > 0){
        if(read_index == -1){         // looking for new packet
            if(serialRead() == 0xff){
                read_index = 0;
                checksum = 0;
            }
        }else if(read_index == 0){
            vals[read_index] = (unsigned char) serialRead();
            if(vals[read_index] != 0xff){            
                checksum += (int) vals[read_index];
                read_index++;
            }
        }else{
            vals[read_index] = (unsigned char) serialRead();
            checksum += (int) vals[read_index];
            read_index++;
            if(read_index == 7){ // packet complete
                if(checksum%256 != 255){
                    // packet error!
                    read_index = -1;
                    return 0;
                }else{
                    if((status&0x01) > 0){     // SouthPaw
                        walkV = (signed char)( (int)vals[0]-128 );
                        walkH = (signed char)( (int)vals[1]-128 );
                        lookV = (signed char)( (int)vals[2]-128 );
                        lookH = (signed char)( (int)vals[3]-128 );
                    }else{
                        lookV = (signed char)( (int)vals[0]-128 );
                        lookH = (signed char)( (int)vals[1]-128 );
                        walkV = (signed char)( (int)vals[2]-128 );
                        walkH = (signed char)( (int)vals[3]-128 );
                    }
                    pan = (vals[0]<<8) + vals[1];
                    tilt = (vals[2]<<8) + vals[3];
                    buttons = vals[4];
                    ext = vals[5];
                }
                read_index = -1;
                return 1;
            }
        }
    }
    return 0;
}
}