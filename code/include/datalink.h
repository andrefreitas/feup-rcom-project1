#ifndef DATALINK_H
#define DATALINK_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>

using namespace std;

#define HALF_SIZE 4096
#define MAX_SIZE 8192
#define TRANSMITTER 100
#define RECEIVER 200
#define FLAG 0x7E
#define ADDRESS_ER 0x03
#define ADDRESS_RE 0x01
#define SET 0x03
#define UA 0x07
#define DISC 0x0B
#define RR0 0x05
#define RR1 0x25
#define REJ0 0x01
#define REJ1 0x21
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

// Data Link
class dataLink {  
		char port[MAX_SIZE];
		int baudRate;
		unsigned int sequenceNumber;
		unsigned int timeout;
		unsigned int maxAttempts;
		struct termios oldtio,newtio;
		int unsigned fd;
		// -->
		void setupSerialPort();
		void restoreSerialPort();
		void readSupervisionFrame(int fd,unsigned char*buf);
		bool isReceiverReady(int fd, unsigned char *rr, unsigned char *rej);
		int readInformationFrame(int fd, unsigned char *buf);
		int parseSequenceNumber(unsigned char *frame);
		void buildREJRR(int sequenceNumber, unsigned char *rej, unsigned char *rr);
		bool rejectFrame(unsigned char *frame, int frameLen);
		static int stuffFrame(unsigned char* frame, int frameLen, unsigned char* newFrame);
		static int deStuffFrame(unsigned char* frame, int frameLen, unsigned char* newFrame);
		// -->
		
	public:
		dataLink(char *port,int baudRate, unsigned int timeout, unsigned int maxAttempts);
		static void handleTimeout(int signo);
		static unsigned char *currentFrame;
		static int reaminingAttempts;
		static int currentFD;
		static int currentTimeout;
		static int currentFrameLength;
		int llopen(unsigned int who);
		int llclose(unsigned int who);
		int llwrite(unsigned char *buf,int unsigned length);
		int llread(unsigned char *buf);
		~dataLink();
}; 

#endif
