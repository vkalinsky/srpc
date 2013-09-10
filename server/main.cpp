//
//  main.cpp
//
//  Created by Vadim Kalinsky on 13-08-05.
//  Copyright (c) 2013 Vadim Kalinsky. All rights reserved.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "../libsrpc/srpc.hpp"
#include "../libsrpc/srpc.h"

class CMD_Hello : public srpc::ICommand
{
	const char* name()        const { return "hello"; }
	const char* description() const { return "Says hello to the application"; }
	
	void invoke( const std::vector<std::string>& arguments, srpc::ITransport* output )
	{
		output->write("Hello, how can I help you?");
	}
};

class CMD_Sqrt : public srpc::ICommand
{
	const char* name()        const { return "sqrt"; }
	const char* description() const { return "Calculates square root"; }
	
	void invoke( const std::vector<std::string>& arguments, srpc::ITransport* output )
	{
		if( arguments.size() != 1 )
		{
			output->write("usage: sqrt <argument>");
		}
		else
		{
			float arg = atof( arguments[0].c_str() );
			
			if( arg < 0 )
				output->write("argument must be above zero");
			
			char res[50];
			snprintf( res, sizeof(res), "%f", sqrt(arg) );
			
			output->write(res);
		}
	}
};

static void cb_CMD_Random( const char* argv[], size_t argc, srpc_output_writer_t* output )
{
	srpc_write(output, "42");
	
	if( argc == 1 )
	{
		srpc_write(output, "... or ");
		srpc_write(output, argv[0]);
   }
}

int main(int argc, const char * argv[])
{
	srpc::Server server(32000);
	
	srpc::ICommand::ptr_t helloCommand = srpc::ICommand::ptr_t( new CMD_Hello );
	
	server.registerCommand( helloCommand );
	server.registerCommand( srpc::ICommand::ptr_t( new CMD_Sqrt ) );
	
	srpc_command_t randomCommand;
	randomCommand.name        = "random";
	randomCommand.description = "Returns random number";
	randomCommand.callback    = cb_CMD_Random;
	
	srpc_server_register_command( reinterpret_cast<srpc_server_handle_t*>(&server), &randomCommand );
	
	server.start();
	
	sleep(5000);
	
	server.stop();
	
	pthread_exit(NULL);
	
	return 0;
}

