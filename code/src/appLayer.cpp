#include "appLayer.h"
#define READSIZE 120
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

appLayer::appLayer(char* filePath) {
	this->filePath = new char[MAX_SIZE];
	strcpy(this->filePath, filePath);
}

int appLayer::sendFile() {
	FILE* pFile;
	char* package = new char[HALF_SIZE];
	char* buf = new char[HALF_SIZE];
	int bufLen, packageLen, index = 0;

	pFile = fopen(filePath, "r");
	if (pFile == NULL)
		return -1;
	fseek(pFile, 0, SEEK_END);
	int fileSize = ftell(pFile);
	rewind(pFile);
	dataLink d((char*) MODEMDEVICE, BAUDRATE, 3, 3);
	printf("=== OPEN ===\n");
	d.llopen(TRANSMITTER);
	printf("\n=== DATA ===\n");
	packageLen = buildControlPackage(filePath, package, fileSize, 0x01);
	d.llwrite(package, packageLen);
	while (1) {
		bzero(buf, HALF_SIZE);
		bzero(package, HALF_SIZE);
		bufLen = fread(buf, 1, READSIZE, pFile);
		if (bufLen == 0)
			break;
		packageLen = buildDataPackage(package, buf, index);
		d.llwrite(package, packageLen);
		index++;
	}

	bzero(package, HALF_SIZE);
	packageLen = buildControlPackage(filePath, package, fileSize, 0x02);
	//d.llwrite(package, packageLen);

	printf("\n=== CLOSE ===\n");
	d.llclose(TRANSMITTER);
	return 0;
}

int appLayer::buildControlPackage(char* filePath, char* package, int fileSize,
		char control) {
	char fileSizestr[MAX_SIZE];
	int fileSizeLen, packageLen;
	package[0] = control;

	// Size of the file
	package[1] = 0x00;
	fileSizeLen = sprintf(fileSizestr, "%d", fileSize);
	package[2] = fileSizeLen;
	for (int i = 3; i < (fileSizeLen + 3); i++) {
		package[i] = fileSizestr[i - 3];
	}

	// Name of the file
	int filePathLen = strlen(filePath);
	package[3 + fileSizeLen] = 0x01;
	package[4 + fileSizeLen] = filePathLen;
	for (int i = (5 + fileSizeLen); i < (filePathLen + 5 + fileSizeLen); i++) {
		package[i] = filePath[i - (5 + fileSizeLen)];
	}

	// The package length
	packageLen = 5 + fileSizeLen + filePathLen;

	return packageLen;
}

int appLayer::buildDataPackage(char* package, char* data, int index) {
	char sequenceNumber = index % 255;
	int dataLen = strlen(data);
	if (dataLen > 65535) // Limit exceeded
		return -1;

	package[0] = 0x00;
	package[1] = sequenceNumber;

	// l2 and l1
	if (dataLen > 0xff) {
		package[2] = dataLen >> 8;
		package[3] = dataLen & 0x00FF;
	} else {
		package[2] = 0x00;
		package[3] = dataLen;
	}

	// fill with the data
	for (int i = 4; i < (dataLen + 4); i++) {
		package[i] = data[i - 4];
	}
	return (4 + dataLen);
}

int appLayer::receiveFile() {
	dataLink d((char*) MODEMDEVICE, BAUDRATE, 3, 3);
	int bufLen, packageLen, index = 0;
	FILE* pFile;
	char* package = new char[HALF_SIZE];
	char* buf = new char[HALF_SIZE];
	char* data;
	int bufLen;

	pFile = fopen(filePath, "wb");

	printf("=== OPEN ===\n");
	d.llopen(RECEIVER);
	printf("\n=== DATA ===\n");
	d.llread(buf);
	bzero(buf,HALF_SIZE);

	while (d.llread(buf)) {
		bufLen = strlen(buf);
		data = buf+4;
		fwrite(data,1,bufLen-4,pFile);
		bzero(buf,HALF_SIZE);
	}

	//d.llread(buf);
	cout << endl;
	printf("\n=== CLOSE ===\n");
	d.llclose(RECEIVER);
	//cout << "\nRecebi: " << buf << endl;
	return 0;
}
