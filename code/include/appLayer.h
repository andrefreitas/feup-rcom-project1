#include "datalink.h"


class appLayer {
	char* filePath;
	int buildControlPackage(char* filePath, char* package, int fileSize,char control);
	int buildDataPackage(char* package,char* data, int index);

public:
	appLayer(char* filePath);
	int sendFile();
	int receiveFile();

};
