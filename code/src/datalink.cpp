#include "datalink.h"
char* dataLink::currentFrame=0;
int dataLink::reaminingAttempts=0;
int dataLink::currentFD=0;
int dataLink::currentTimeout=0;

void dataLink::handleTimeout(int signo){
	if(dataLink::reaminingAttempts>0){
		printf("Alarme%d\n",dataLink::reaminingAttempts);
		write(dataLink::currentFD,dataLink::currentFrame,5);
		alarm(dataLink::currentTimeout);
		dataLink::reaminingAttempts--;
	}
	else exit(0);
}
dataLink::dataLink(char *port, int baudRate, unsigned int timeout,	unsigned int maxAttempts) {

	strcpy(this->port, port);
	this->baudRate = baudRate;
	this->timeout = timeout;
	this->maxAttempts = maxAttempts;
	setupSerialPort();
}

void dataLink::setupSerialPort() {
	fd = open(port, O_RDWR | O_NOCTTY);
	tcgetattr(fd, &oldtio);
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = baudRate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0; /* set input mode (non-canonical, no echo,...) */
	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 5; /* blocking read until 5 chars received */

	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
}

void dataLink::restoreSerialPort() {
	sleep(3);
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
}

int dataLink::llopen(unsigned int who) {

	if (who == TRANSMITTER) {
		int estado = 0;
		char readC;
		frame[0] = FLAG;
		frame[1] = ADDRESSE;
		frame[2] = CONTROLE;
		frame[3] = BBCE;
		frame[4] = FLAG;
		// -->
		dataLink::currentFrame=frame;
		dataLink::reaminingAttempts=maxAttempts;
		dataLink::currentTimeout=timeout;
		dataLink::currentFD=fd;
		(void) signal(SIGALRM,dataLink::handleTimeout);
		// -->
		write(fd, frame, 5);
		printf("Emissor escreveu\n");
		alarm(timeout);
		while (estado < 5) {
			read(fd, &readC, 1);
			switch (estado) {
			case 0: {
				if (readC == FLAG)
					estado++;
				break;
			}
			case 1: {
				if (readC == ADDRESSR)
					estado++;
				break;
			}
			case 2: {
				if (readC == CONTROLR)
					estado++;
				else if (readC == FLAG)
					estado = 1;
				break;
			}
			case 3: {
				if (readC == BBCR)
					estado++;
				else if (readC == FLAG)
					estado = 1;
				break;
			}
			case 4: {
				if (readC == FLAG)
					estado++;
				break;
			}
			}

		}
		// Todo: imediatamente apos receber, tem de desativar o alarme
		alarm(0);
		printf("Emissor recebeu o UA\n");
		return fd;
	} else if (who == RECEIVER) {
		int estado = 0;
		char readC;
		while (estado < 5) {
			read(fd, &readC, 1);
			switch (estado) {
			case 0: {
				if (readC == FLAG)
					estado++;
				break;
			}
			case 1: {
				if (readC == ADDRESSE)
					estado++;
				break;
			}
			case 2: {
				if (readC == CONTROLE)
					estado++;
				else if (readC == FLAG)
					estado = 1;
				break;
			}
			case 3: {
				if (readC == BBCE)
					estado++;
				else if (readC == FLAG)
					estado = 1;
				break;
			}
			case 4: {
				if (readC == FLAG)
					estado++;
				break;
			}
			}

		}
		printf("Receptor recebeu SET\n");
		frame[0] = FLAG;
		frame[1] = ADDRESSR;
		frame[2] = CONTROLR;
		frame[3] = BBCR;
		frame[4] = FLAG;
		write(fd, frame, 5);
		printf("Emissor recebeu o UA\n");
		return fd;
	}
	return -1;
}

dataLink::~dataLink(){
	restoreSerialPort();
}
