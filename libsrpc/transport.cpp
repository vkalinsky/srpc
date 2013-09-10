//
//  transport.cpp
//
//  Created by Vadim Kalinsky on 13-08-06.
//  Copyright (c) 2013 Vadim Kalinsky. All rights reserved.
//

#include <sys/select.h>
#include <sys/errno.h>

#include "transport.h"

using namespace srpc;

UDPTransport::UDPTransport( int port )
{
	mPort   = port;
	mSockfd = -1;
}

UDPTransport::~UDPTransport()
{
	close();
}

void UDPTransport::open()
{
	mSockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&mServaddr, 0, sizeof(mServaddr));
	mServaddr.sin_family      = AF_INET;
	mServaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	mServaddr.sin_port        = htons(mPort);
	bind( mSockfd, (struct sockaddr *)&mServaddr, sizeof(mServaddr) );
}

void UDPTransport::close()
{
	if( mSockfd != -1 )
	{
		printf( "shutdown\n" );
		shutdown(mSockfd, SHUT_RDWR);
		mSockfd = -1;
	}
}

void UDPTransport::write( const std::string& buffer )
{
	sendto(mSockfd, buffer.c_str(), buffer.size(), 0, (struct sockaddr*)&mCliaddr, sizeof(mCliaddr));
}

void UDPTransport::read( std::string& buffer )
{
	socklen_t len = sizeof(mCliaddr);
	char b[512];
	
	// TODO: Make signal pipe for fast cancelling of 'select' upon closing.
	while( mSockfd != -1 )
	{
		fd_set fs;
		FD_ZERO(&fs);
		FD_SET(mSockfd, &fs);
		struct timeval timeout;
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;
		
		int sr = select(mSockfd+1, &fs, NULL, NULL, &timeout);
		
		if( sr != 0 ) // If not a timeout
		{
			if( sr == - 1 ) // Error occured - just exiting
				return;
			else if( FD_ISSET(mSockfd, &fs) )
				break;
		}
	}

	ssize_t n = recvfrom( mSockfd, b, sizeof(b)-1, 0, (struct sockaddr *)&mCliaddr, &len );
	if( n > 0 )
	{
		b[n] = '\0';
	}
	
	buffer.clear();
	buffer.append(b);
}
