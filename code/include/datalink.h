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
#define ADDRESS_ER 0x03
#define ADDRESS_RE 0x01
#define SET 0x03
#define UA 0x07
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
		void readSupervisionFrame(int fd,char *buf);
		// -->
		
	public:
		dataLink(char *port,int baudRate, unsigned int timeout, unsigned int maxAttempts);
		static void handleTimeout(int signo);
		int llopen(unsigned int who);
		int llclose(unsigned int who);
		static char *currentFrame;
		static int reaminingAttempts;
		static int currentFD;
		static int currentTimeout;
		~dataLink();
}; 

