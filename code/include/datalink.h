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
		char port[20];          /*Dispositivo /dev/ttySx, x = 0, 1*/  
		int baudRate;          /*Velocidade de transmissão*/  
		unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
		unsigned int timeout;         /*Valor do temporizador: 1 s*/  
		unsigned int maxAttempts; /*Número de tentativas em caso de falha*/  
		char frame[MAX_SIZE];         /*Trama*/
		struct termios oldtio,newtio;
		int unsigned fd;
		void setupSerialPort();
		
		
	public:
		dataLink(char *port,int baudRate, unsigned int timeout, unsigned int maxAttempts);
		int llopen(unsigned int who);
		void restoreSerialPort();
}; 

