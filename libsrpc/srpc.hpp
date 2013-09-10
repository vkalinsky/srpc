//
//  srpc.h
//
//  Created by Vadim Kalinsky on 13-08-05.
//  Copyright (c) 2013 Vadim Kalinsky. All rights reserved.
//

#ifndef __udp_server__srpc__
#define __udp_server__srpc__

#include <pthread.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <list>

#include "transport.h"

namespace srpc
{

class ICommand
{
public:
	typedef std::shared_ptr<ICommand> ptr_t;
	typedef std::vector<std::string>  arglist_t;
	
public:
	virtual void        invoke( const arglist_t& arguments, ITransport* output ) = 0;
	virtual const char* name()        const = 0;
	virtual const char* description() const = 0;
};


class Server
{
private:
	std::list<ICommand::ptr_t> mKnownCommands;
	pthread_mutex_t mCommandsMutex;
	ITransport*     mTransport;
	pthread_t       mMainThread;
	bool            mNeedStop;
	
private:
	void            handleRequest( const std::string& request );
	void            showHelp() const;

	void            serverMain();
	static void*    serverMainThread( void* param );
	
	
public:
	      Server( int port );
	      Server( ITransport* transport );
	     ~Server();
	
	void  registerCommand  ( ICommand::ptr_t command );
	void  unregisterCommand( ICommand::ptr_t command );
	void  start();
	void  stop();
};
	
}

#endif /* defined(__udp_server__srpc__) */
