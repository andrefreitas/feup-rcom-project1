#include <iostream>
#include "datalink.h"
#include "appLayer.h"
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

using namespace std;
/*
int main() {
	appLayer app((char*)"/usr/users2/mieic2010/ei10036/Desktop/pinguim.gif");
	cout << app.sendFile();
	return 0;
}

*/
int main(int argc, char* argv[]) {

	if (argc == 2) {
		if (strcmp(argv[1], "transmitter") == 0) {
			appLayer app((char*)"/home/ubfeup/Desktop/pinguim.gif");
			app.sendFile();
		}
		else if (strcmp(argv[1], "receiver") == 0) {
			dataLink d((char*) MODEMDEVICE, BAUDRATE, 3, 3);
			printf("=== OPEN ===\n");
			d.llopen(RECEIVER);
			//d.llclose(RECEIVER);
			printf("\n=== DATA ===\n");
			char * buf= new char[20];
			while(d.llread(buf)) {
				bzero(buf,20);
			}
			cout << endl;
			printf("\n=== CLOSE ===\n");
			d.llclose(RECEIVER);
			//cout << "\nRecebi: " << buf << endl;
		}
		else
			cout << "Uso: \"nserial receiver\" or \"nserial transmitter \" " << endl;
	}
	else
		cout << "Uso: \"nserial receiver\" or \"nserial transmitter \" " << endl;
	return 0;
}

// TESTE DOS PACOTES REPETIDOS
/*
int main(int argc, char* argv[]) {
	if (argc == 2) {
		if (strcmp(argv[1], "transmitter") == 0) {
			dataLink d((char*) MODEMDEVICE, BAUDRATE, 3, 3);
			d.llwrite((char *)"ABCD",4);
			sleep(4); // Fazer Ctrl+C e voltar a reiniciar o transmitter
			d.llwrite((char*) "E", 1);
			printf("\n=== CLOSE ===\n");
			d.llclose(TRANSMITTER);
			//d.llwrite((char *)"ABCD",4);
		}
		else if (strcmp(argv[1], "receiver") == 0) {
			dataLink d((char*) MODEMDEVICE, BAUDRATE, 3, 3);
			char * buf= new char[20];
			while(d.llread(buf)) {
				bzero(buf,20);
			}
			cout << endl;
			printf("\n=== CLOSE ===\n");
			d.llclose(RECEIVER);
			//cout << "\nRecebi: " << buf << endl;
		}
		else
			cout << "Uso: \"nserial receiver\" or \"nserial transmitter \" " << endl;
	}
	else
		cout << "Uso: \"nserial receiver\" or \"nserial transmitter \" " << endl;
	return 0;
}

*/


// TESTS STUFFING
/*
int main(int argc, char* argv[]) {
	char* frame = new char[20];
	char* deFrame = new char[20];
	frame[0] = FLAG;
	frame[1] = ADDRESS_ER;
	frame[2] = 0x02;
	frame[3] = frame[1] ^ frame[2];
	frame[4] = 0x3D;
	frame[5] = 0x3D;
	frame[6] = 0x3D;
	frame[7] = 0x3D;
	frame[8] = FLAG;

	char* newFrame = new char[40];
	int len = dataLink::stuffFrame(frame,9,newFrame);

	for(int i = 0; i < len; i++) {
		printf("%x ", newFrame[i]);
	}
	cout << endl;
	int len2 = dataLink::deStuffFrame(newFrame,len,deFrame);

	for(int i = 0; i < len2; i++) {
		printf("%x ", deFrame[i]);
	}

	return 0;
}
*/
