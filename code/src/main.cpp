#include <iostream>
#include "datalink.h"
#include "appLayer.h"

using namespace std;

void printUsage();

int main(int argc, char* argv[]) {
	srand ( time(NULL) );
	if (argc >= 2) {
		if (strcmp(argv[1], "transmitter") == 0) {
			appLayer * app = new appLayer();
			app->buildArgs(argc,argv);
			app->sendFile();
			app->showStats();
			
		}
		else if (strcmp(argv[1], "receiver") == 0) {
			appLayer * app = new appLayer();
			app->buildArgs(argc,argv);
			app->receiveFile();
			app->showStats();
		}
		else
			printUsage();
	}
	else
		printUsage();
	return 0;
}

void printUsage(){
	cout << "usage: nserial [type] [options]" <<  endl;
	cout << endl << "[type] receiver or transmitter" << endl;
	cout << "[options]" << endl;
	cout << "-l\t\t File path  default: actual directory and the name of the transmitter's file" << endl;
	cout << "-s\t\t Size of the information block to be send in bytes (min:5 max:4000)" << endl;
	cout << "-restore\t Restore the connection in case of transmitter crash (0 or 1)" << endl;
	cout << "-t\t\t Time in seconds for the timeouts" << endl;
	cout << "-error\t\t Probability for creating errors (0-100)" << endl;
	cout << "-r\t\t Max attempts for reconnect to the receiver" << endl << endl;

	cout << "Example: ./nserial transmitter -l ~/Desktop/pinguim.gif -s 2000 -error 5 -r 4" << endl;
}
