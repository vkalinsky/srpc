//
//  srpc.h
//
//  Created by Vadim Kalinsky on 13-08-06.
//  Copyright (c) 2013 Vadim Kalinsky. All rights reserved.
//

#ifndef srpc_srpc_h
#define srpc_srpc_h

#include <sys/types.h>

typedef struct { int dummy; } srpc_server_handle_t;
typedef struct { int dummy; } srpc_output_writer_t;

typedef void (*srpc_command_callback_t)(const char* argv[], size_t argc, srpc_output_writer_t* output);

typedef struct {
	const char*              name;
	const char*              description;
	srpc_command_callback_t  callback;
} srpc_command_t;

// Server

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	
srpc_server_handle_t* srpc_server_create( int port );
void                  srpc_server_delete( srpc_server_handle_t* server );

void                  srpc_server_start(srpc_server_handle_t* server);
void                  srpc_server_stop(srpc_server_handle_t* server);

void                  srpc_server_register_command(srpc_server_handle_t* server, srpc_command_t* command);


// Command output writer

void srpc_write( srpc_output_writer_t* writer, const char* message );

#ifdef __cplusplus
}
#endif // __cplusplus
	
#endif
