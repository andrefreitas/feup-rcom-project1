#include "datalink.h"
char* dataLink::currentFrame = 0;
int dataLink::reaminingAttempts = 0;
int dataLink::currentFD = 0;
int dataLink::currentTimeout = 0;

void dataLink::handleTimeout(int signo) {
	if (dataLink::reaminingAttempts > 0) {
		printf("Alarme%d\n", dataLink::reaminingAttempts);
		write(dataLink::currentFD, dataLink::currentFrame, 5);
		alarm(dataLink::currentTimeout);
		dataLink::reaminingAttempts--;
	} else {
		printf("Connection FAILED!\n");
		exit(0);
	}
}
dataLink::dataLink(char *port, int baudRate, unsigned int timeout,
		unsigned int maxAttempts) {

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

void dataLink::readSupervisionFrame(int fd, char *buf) {
	int estado = 0;
	char readC;
	while (estado < 5) {
		read(fd, &readC, 1);
		switch (estado) {
		case 0: {
			if (readC == buf[0])
				estado++;
			break;
		}
		case 1: {
			if (readC == buf[1])
				estado++;
			break;
		}
		case 2: {
			if (readC == buf[2])
				estado++;
			else if (readC == buf[0])
				estado = 1;
			break;
		}
		case 3: {
			if (readC == buf[3])
				estado++;
			else if (readC == buf[0])
				estado = 1;
			break;
		}
		case 4: {
			if (readC == buf[4])
				estado++;
			break;
		}
		}

	}
}

int dataLink::llopen(unsigned int who) {
	char set[5];
	set[0] = FLAG;
	set[1] = ADDRESS_ER;
	set[2] = SET;
	set[3] = ADDRESS_ER ^ SET;
	set[4] = FLAG;

	char ua[5];
	ua[0] = FLAG;
	ua[1] = ADDRESS_ER;
	ua[2] = UA;
	ua[3] = ADDRESS_ER ^ UA;
	ua[4] = FLAG;

	if (who == TRANSMITTER) {

		dataLink::currentFrame = set;
		dataLink::reaminingAttempts = maxAttempts;
		dataLink::currentTimeout = timeout;
		dataLink::currentFD = fd;

		(void) signal(SIGALRM, dataLink::handleTimeout);

		write(fd, set, 5);
		printf("Emissor escreveu\n");
		alarm(timeout);
		readSupervisionFrame(fd, ua);
		alarm(0);
		printf("Emissor recebeu o UA\n");

		return fd;

	} else if (who == RECEIVER) {

		readSupervisionFrame(fd, set);

		printf("Receptor recebeu SET\n");
		write(fd, ua, 5);
		printf("Emissor recebeu o UA\n");
		return fd;
	}
	return -1;
}
int dataLink::llclose(unsigned int who) {

	char ua[5];
	ua[0] = FLAG;
	ua[1] = ADDRESS_RE;
	ua[2] = UA;
	ua[3] = ADDRESS_RE ^ UA;
	ua[4] = FLAG;

	char discT[5]; // disconect from Transmitter
	discT[0] = FLAG;
	discT[1] = ADDRESS_ER;
	discT[2] = DISC;
	discT[3] = ADDRESS_ER ^ DISC;
	discT[4] = FLAG;

	char discR[5]; // disconnect from Receiver
	discR[0] = FLAG;
	discR[1] = ADDRESS_RE;
	discR[2] = DISC;
	discR[3] = ADDRESS_RE ^ DISC;
	discR[4] = FLAG;

	dataLink::currentTimeout = timeout;
	dataLink::currentFD = fd;
	(void) signal(SIGALRM, dataLink::handleTimeout);

	if (who == TRANSMITTER) {

		write(fd, discT, 5);
		printf("Escreveu discT\n");
		dataLink::currentFrame = discT;
		dataLink::reaminingAttempts = maxAttempts;
		alarm(timeout);
		readSupervisionFrame(fd, discR);
		alarm(0);
		printf("Leu discR\n");
		write(fd, ua, 5);
		printf("Escreveu UA\n");
		return fd;
	}

	if (who == RECEIVER) {
		dataLink::currentFrame = discT;
		dataLink::reaminingAttempts = maxAttempts;

		readSupervisionFrame(fd, discT);
		printf("Leu discT\n");
		write(fd, discR, 5);
		printf("Escreveu discR\n");
		alarm(timeout);
		readSupervisionFrame(fd, ua);
		alarm(0);
		printf("Leu UA\n");
		return fd;
	}

	return -1;

}
dataLink::~dataLink() {
	restoreSerialPort();
}
