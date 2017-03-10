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
    if (pthread_mutex_trylock(mutex)){
        return true;
    }
    return false;
}


/*
    Check a object if it is successfully created. If it is null, exit the program.

    @param an object
*/
void mc_check_null(void * object){
    if (object == NULL){
        cout << ERROR_MEMORY << endl;
        exit(1);
    }
}