//
//  srpc.cpp
//
//  Created by Vadim Kalinsky on 13-08-05.
//  Copyright (c) 2013 Vadim Kalinsky. All rights reserved.
//

#include "srpc.hpp"

using namespace srpc;

/////////////////////////////////////////////////////////////////////////////
// Server

Server::Server( int port )
{
	mTransport = new UDPTransport(port);
	
	mNeedStop = true;
	
	pthread_mutex_init(&mCommandsMutex, NULL);
}


Server::~Server()
{
	delete mTransport;
	mTransport = NULL;
	
	pthread_mutex_destroy(&mCommandsMutex);
}


void Server::showHelp() const
{
	std::string helpText;
	
	// Calculate widths for alignment
	size_t maxNameWidth = 0;
	for( auto const &it: mKnownCommands )
	{
		size_t len = strlen(it->name());
		if( len > maxNameWidth )
			maxNameWidth = len;
	}
	
	for( auto const &it: mKnownCommands )
	{
		size_t len = strlen(it->name());
		
		helpText.append(it->name());
		
		// Align descriptions
		for( int i=0; i<maxNameWidth-len + 4; i++ )
			helpText.push_back(' ');
		
		helpText.append(it->description());
		helpText.push_back('\r');
		helpText.push_back('\n');
	}
	
	mTransport->write("Available commands:\r\n");
	mTransport->write(helpText.c_str());
	mTransport->write("\r\n");
}


void Server::handleRequest( const std::string& request )
{
	std::string buf;
	std::stringstream ss(request);
	ICommand::arglist_t arglist;
	
	std::string command;

	// Split line to arguments
	// TODO: Add quoting/escaping support.
	int i = 0;
	while( ss >> buf )
	{
		if( i==0 )
			command = buf;
		else
			arglist.push_back(buf);
		
		i++;
	}
	
	if( command.size() == 0 )
		return;
	
	// TODO: locks (from shared library)
	if( command == "help" )
	{
		showHelp();
		return;
	}
	
	// Execute command
	for( auto &it : mKnownCommands )
	{
		if( it->name() == command )
		{
			it->invoke(arglist, mTransport);
			
			mTransport->write("\r\n");
			
			return;
		}
	}
	
	mTransport->write( "Unknown command\r\n" );
}


// Dummy thread function
void* Server::serverMainThread( void* param )
{
	Server* self = (Server*)param;
	self->serverMain();
	
	return NULL;
}

// Main thread function
void Server::serverMain()
{
	while( !mNeedStop )
	{
		std::string buffer;
		mTransport->read(buffer);
		
		if( !buffer.empty() )
		{
			handleRequest( buffer );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Public interface

void Server::registerCommand( ICommand::ptr_t command )
{
	pthread_mutex_lock(&mCommandsMutex);
	mKnownCommands.push_back(command);
	pthread_mutex_unlock(&mCommandsMutex);
}

void Server::unregisterCommand( ICommand::ptr_t command )
{
	pthread_mutex_lock(&mCommandsMutex);
	mKnownCommands.remove(command);
	pthread_mutex_unlock(&mCommandsMutex);
}

void Server::start()
{
	mTransport->open();
	mNeedStop = false;
	pthread_create(&mMainThread, NULL, serverMainThread, (void*)this);
}

void Server::stop()
{
	if( ! mNeedStop )
	{
		mTransport->close();
		mNeedStop = true;
		pthread_join(mMainThread, NULL);
	}
}


///////////////////////////////////////////////////////////////////////////////
// C interface

#include "srpc.h"

#define SERVER_TO_HANDLE(a) reinterpret_cast<srpc_server_handle_t*>(a)
#define HANDLE_TO_SERVER(a) reinterpret_cast<Server*>(a)

// C command adapter
class CCommandAdapter : public ICommand
{
private:
	std::string             mName;
	std::string             mDescription;
	srpc_command_callback_t mCallback;
	
public:
	CCommandAdapter( srpc_command_t* command )
	{
		mName        = command->name;
		mDescription = command->description;
		mCallback    = command->callback;
	}
	
	const char* name() const
	{
		return mName.c_str();
	}
	
	const char* description() const
	{
		return mDescription.c_str();
	}
	
	void invoke( const arglist_t& arguments, ITransport* output )
	{
		const char** argv = new const char*[arguments.size()];
		
		int i = 0;
		for( auto const &it: arguments )
		{
			argv[i] = it.c_str();
			i++;
		}
		
		mCallback( argv, arguments.size(), reinterpret_cast<srpc_output_writer_t*>(output) );
		
		free(argv);
	}
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Server API
srpc_server_handle_t* srpc_server_create( int port )
{
	return SERVER_TO_HANDLE(new Server(port));
}

void srpc_server_delete( srpc_server_handle_t* server )
{
	delete HANDLE_TO_SERVER(server);
}

void srpc_server_start(srpc_server_handle_t* server)
{
	HANDLE_TO_SERVER(server)->start();
}

void srpc_server_stop(srpc_server_handle_t* server)
{
	HANDLE_TO_SERVER(server)->stop();
}

void srpc_server_register_command(srpc_server_handle_t* server, srpc_command_t* command)
{
	ICommand::ptr_t cmd = ICommand::ptr_t( new CCommandAdapter(command) );
	
	HANDLE_TO_SERVER(server)->registerCommand(cmd);
}

// Transport API
void srpc_write( srpc_output_writer_t* writer, const char* message )
{
	ITransport* transport = reinterpret_cast<ITransport*>(writer);
	transport->write(message);
}

#ifdef __cplusplus
}
#endif // __cplusplus
