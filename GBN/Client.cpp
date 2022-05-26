//*****************************************
//Seyyed Adel Mirsharji			9730653//
//*****************************************
//UDP Client

#include<iostream>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") 		//Winsock Library

#define SERVER "127.0.0.1"				//ip address of udp server
#define BUFLEN 1024						//Max length of buffer
#define PORT 8888						//The port on which to listen for incoming data

using namespace std;

struct frame
{
	int packet[40];
};

struct ack
{
	int acknowledge[40];
};

int main(void)
{
	struct sockaddr_in si_other;
	int out, slen=sizeof(si_other);
	frame frame1;
	ack acknowledgement;
	int windowSize;
	int totalPackets;
	int i,j = 0;
	int numberOfFramesRecived = 0;
	int numberOfPacketsReceived = 0;
	int temp;
	int randomNumber = 0;
	int startProcess = 0;
	int receivedFrameStatus = 0;
	char buf[BUFLEN];
	char message[BUFLEN] = "Hello from client!";
	WSADATA wsa;

	//Initialise winsock
	cout << endl << "*********************************************";
	cout << endl << "Initialising Winsock..." << endl;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		cout << "Failed. Error Code : "<< WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Initialised." << endl << endl;
	cout << "*********************************************" << endl;
	
	//create socket
	if ( (out=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		cout << "socket() failed with error code :" << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	
	//setup address structure
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	//******************************************************************	
	//start communication
	//Connecting to sever via port
	if (sendto(out, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR){
			cout << "sendto() failed with error code : " << WSAGetLastError() << endl;
			exit(EXIT_FAILURE);
	}

	//******************************************************************
	//Obtaining server request
	memset(buf,'\0', BUFLEN);
	if (recvfrom(out, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR){
			cout << "recvfrom() failed with error code : " << WSAGetLastError() << endl;
			exit(EXIT_FAILURE);
	}
	cout << endl << "Serevr requested : " << buf << endl;

	//Getting window size from user
	cout << "Please enter window size =  ";
	cin >> windowSize;
	//Send window size to server
	if (sendto(out, (char*)&windowSize, sizeof(windowSize) , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR){
			cout << "sendto() failed with error code : " << WSAGetLastError() << endl;
			exit(EXIT_FAILURE);
	}
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Wait to recive total numebr of packets from server obtained from network layer
	cout << endl << "Wait to receive total number of packets from server..." << endl;
	if (recvfrom(out, (char*)&totalPackets, sizeof(totalPackets), 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR){
			cout << "recvfrom() failed with error code : " << WSAGetLastError() << endl;
			exit(EXIT_FAILURE);
	}
	cout << "Total number of packets to receive from server obtained is = " << totalPackets << endl;

	//Send ack to server that total number of frames is received!
	cout << endl << "Sending ack to server that total number of packets received!" << endl;
	if (sendto(out, "From client : total number of packets received!", sizeof("From client : total number of packets received!") , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR){
			cout << "sendto() failed with error code : " << WSAGetLastError() << endl;
			exit(EXIT_FAILURE);
	}
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Wait till process starts
	while(startProcess == 0){
		cout << endl << "Wait till process starts!" << endl;
		recvfrom(out, (char*)&startProcess, sizeof(startProcess), 0, (struct sockaddr *) &si_other, &slen);
	}
	cout << "Transmitting process started!" << endl;
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Receiving frame from server
	while(numberOfPacketsReceived < totalPackets){
		cout << endl << "Wait to recieve data from server..." << endl;
		recvfrom(out,(char*)&numberOfPacketsReceived, sizeof(numberOfPacketsReceived), 0, (struct sockaddr *) &si_other, &slen);
		recvfrom(out,(char*)&numberOfFramesRecived, sizeof(numberOfFramesRecived), 0, (struct sockaddr *) &si_other, &slen);

		//recive frame and packets from serevr
		recvfrom(out,(char*)&frame1, sizeof(frame1), 0, (struct sockaddr *) &si_other, &slen);

		//Print out wich frame and packets are needed
		temp = numberOfPacketsReceived-windowSize;
		cout << "Frame-" << numberOfFramesRecived << " is needed to be received!" << endl << "whit packets:";
		for(i = 0 ; i < windowSize ; ++i){
			cout << " packet = " << frame1.packet[i] << " || ";
			temp++;

			if(frame1.packet[i] == totalPackets-1)
				break;
		}

		//randomly decide a packet is lost or received successfully
		cout << endl << endl << "Wait for packets to be recived..." << endl;
		temp = numberOfPacketsReceived-windowSize;
		receivedFrameStatus = 0;
		for(i = 0 ; i < windowSize ; ++i){
			randomNumber = rand()%5;
			if(randomNumber  < 2){
				acknowledgement.acknowledge[i] = -1;
				cout << "Packet = " << frame1.packet[i] << " is lost!" <<  endl;
				receivedFrameStatus = 1;
			}
			else if(randomNumber >= 2){
				acknowledgement.acknowledge[i] = 1;
				cout << "packet = " << frame1.packet[i] << " successfully received!" << endl;
			}
			if(frame1.packet[i] == totalPackets-1)
				break;
		}
		if(receivedFrameStatus == 0)
			cout << "Frame-" << numberOfFramesRecived << "received successfully!" << endl;
		else if(receivedFrameStatus == 1)
			cout << endl << "Received Frame-" << numberOfFramesRecived << " was damaged...waiting till server resend the frame!" << endl;

		//Sending ack(1) or nak(-1) to server
		sendto(out, (char*)&acknowledgement, sizeof(acknowledgement), 0, (struct sockaddr *) &si_other, slen);

		//Waiting to receive frame completely seuccessfull!
		while(receivedFrameStatus == 1){
			//wait untill frame is received again!
			cout << endl << "Wait to receive frame-" << numberOfFramesRecived << " again!" << endl;
			recvfrom(out,(char*)&frame1, sizeof(frame1), 0, (struct sockaddr *) &si_other, &slen);
			cout << "Frame-" << numberOfFramesRecived << " received again" << endl;

			//randomly decide if frame is received successfully or not
			temp = numberOfPacketsReceived-windowSize;
			receivedFrameStatus = 0;
			for(i = 0 ; i < windowSize ; ++i){
				randomNumber = rand()%5;
				if(randomNumber  < 2){
					acknowledgement.acknowledge[i] = -1;
					cout << "Packet = " << frame1.packet[i] << " is lost!" <<  endl;
					receivedFrameStatus = 1;
				}
				else if(randomNumber >= 2){
					acknowledgement.acknowledge[i] = 1;
					cout << "packet = " << frame1.packet[i] << " successfully received!" << endl;
				}
				if(frame1.packet[i] == totalPackets-1)
					break;
			}
			if(receivedFrameStatus == 0)
				cout << endl << "Frame-" << numberOfFramesRecived << " received successfully!...going to receive next frame!" << endl;
			else if(receivedFrameStatus == 1)
				cout << endl << "Received Frame-" << numberOfFramesRecived << " was damaged...waiting till server resend the frame!" << endl;

			//Sending ack(1) or nak(-1) to server
			sendto(out, (char*)&acknowledgement, sizeof(acknowledgement), 0, (struct sockaddr *) &si_other, slen);
		}
	}

	cout << endl << "*********************************************" << endl;

	//close socket
	closesocket(out);
	//shutdown winsock
	WSACleanup();

	return 0;
}