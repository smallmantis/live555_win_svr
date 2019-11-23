/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)
This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.
You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2013, Live Networks, Inc.  All rights reserved
// LIVE555 Media Server
// main program

#include <BasicUsageEnvironment.hh>
#include "DynamicRTSPServer.hh"
#include "version.hh"
#include "H264LiveVideoSource.h"
#include "H264LiveVideoServerMediaSubsession.h"

int g_sock = -1;

static int socket_tcp(int& sock_client)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	int sockSrv = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(sockSrv, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
		return -1;
	}
	//¿ªÊ¼¼àÌý  
	if (listen(sockSrv, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}

	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	sock_client = accept(sockSrv, (SOCKADDR *)&remoteAddr, &nAddrlen);
	if (sock_client == INVALID_SOCKET)
	{
		printf("accept error !");
		return -1;
	}

	printf("accept ok, client:%s\n", inet_ntoa(remoteAddr.sin_addr));

	return sockSrv;

}

static int socket_udp()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	int sockSrv = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(sockSrv, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
		return -1;
	}

	return sockSrv;

}

int main(int argc, char** argv) {
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	OutPacketBuffer::maxSize = 3000000;

	UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
	// To implement client access control to the RTSP server, do the following:
	authDB = new UserAuthenticationDatabase;
	authDB->addUserRecord("username1", "password1"); // replace these with real strings
	// Repeat the above with each <username>, <password> that you wish to allow
	// access to the server.
#endif

	// Create the RTSP server:
	RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

#if 0
	printf("###### Welcome Live555 Win Server!!!\n");
	printf("Server starting now...\n");
	printf("Waiting for pusher connect and push rtsp stream...\n");
	socket_tcp(g_sock);
#endif

	H264LiveVideoSource * videoSource = 0;

	ServerMediaSession * sms = ServerMediaSession::createNew(*env, "mystream", 0, "mystream test");
	sms->addSubsession(H264LiveVideoServerMediaSubsession::createNew(*env, videoSource));
	rtspServer->addServerMediaSession(sms);

	char * url = rtspServer->rtspURL(sms);
	*env << "using url \"" << url << "\"\n";
	delete[] url;

	// Run loop
	env->taskScheduler().doEventLoop();

	rtspServer->removeServerMediaSession(sms);
	Medium::close(rtspServer);
	env->reclaim();
	delete scheduler;

	return 1;
}