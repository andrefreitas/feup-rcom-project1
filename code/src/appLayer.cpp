#include "appLayer.h"

appLayer::appLayer() {
	this->filePath = new char[MAX_SIZE];
	timeout = 1;
	baudrate = B38400;
	readSize = 4000;
	maxAttempts = 3;
	errorProb = 5;
}

int appLayer::sendFile() {
	FILE* pFile;
	unsigned char* package = new unsigned char[HALF_SIZE];
	unsigned char* buf = new unsigned char[HALF_SIZE];
	int bufLen, packageLen, index = 0;

	pFile = fopen(filePath, "r");
	if (pFile == NULL)
		return -1;
	fseek(pFile, 0, SEEK_END);
	int fileSize = ftell(pFile);
	rewind(pFile);
	dataLink *d = new dataLink((char*) MODEMDEVICE, baudrate, timeout,
			maxAttempts);

	if (!restoreTransfer) {
		printf("=== OPEN ===\n");
		d->llopen(TRANSMITTER);
	}

	if ((fileSize / readSize) > 255) {
		cout << "\n ATTENTION: restore may fail because -s is too low\n";
		cout << "Do you want to continue?(y/n): ";
		char op;
		cin >> op;
		if (op == 'n' || op == 'N')
			exit(0);
	}

	printf("\n=== DATA ===\n");
	packageLen = buildControlPackage(filePath, package, fileSize, 0x01);
	d->llwrite(package, packageLen);
	while (1) {
		bzero(buf, HALF_SIZE);
		bzero(package, HALF_SIZE);
		bufLen = fread(buf, 1, readSize - 4, pFile);
		if (bufLen == 0)
			break;
		packageLen = buildDataPackage(package, buf, index, bufLen);
		d->llwrite(package, packageLen);
		index++;
	}

	bzero(package, HALF_SIZE);
	packageLen = buildControlPackage(filePath, package, fileSize, 0x02);
	//d.llwrite(package, packageLen);

	printf("\n=== CLOSE ===\n");
	d->llclose(TRANSMITTER);
	d->getStats(stats);
	return 0;
}

int appLayer::buildControlPackage(const char* filePath, unsigned char* package,
		int fileSize, unsigned char control) {
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

	// Name of the file (remove the path if needed)

	int filePathLen = strlen(filePath);
	int fileNamePos = -1;
	for (int i = filePathLen - 1; i > -1; i--) {
		if (filePath[i] == '/') {
			fileNamePos = i + 1;
			break;
		}
	}
	if (fileNamePos != -1) {
		filePath += fileNamePos;
		filePathLen = strlen(filePath);
	}
	//cout << "Name of the file: " << filePath << endl;

	package[3 + fileSizeLen] = 0x01;
	package[4 + fileSizeLen] = filePathLen;
	for (int i = (5 + fileSizeLen); i < (filePathLen + 5 + fileSizeLen); i++) {
		package[i] = filePath[i - (5 + fileSizeLen)];
	}

	// The package length
	packageLen = 5 + fileSizeLen + filePathLen;

	return packageLen;
}

int appLayer::buildDataPackage(unsigned char* package, unsigned char* data,
		int index, int dataLen) {
	unsigned char sequenceNumber = index % 255;

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
	dataLink * d = new dataLink((char*) MODEMDEVICE, baudrate, timeout,
			maxAttempts);
	d->setErrorProb(errorProb);
	int bufLen;
	FILE* pFile;
	unsigned char* buf = new unsigned char[HALF_SIZE];
	unsigned char* data;
	fileSizeReceived = 0;
	string fileName;
	char *temp = new char[MAX_SIZE];

	printf("=== OPEN ===\n");
	d->llopen(RECEIVER);
	printf("\n=== DATA ===\n");

	bufLen = d->llread(buf);
	fileSize = parseFileName(buf, 0, bufLen);

	// Check if user specified the file name
	if (strcmp(filePath, "NONE") == 0) {
		parseFileName(buf, filePath, bufLen);
		pFile = fopen(filePath, "wb");
	} else {
		fileName = filePath;

		if (fileName.at(fileName.length() - 1) == '/') {
			parseFileName(buf, temp, bufLen);
			strcat(filePath, temp);
			//cout << "FileP: " << filePath << endl;
		}
		pFile = fopen(filePath, "wb");
	}
	bzero(buf, HALF_SIZE);

	int sequenceNumber = -1;

	while ((bufLen = d->llread(buf))) {
		bufLen -= 4;
		data = buf + 4;

		// Check sequence number ( to restore file transfer)
		if ((sequenceNumber + 1) == buf[1]) {
			sequenceNumber++;
			fwrite(data, 1, bufLen, pFile);
			fileSizeReceived += bufLen;
		}
		bzero(buf, HALF_SIZE);
	}
	fclose(pFile);

	cout << endl;
	printf("\n=== CLOSE ===\n");
	d->llclose(RECEIVER);

	// check if the file received is complete
	if (fileSizeReceived != fileSize)
		cout << "\n ERROR: The file is not complete!\n";
	else
		cout << "\nFile received with success\n";
	d->getStats(stats);
	return 0;
}

void appLayer::buildArgs(int argc, char* argv[]) {
	args["b"] = "NONE";
	args["t"] = "NONE";
	args["s"] = "NONE";
	args["r"] = "NONE";
	args["l"] = "NONE";
	args["error"] = "NONE";
	args["restore"] = "0";
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			char *key = &argv[i][1];
			args[key] = argv[i + 1];
			i++;
		}
	}

	//	map<string,string>::iterator it;

	// show content:
	//for ( it=args.begin() ; it != args.end(); it++ )
	//cout << (*it).first << " => " << (*it).second << endl;

	if (args["t"] != "NONE")
		timeout = atoi(args["t"].c_str());
	if (args["b"] != "NONE")
		baudrate = atoi(args["b"].c_str());
	if (args["r"] != "NONE")
		maxAttempts = atoi(args["r"].c_str());
	if (args["s"] != "NONE") {
		readSize = atoi(args["s"].c_str());
		if (readSize > 4000 || readSize < 5) {
			readSize = 4000;
			cout << "Readsize corrigido para default:4000\n";
		}
	}
	if (args["error"] != "NONE") {
		errorProb = atoi(args["error"].c_str());
	}

	strcpy(filePath, args["l"].c_str());

	restoreTransfer = atoi(args["restore"].c_str());

}

int appLayer::parseFileName(unsigned char *buf, char *filePath, int bufLen) {
	int fileSizeLen = buf[2];
	int i;
	int fileSize;
	char *fileSizeStr = new char[readSize];
	for (i = 3; i < (3 + fileSizeLen); i++) {
		fileSizeStr[i - 3] = buf[i];
	}

	fileSize = atoi(fileSizeStr);
	i += 2;
	if (filePath != 0) {
		for (int j = 0; i < bufLen; i++, j++) {
			filePath[j] = buf[i];
		}
	}

	return fileSize;

}
void appLayer::showStats() {
	cout << "\n=======================STATS===============================\n";
	cout << "- Total Rejects: " << stats["rejects"] << endl;
	cout << "- Total Timeouts: " << stats["timeouts"] << endl;
	cout << "- Total Frames Sent: " << stats["totalSent"] << endl;
	cout << "- Total Frames Received: " << stats["totalReceived"] << endl;
	cout << "===========================================================\n";
}

int appLayer::getTotalReceived() {
	return fileSizeReceived;
}
