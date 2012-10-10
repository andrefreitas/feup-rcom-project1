#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <signal.h>
#include <unistd.h>
#define MAX_SIZE 255
#define TRANSMITTER 100
#define RECEIVER 200
#define FLAG 0x7E
#define ADDRESSE 0x03
#define ADDRESSR 0x01
#define CONTROLE 0x03
#define CONTROLR 0x07
#define BBCE (ADDRESSE^CONTROLE)
#define BBCR (ADDRESSR^CONTROLR)

class dataLink {  
		char port[20];
		int baudRate;
		unsigned int sequenceNumber;
		unsigned int timeout;
		unsigned int maxAttempts;
		char frame[MAX_SIZE];
		struct termios oldtio,newtio;
		int unsigned fd;
		// -->
		void setupSerialPort();
		void restoreSerialPort();
		// -->
		
	public:
		dataLink(char *port,int baudRate, unsigned int timeout, unsigned int maxAttempts);
		static void handleTimeout(int signo);
		int llopen(unsigned int who);
		static char *currentFrame;
		static int reaminingAttempts;
		static int currentFD;
		static int currentTimeout;
		~dataLink();
}; 

