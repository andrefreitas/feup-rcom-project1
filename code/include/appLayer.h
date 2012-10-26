#include "datalink.h"


class appLayer {
	char* filePath;
	int buildStartPackage(char* filePath, char* package, int fileSize);

public:
	appLayer(char* filePath);
	int sendFile();
	int receiveFile();

};
