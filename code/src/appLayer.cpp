#include "appLayer.h"

appLayer::appLayer(char* filePath) {
	this->filePath = new char[MAX_SIZE];
	strcpy(this->filePath,filePath);
}

int appLayer::sendFile() {
	FILE* pFile;
	char* package = new char[HALF_SIZE];

	pFile = fopen(filePath,"r");

	if (pFile == NULL)
		return -1;
	fseek(pFile,0,SEEK_END);
	int fileSize = ftell(pFile);
	rewind(pFile);

	buildStartPackage(filePath,package,fileSize);

	return 0;
}

void appLayer::buildStartPackage(char* filePath, char* package, int fileSize) {
	char fileSizestr[MAX_SIZE];
	int fileSizelen;
	package[0] = 0x01; // Start Package
	package[1] = 0x00; // Size of the file
	fileSizelen = sprintf (fileSizestr, "%d", fileSize);
	package[2] = fileSizelen;
	for(int i = 3; i < (fileSizelen+3); i++) {
		package[i] = fileSizestr[i-3];
	}

	for(int i = 0; i < 8 ; i++) {
		printf("%x ", package[i]);
	}
	cout << endl;
}
