//
//  mc_util.cpp
//  eecs3214a2
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include "mc_util.hpp"

using namespace std;

/*
    Read data from the socket and return as a string.

    @param handler the socket file descriptor.
    @return a string read from the socket.
*/
string mc_socket_read(int handler){
    string result = "";
    char buffer[BUFFER_SIZE + 1];   // Last character will be empty for concatenating the string.
    mc_zerofill_buffer(buffer);     // fill zero in a range [0, BUFFER_SIZE].

    while (true) {
        int length = recv(handler, buffer, BUFFER_SIZE, 0);
        if (length == 0){
            return string("");
        }
        result += string(buffer);
        if (buffer[length - 1] == 0){
            return result;
        }
    }
}

/*
    Write data to a socket.

    @param handler the socket file descriptor.
    @param content the string that you want to send.
*/
void mc_socket_write(int handler, string content){
    send(handler, content.c_str(), content.length() + 1, 0);
}


/*
    Initialize the buffer.

    @param buffer a pointer to the buffer.
*/
void mc_zerofill_buffer(char * buffer){
    for (int i = 0; i <= BUFFER_SIZE; ++i){
        *(buffer + i) = 0;
    }
}


/*
    Covert the socket address to human-readable address and port format.

    @param address the socket address you want to convert.
    @return human-readable string.
*/
string mc_socket_address_to_string(socket_address * address){
    ostringstream buffer;
    buffer << inet_ntoa(address->sin_addr) << ":" << ntohs(address->sin_port);
    return buffer.str();
}


/*
    Lock a mutex lock. If error, print the error and exit the program.

    @param mutex a pthread mutex lock.
*/
void mc_mutex_lock(pthread_mutex_t *mutex){
    if (pthread_mutex_lock(mutex)){
        cout << ERROR_THREAD << endl;
        exit(1);
    }
}

/*
    Unlock a mutex lock. If error, print the error and exit the program.

    @param mutex a pthread mutex lock.
*/
void mc_mutex_unlock(pthread_mutex_t *mutex){
    if (pthread_mutex_unlock(mutex)){
        cout << ERROR_THREAD << endl;
        exit(1);
    }
}

/*
    Try to lock a mutex lock.

    @param mutex a pthread mutex lock.
    @return true if you successfully acquired the lock; false if you cannot lock it.
*/
bool mc_mutex_trylock(pthread_mutex_t *mutex){
    if (pthread_mutex_trylock(mutex) == 0){
        return true;
    }
    return false;
}


/*
    Check a object if it is successfully created. If it is null, exit the program.

    @param object is an object
*/
void mc_check_null(void * object){
    if (object == NULL){
        cout << ERROR_MEMORY << endl;
        exit(1);
    }
}



/*
    Create a server. And return the socket descriptor.

    @param ip_address is the IP address you want to bind.
    @param port is the port that the server is listening on.
    @return the socket. If not success, a value of -1 shall be returned.
*/
int mc_create_server(in_addr_t ip_address, int port){
    // Configure the address
    socket_address * address = new socket_address;
    mc_check_null(&address);
    address->sin_family = AF_INET;                            // IPv4 is OK
    address->sin_addr.s_addr = ip_address;          // Address
    address->sin_port = htons(port);                          // Port
    socklen_t address_size = sizeof(* address);            // size of the listen address information

    // Create socket
    int connection;
    if ( (connection = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << ERROR_SOCKET << endl;
        return -1;
    }

    // Bind to address
    if ( bind(connection, (socket_address_system *) address, address_size) ){
        cout << ERROR_BIND << endl;   
        return -1;
    }

    // Listen
    if ( listen(connection, 0)  ){                           //A backlog argument of 0 may allow the socket to accept connections
        cout << ERROR_LISTEN << endl;
        return -1;
    }

    return connection;
}


/*
    Connect to a remote server and return the connection descriptor.

    @param remote_ip_address is the IP address for the remote server.
    @param port is the port that the server is listening on.
    @return the socket. If not success, a value of -1 shall be returned.
*/
int mc_connect_server(in_addr_t remote_ip_address, int port){

    // Configure the address
    socket_address * address = new socket_address;
    mc_check_null(&address);
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = remote_ip_address;
    address->sin_port = htons(port);
    socklen_t address_size = sizeof(* address);

    // Create socket
    int connection;
    if ( (connection = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << ERROR_SOCKET << endl;
        return -1;
    }

    // Connect
    if ( connect(connection, (socket_address_system *) address, address_size) != 0){
        cout << ERROR_ACCEPT << endl;   
        return -1;
    }

    return connection;
}