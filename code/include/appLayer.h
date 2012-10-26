#include "datalink.h"


class appLayer {
	char* filePath;
	void buildStartPackage(char* filePath, char* package, int fileSize);

public:
	appLayer(char* filePath);
	int sendFile();
	int receiveFile();

};
