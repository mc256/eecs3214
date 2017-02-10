//
//  client.h
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "mc_util.hpp"

//CONSTANT
#define PORT_NUMBER 23111
#define INTERVAL 50000

#define ERROR_SOCKET "socket error"
#define ERROR_BIND "cannot bind to address"
#define ERROR_LISTEN "listen error"
#define ERROR_ACCEPT "cannot establish connection"
#define ERROR_SERVER "cannot handle connection"
#define ERROR_LOSTCNT "connection lost"

//STRUCTURE
typedef struct sockaddr_in socket_address;                      // Easy for construct
typedef struct sockaddr socket_address_system;                  // For system use

//VARIABLE
int mc_connection;
pthread_t mc_client_thread;
pthread_t mc_heartbeat_thread;
pthread_mutex_t mc_holding_mutex;


//METHOD
int main(int argc, char ** argv);
void * mc_client_response(void * connection);
void * mc_heartbeat_response(void * connection);