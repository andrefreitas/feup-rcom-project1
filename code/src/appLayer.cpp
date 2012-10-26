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

int appLayer::buildStartPackage(char* filePath, char* package, int fileSize) {
	char fileSizestr[MAX_SIZE];
	int fileSizeLen,packageLen;
	package[0] = 0x01; // Start Package

	// Size of the file
	package[1] = 0x00;
	fileSizeLen = sprintf (fileSizestr, "%d", fileSize);
	package[2] = fileSizeLen;
	for(int i = 3; i < (fileSizeLen+3); i++) {
		package[i] = fileSizestr[i-3];
	}

	// Name of the file
	int filePathLen=strlen(filePath);
	package[3+fileSizeLen]=0x01;
	package[4+fileSizeLen]=filePathLen;
	for(int i = (5+fileSizeLen); i<(filePathLen+5+fileSizeLen);i++){
		package[i]=filePath[i-(5+fileSizeLen)];
	}

	// The package length
	packageLen=5+fileSizeLen+filePathLen;

	for(int i=0; i<packageLen; i++){
		printf("%c:%x ",package[i],package[i]);

	}
	cout << endl;

	return packageLen;

}
