#include "datalink.h"


class appLayer {
	char* filePath;
	int buildControlPackage(char* filePath, unsigned char* package, int fileSize,unsigned char control);
	int buildDataPackage(unsigned char* package,unsigned char* data, int index,int dataLen);

public:
	appLayer(char* filePath);
	int sendFile();
	int receiveFile();

};
