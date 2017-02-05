//
//  server.h
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include <iostream>
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
#define BIND_PORT 23111
#define BIND_ADDR "0.0.0.0"

#define ERROR_SOCKET "socket error"
#define ERROR_BIND "cannot bind to address"
#define ERROR_LISTEN "listen error"
#define ERROR_ACCEPT "cannot establish connection"
#define ERROR_SERVER "cannot handle connection"
#define ERROR_THREAD "unable to handle a thread lock"

//STRUCTURE
typedef struct sockaddr_in socket_address;                      // Easy for construct
typedef struct sockaddr socket_address_system;                  // For system use
typedef struct client_structure
{
    socket_address * address;
    pthread_t thread;
    int handler;
    client_structure * prev;
    client_structure * next;
    bool active;
    std::string message_buffer;
    pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;
} client_structure;


//VARIABLE
int mc_connection;
pthread_t mc_server_menu_thread;
pthread_t mc_listener_thread;
client_structure * mc_user_list;
pthread_mutex_t mc_user_list_mutex = PTHREAD_MUTEX_INITIALIZER;


//METHOD
int main(int argc, char ** argv);

void * mc_listener(void * connection);
void * mc_server_menu(void * connection);

int mc_create_client(socket_address * address, int handler);
int mc_remove_client(client_structure * current_client);
std::string mc_list_clients(bool show_unactive);
void * mc_message_handler(void * current_client);
void mc_spread_out_message(client_structure * current_client, std::string message);