//
//  client.h
//  eecs3214a2
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "mc_util.hpp"

//CONSTANT
#define BIND_PORT 23111
#define BIND_ADDR "0.0.0.0"
#define SHUTDOWN_TIMEOUT 1
#define INTERVAL 50000

#define ERROR_LOSTCNT "connection lost"

//STRUCTURE
typedef struct sockaddr_in socket_address;                      // Easy for construct
typedef struct sockaddr socket_address_system;                  // For system use

typedef struct peer_structure
{
	int id;
    in_addr_t ip_address;
    int port;
    peer_structure * next;
} peer_structure;


//VARIABLE
int mc_connection;
int mc_peer_port;
pthread_t mc_client_thread;
pthread_t mc_heartbeat_thread;
pthread_mutex_t mc_holding_mutex;
peer_structure * mc_peer_list;

//METHOD
int main(int argc, char ** argv);
void * mc_client_response(void * connection);
void * mc_heartbeat_response(void * connection);
void * mc_update_peer_list(std::string data);