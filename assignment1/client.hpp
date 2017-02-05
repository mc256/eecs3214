//
//  client.h
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mc_util.hpp"

#define PORT_NUMBER 23111

typedef struct sockaddr_in socket_address;                      // Easy for construct
typedef struct sockaddr socket_address_system;                  // For system use
