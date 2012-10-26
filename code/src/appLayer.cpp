#include "appLayer.h"

appLayer::appLayer(char* filePath) {
	this->filePath = new char[MAX_SIZE];
	strcpy(this->filePath,filePath);
}

int appLayer::sendFile() {
	FILE* pFile;
	char* startPackage = new char[HALF_SIZE];

	pFile = fopen(filePath,"r");
	if (pFile == NULL)
		return -1;
	fseek(pFile,0,SEEK_END);
	int fileSize = ftell(pFile);
	rewind(pFile);

	//buildControlPackage(filePath,startPackage,fileSize,0x01);
	unsigned char *dataPackage=new unsigned char [512];
	int pLen=buildDataPackage(dataPackage,(char*)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",256);
	for (int i=0; i<pLen; i++){
		printf("%x " , dataPackage[i]);
	}
	cout << endl;
	return 0;
}

int appLayer::buildControlPackage(char* filePath, char* package, int fileSize,char control) {
	char fileSizestr[MAX_SIZE];
	int fileSizeLen,packageLen;
	package[0] = control;

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

	return packageLen;
}

int appLayer::buildDataPackage(unsigned char* package,char* data, int index){
	char sequenceNumber=index%255;
	int dataLen=strlen(data);
	if(dataLen>65535) // Limit exceeded
		return -1;

	package[0]=0x00;
	package[1]=sequenceNumber;

	// l2 and l1
	if(dataLen>0xff){
		package[2]=dataLen>>8;
		package[3]=dataLen & 0x00FF;
	}
	else {
		package[2]=0x00;
		package[3]=dataLen;
	}

	// fill with the data
	for(int i=4;i<(dataLen+4); i++){
		package[i]=data[i-4];
	}
	return (4+dataLen);
}
