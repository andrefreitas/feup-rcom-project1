#include <iostream>
#include "datalink.h"
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

using namespace std;

int main(int argc, char* argv[]) {
	if (argc == 2) {
		if (strcmp(argv[1], "transmitter") == 0) {
			dataLink d((char*) MODEMDEVICE, BAUDRATE, 3, 3);
			printf("=== OPEN ===\n");
			d.llopen(TRANSMITTER);
			printf("\n=== DATA ===\n");
			d.llwrite((char *)"ABCD",4);
			d.llwrite((char*) "E", 1);
			printf("\n=== CLOSE ===\n");
			d.llclose(TRANSMITTER);
			//d.llwrite((char *)"ABCD",4);
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
