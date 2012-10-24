#include "datalink.h"
char* dataLink::currentFrame = 0;
int dataLink::reaminingAttempts = 0;
int dataLink::currentFrameLength = 0;
int dataLink::currentFD = 0;
int dataLink::currentTimeout = 0;

void dataLink::handleTimeout(int signo) {
	if (dataLink::reaminingAttempts > 0) {
		printf("\n\nAlarme. Tentativas restantes: %d\n\n",
				dataLink::reaminingAttempts);
		write(dataLink::currentFD, dataLink::currentFrame,
				dataLink::currentFrameLength);
		alarm(dataLink::currentTimeout);
		dataLink::reaminingAttempts--;
	} else {
		printf("\nTimeout !\n");
		exit(0);
	}
}
dataLink::dataLink(char *port, int baudRate, unsigned int timeout,
		unsigned int maxAttempts) {

	strcpy(this->port, port);
	this->baudRate = baudRate;
	this->timeout = timeout;
	this->maxAttempts = maxAttempts;
	sequenceNumber = 0;
	setupSerialPort();
}

dataLink::~dataLink() {
	restoreSerialPort();
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

	(void) signal(SIGALRM, dataLink::handleTimeout);
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
			else if (readC != buf[0])
				estado = 0;
			break;
		}
		case 2: {
			if (readC == buf[2])
				estado++;
			else if (readC == buf[0])
				estado = 1;
			else
				estado = 0;
			break;
		}
		case 3: {
			if (readC == buf[3])
				estado++;
			else if (readC == buf[0])
				estado = 1;
			else
				estado = 0;
			break;
		}
		case 4: {
			if (readC == buf[4])
				estado++;
			else if (readC == buf[0])
				estado = 1;
			else
				estado = 0;
			break;
		}
		}

	}
}

bool dataLink::isReceiverReady(int fd, char* rr, char* rej) {
	int estado = 0;
	char readC;
	while (1) {
		read(fd, &readC, 1);
		printf("%x:st%d ", readC, estado);
		switch (estado) {
		case 0: {
			if (readC == rr[0]) // FLAG
				estado = 1;
			break;
		}
		case 1: {
			if (readC == rr[1]) // ADDRESS
				estado = 2;
			else if (readC != rr[0])
				estado = 0;
			break;
		}
		case 2: {
			if (readC == rr[2]) // RR
				estado = 3;
			else if (readC == rej[2]) // REJ
				estado = 4;
			else if (readC == rr[0])
				estado = 1;
			else
				estado = 0;
			break;
		}
		case 3: {
			if (readC == rr[3])
				estado = 5;
			else if (readC == rr[0])
				estado = 1;
			else
				estado = 0;
			break;
		}
		case 4: {
			if (readC == rej[3])
				estado = 6;
			else if (readC == rej[0])
				estado = 1;
			else
				estado = 0;
			break;
		}
		case 5: {
			if (readC == rr[4]) {
				cout << endl;
				return true;
			} else
				estado = 0;
			break;
		}
		case 6: {
			if (readC == rej[4]) {
				cout << endl;
				return false;
			} else
				estado = 0;
			break;
		}

		}

	}
	return false;
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
		dataLink::currentFrameLength = 5;

		write(fd, set, 5);
		printf("-> Enviou SET\n");
		alarm(timeout);
		readSupervisionFrame(fd, ua);
		alarm(0);
		printf("<- Recebeu UA\n");

		return fd;

	} else if (who == RECEIVER) {

		readSupervisionFrame(fd, set);

		printf("<- Recebeu SET\n");
		write(fd, ua, 5);
		printf("-> Enviou UA\n");
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
	dataLink::currentFrameLength = 5;

	if (who == TRANSMITTER) {

		write(fd, discT, 5);
		printf("-> Escreveu discT\n");
		dataLink::currentFrame = discT;
		dataLink::reaminingAttempts = maxAttempts;
		alarm(timeout);
		readSupervisionFrame(fd, discR);
		alarm(0);
		printf("<- Recebeu discR\n");
		write(fd, ua, 5);
		printf("-> Enviou UA\n");
		return fd;
	}

	if (who == RECEIVER) {

		write(fd, discR, 5);
		printf("-> Enviou discR\n");
		alarm(timeout);
		readSupervisionFrame(fd, ua);
		alarm(0);
		printf("<- Recebeu UA\n");
		return fd;
	}

	return -1;

}

int dataLink::llwrite(char *buf, int unsigned length) {
	// Data Frame
	char frame[HALF_SIZE];
	char* stuffedFrame = new char[MAX_SIZE];
	frame[0] = FLAG;
	frame[1] = ADDRESS_ER;
	frame[2] = 0x02 * sequenceNumber;
	frame[3] = frame[1] ^ frame[2];
	char bcc2 = 0;
	for (int unsigned i = 0; i < length; i++) {
		frame[4 + i] = buf[i];
		bcc2 = bcc2 ^ buf[i];
	}
	frame[4 + length] = bcc2;
	frame[4 + length + 1] = FLAG;
	dataLink::currentFrameLength = stuffFrame(frame, 6+length, stuffedFrame);
	// Receiver Ready
	char rr[5], rej[5];
	buildREJRR(sequenceNumber, rej, rr);
	int retries = 0;
	do {
		alarm(0);

		dataLink::currentTimeout = timeout;
		dataLink::currentFD = fd;
		dataLink::currentFrame = stuffedFrame;
		dataLink::reaminingAttempts = maxAttempts;

		write(fd, stuffedFrame, dataLink::currentFrameLength);
		if (retries > 0)
			cout << "*";
		printf("%d -> ", sequenceNumber);
		alarm(timeout);
		retries++;

	} while (!isReceiverReady(fd, rr, rej));

	alarm(0);
	sequenceNumber = !sequenceNumber;
	return length;
}

int dataLink::readInformationFrame(int fd, char *buf) {
	int estado = 0;
	char readC;
	char C;
	int counter;

	while (1) {
		read(fd, &readC, 1);
		// Debug
		printf("%x:st%d ", readC, estado);
		switch (estado) {
		case 0:
			if (readC == FLAG) {
				estado++;
				counter = 0; // reset counter
				buf[counter] = readC;
			}
			break;
		case 1:
			if (readC == ADDRESS_ER) {
				estado++;
				counter = 1;
				buf[counter] = readC;
			} else if (readC != FLAG)
				estado = 0;
			break;
		case 2:
			if (readC == 0x02 || readC == 0x00) {
				C = readC;
				estado++;
				counter = 2;
				buf[counter] = readC;
			} else if (readC == FLAG)
				estado = 1;
			else if (readC == DISC) {
				estado = 7;
			} else
				estado = 0;
			break;

		case 3:
			if (readC == (ADDRESS_ER ^ C)) {
				estado++;
				counter = 3;
				buf[counter] = readC;
			} else if (readC == FLAG)
				estado = 1;
			else
				estado = 0;
			break;
		case 4:
			if (readC == FLAG)
				estado = 1;
			else {
				estado++;
				counter++;
				buf[counter] = readC;
			}
			break;
		case 5:
			if (readC == FLAG)
				estado = 1;
			else {
				estado++;
				counter++;
				buf[counter] = readC;
			}
			break;
		case 6:
			counter++;
			if (readC == FLAG) {
				buf[counter] = readC;
				return counter + 1;
			} else {
				buf[counter] = readC;
			}
			break;
		case 7:
			if (readC == (ADDRESS_ER ^ DISC))
				estado = 8;
			else if (readC == FLAG)
				estado = 1;
			else
				estado = 0;
			break;
		case 8:
			if (readC == FLAG) {
				cout << " DISC";
				return -1;
			} else
				estado = 0;
			break;
		}

	}

}
int dataLink::parseSequenceNumber(char *frame) {
	if (frame[2] == 0x02)
		return 1;
	else if (frame[2] == 0x00)
		return 0;
	return -1;
}

void dataLink::buildREJRR(int sequenceNumber, char *rej, char *rr) {
	if (rr != 0) {
		// Receiver Ready
		rr[0] = FLAG;
		rr[1] = ADDRESS_ER;
		if (sequenceNumber == 0)
			rr[2] = RR1;
		else
			rr[2] = RR0;
		rr[3] = rr[1] ^ rr[2];
		rr[4] = FLAG;

	}

	if (rej != 0) {
		// Reject
		rej[0] = FLAG;
		rej[1] = ADDRESS_ER;
		if (sequenceNumber == 0)
			rej[2] = REJ0;
		else
			rej[2] = REJ1;
		rej[3] = rej[1] ^ rej[2];
		rej[4] = FLAG;
	}
}

bool dataLink::rejectFrame(char *frame, int frameLen) {

	int unsigned dataLen = frameLen - 6;
	if (dataLen == 0)
		return true;

	// Check BCC1
	if ((frame[1] ^ frame[2]) != frame[3]) {
		cout << " Erro no BCC1";
		return true;
	}

	// Check BCC2
	char bcc2ToCheck = 0;
	for (int unsigned i = 0; i < dataLen; i++)
		bcc2ToCheck = bcc2ToCheck ^ frame[4 + i];

	if (bcc2ToCheck != frame[4 + dataLen]) {
		printf(" Erro no BCC2- Esperado: %x Calculado: %x", frame[4 + dataLen],
				bcc2ToCheck);
		return true;
	}

	return false;

}
int dataLink::llread(char *buf) {

	while (1) {
		char *frame = new char[HALF_SIZE];
		char* stuffedFrame = new char[MAX_SIZE];
		printf("%d -> ", sequenceNumber);
		int stuffedFrameLen = readInformationFrame(fd, stuffedFrame);
		if (stuffedFrameLen == -1)
			return 0;
		int frameLen = deStuffFrame(stuffedFrame,stuffedFrameLen,frame);
		int unsigned dataLen = frameLen - 6;
		int unsigned sReceived = parseSequenceNumber(frame);
		char rej[5], rr[5];
		buildREJRR(sequenceNumber, rej, rr);

		// If the sequenceNumber is repeated ( need to check later)
		if (sReceived != sequenceNumber) {
			char rrWrongSequence[5];
			buildREJRR(!sequenceNumber, 0, rrWrongSequence);
			write(fd, rrWrongSequence, 5);
		}

		// Check errors
		else {
			if (rejectFrame(frame, frameLen)) {
				write(fd, rej, 5);
				cout << "\n  -> Enviou REJ" << sequenceNumber << endl;
			} else {
				write(fd, rr, 5);
				sequenceNumber = !sequenceNumber;
				for (int unsigned i = 0; i < dataLen; i++) {
					buf[i] = frame[4 + i];
				}
				cout << " \"" << buf << "\"" << endl;
				cout << "  -> Enviou RR" << !sequenceNumber << endl;
				return dataLen;

			}
		}
	}
	return -1;
}

int dataLink::stuffFrame(char* frame, int frameLen, char* newFrame) {
	int i, j;

	for (i = 0, j = 0; i < frameLen; i++, j++) {
		if (i > 3 && i != (frameLen - 1)) {
			if (frame[i] == FLAG) {
				newFrame[j] = 0x7d;
				newFrame[j + 1] = 0x5e;
				j++;
			}
			else if(frame[i] == 0x7d) {
				newFrame[j] = 0x7d;
				newFrame[j + 1] = 0x5d;
				j++;
			}
			else
				newFrame[j] = frame[i]; //For the data bytes till BCC2
		} else {
			newFrame[j] = frame[i]; // For the first 4 bytes and the last one
		}
	}

	return j;
}

int dataLink::deStuffFrame(char* frame, int frameLen, char* newFrame) {
	int i, j;

	for (i = 0, j = 0; i < frameLen; i++, j++) {
		if (i > 3 && i != (frameLen - 1)) {
			if (frame[i] == 0x7d && frame[i+1] == 0x5e) {
				newFrame[j] = 0x7e;
				i++;
			}
			else if(frame[i] == 0x7d && frame[i+1] == 0x5d) {
				newFrame[j] = 0x7d;
				i++;
			}
			else
				newFrame[j] = frame[i]; //For the data bytes till BCC2
		} else {
			newFrame[j] = frame[i]; // For the first 4 bytes and the last one
		}
	}

	return j;
}
