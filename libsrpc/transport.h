//
//  transport.h
//
//  Created by Vadim Kalinsky on 13-08-06.
//  Copyright (c) 2013 Vadim Kalinsky. All rights reserved.
//

#ifndef __srpc__transport__
#define __srpc__transport__

#include <sys/socket.h>
#include <netinet/in.h>

#include <string>

namespace srpc
{
	
class ITransport
{
	friend class Server;
	
protected:
	virtual void read ( std::string& buffer )       = 0;
	virtual void open()                             = 0;
	virtual void close()                            = 0;

public:
	virtual void write( const std::string& buffer ) = 0;
	
	virtual ~ITransport() {};
};


class UDPTransport : public ITransport
{
private:
	int                mSockfd;
	int                mPort;
	
	struct sockaddr_in mServaddr;
	struct sockaddr_in mCliaddr;
	
public:
	UDPTransport( int port );
	~UDPTransport();
	
	void open();
	void close();
	
	void write( const std::string& buffer );
	
protected:
	void read( std::string& buffer );
};

}

#endif /* defined(__srpc__transport__) */
