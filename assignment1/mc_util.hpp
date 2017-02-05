//
//  mc_util.h
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>


#define BUFFER_SIZE 1024

typedef struct sockaddr_in socket_address;                      // Easy for construct
typedef struct sockaddr socket_address_system;                  // For system use

std::string mc_socket_read(int handler);
void mc_socket_write(int handler, std::string content);
void mc_zerofill_buffer(char * buffer);
std::string mc_socket_address_to_string(socket_address * address);