//*****************************************
//Seyyed Adel Mirsharji			9730653//
//*****************************************
//UDP Server

#include<iostream>
#include <chrono>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") 		//Winsock Library

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



int main()
{
	SOCKET in;
	struct sockaddr_in server, clientAddr;
	int clientAddrLen , recv_len;
	int windowSize;
	int totalPackets;
	int framesSend = 0;
	int i,j = 0;
	int sentPacketStatus = 0;
	int numberOfPacketLoss = 0;
	int repacket[40];
	char req[40];
	int startProcess = 0;
	frame frame1;
	frame frame2;
	ack acknowledgement;
	char buf[BUFLEN];

	//Timer vairables
	using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
	//**************************************

	WSADATA wsa;
	clientAddrLen = sizeof(clientAddr) ;
	
	//******************************************************************
	//Initialise winsock
	cout << endl << "*********************************************";
	cout << endl << "Initialising Winsock..." << endl;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	cout << "Initialised." << endl;

	//Create a socket
	if((in = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
		cout << "Could not create socket : " << WSAGetLastError() << endl;

	cout << "Socket created!" << endl;
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );
	
	//Bind
	if( bind(in ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		cout << "Bind failed with error code : " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Bind done" << endl;
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Waiting for client connection
	cout << endl << "Waiting for client connection" << endl;
	if ((recv_len = recvfrom(in, buf, BUFLEN, 0, (struct sockaddr *) &clientAddr, &clientAddrLen)) == SOCKET_ERROR){
		cout << "recvfrom() failed with error code : " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	cout << "The client " <<  inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connection obtained!"<< endl;
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Sending request for window size
	cout << endl << "Sending request for window size!" << endl;
	if (sendto(in, "Request for window size", sizeof("Request for window size"), 0, (struct sockaddr *) &clientAddr, clientAddrLen) == SOCKET_ERROR){
		cout << "recvfrom() failed with error code : " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}

	//Waiting for window size
	cout << "Waiting for window size..." << endl;
	if ((recv_len = recvfrom(in, (char*)&windowSize, sizeof(windowSize), 0, (struct sockaddr *) &clientAddr, &clientAddrLen)) == SOCKET_ERROR){
		cout << "recvfrom() failed with error code : " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	cout << "The window size obtained as = " << windowSize << endl;
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Obtaining packets from network layer
	cout << endl << "Obtaining packets from network layer" << endl;
	totalPackets = windowSize * 5;
	cout << "Total number of packets obtained = " << totalPackets << endl;
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Sending total numebr of packets to client
	cout << endl << "Sending total number of packets to client..." << endl;
	if (sendto(in, (char*)&totalPackets, sizeof(totalPackets), 0, (struct sockaddr *) &clientAddr, clientAddrLen) == SOCKET_ERROR){
		cout << "recvfrom() failed with error code : " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}

	memset(buf,'\0', BUFLEN);
	//Wait for client to receive total number of packets
	cout << "Wait for client to receive total number of packets ..." << endl;
	if ((recv_len = recvfrom(in, buf, BUFLEN, 0, (struct sockaddr *) &clientAddr, &clientAddrLen)) == SOCKET_ERROR){
		cout << "recvfrom() failed with error code : " << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	cout << buf << endl;
	cout << endl << "*********************************************" << endl;

	//******************************************************************
	//Staring transmitting process via SRARQ protocol
	cout << endl << "Press ENTER to start transmitting process via SRARQ protocol" << endl;
	fgets(req,2,stdin);

	cout << endl << "*********************************************" << endl;
	startProcess = 1;
	cout << endl << "Transmitting data to client process started!" << endl;
	//Send ack to client that process started
	sendto(in, (char*)&startProcess, sizeof(startProcess), 0, (struct sockaddr *) &clientAddr, clientAddrLen);

	//******************************************************************
	//creating packets
	for(i = 0 ; i < totalPackets ; ++i){
		frame1.packet[i] = i;
	}	
	//Sending packets to client whit SRARQ protocol
	int numberOfPacketsSent = 0;
	int numberOfframesSent = 0;
	int numberOfFailedPackets = 0;
	int counter = 0;
	int temp = numberOfPacketsSent;

	for(i = 0 ; i < windowSize ; ++i){
		repacket[i] = 0;
	}

	for(i = 0 ; i < windowSize ; ++i){
		acknowledgement.acknowledge[i] = 0;
	}


	auto t1 = high_resolution_clock::now();					//to get run time of SRARQ whit W window size
	while(numberOfPacketsSent < totalPackets){
		numberOfFailedPackets = 0;
		for(i = 0 ; i < windowSize ; ++i){
			if(acknowledgement.acknowledge[i] == -1){
				repacket[i] = 1;
				numberOfFailedPackets++;
			}

		}

		counter = 0;
		for(j = 0 ; j < windowSize ; ++j){
			if(repacket[j] == 1){
				frame2.packet[counter] = frame2.packet[j];
				counter++;
				repacket[j] = 0;
				acknowledgement.acknowledge[j] = 0;
			}
		}

		for(i = 0 ; i < (windowSize-numberOfFailedPackets) ; ++i){
			frame2.packet[counter] = frame1.packet[numberOfPacketsSent];
			counter++;
			numberOfPacketsSent++;
		}

		numberOfframesSent++;

		//print out which frame and packets are going to be sent to client
		cout << endl << "Sending frame-" << numberOfframesSent << " Containing packets: ";
		for(i = 0 ; i < windowSize ; ++i){
			cout <<  frame2.packet[i] << " || ";
			if(frame2.packet[i] == totalPackets-1)
				break;
		}

		//Sending detail to client
		sendto(in, (char*)&numberOfPacketsSent, sizeof(numberOfPacketsSent), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
		sendto(in, (char*)&numberOfframesSent, sizeof(numberOfframesSent), 0, (struct sockaddr *)&clientAddr, clientAddrLen);

		//Sending frames to client
		sendto(in, (char*)&frame2, sizeof(frame2), 0, (struct sockaddr *)&clientAddr, clientAddrLen);

		//Wait for aknowledgement from client
		cout << endl << "Waiting for aknowledgement from client..." << endl;
		recvfrom(in, (char*)&acknowledgement, sizeof(acknowledgement), 0, (struct sockaddr *) &clientAddr, &clientAddrLen);

		//Check if frame was sent completly successfull or there was any packet loss
		sentPacketStatus = 0;
		numberOfPacketLoss = 0;
		for(i = 0 ; i < windowSize ; ++i){
			if(acknowledgement.acknowledge[i] == -1 && frame2.packet[i] <= totalPackets){
				if(sentPacketStatus == 0)
					sentPacketStatus = 1;

				numberOfPacketLoss++;
			}
		}
		if(sentPacketStatus == 1)
		cout << "Frame-" << numberOfframesSent << " didn't sent successfully!...There was " << numberOfPacketLoss << " packet loss(s) in sent frame!" << endl;
		else if(sentPacketStatus == 0)
			cout << "Frame-" << numberOfframesSent << " sent successfully!" << endl;
		
		//Print out sent packets status
		for(i = 0 ; i < windowSize ; ++i){
			cout << "Packet"  << " = " << frame2.packet[i] << " ACK is = " << acknowledgement.acknowledge[i];
			if(acknowledgement.acknowledge[i] == -1)
				cout << ". packet" << " is lost!" << endl;
			else
				cout << ". packet" << " sent successfully!" <<endl;

			temp++;

			if(frame2.packet[i] == totalPackets-1)
				break;
		}
	}

	//******************************************************************
	//End timing
	auto t2 = high_resolution_clock::now();

	//Close socket
	closesocket(in);

	//Shutdown winsock
	WSACleanup();
	
	/* Getting number of milliseconds as an integer. */
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;
	cout << endl << "*********************************************" << endl;
    cout << endl << "Selective repeat ARQ time taken to send " << totalPackets << " packets whit window size of " << windowSize << " in miliseconds as integer is = " << ms_int.count() << "ms" << endl;
    cout << endl << "Selective repeat ARQ time taken to send " << totalPackets << " packets whit window size of " << windowSize << " in miliseconds as double is = " << ms_double.count() << "ms" << endl;
	cout << endl << "*********************************************" << endl;
	
	return 0;
}