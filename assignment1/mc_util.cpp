//
//  mc_util.cpp
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include "mc_util.hpp"

using namespace std;

string mc_socket_read(int handler){
    string result = "";
    char buffer[BUFFER_SIZE + 1];   // last character will be empty for concatenating the string
    mc_zerofill_buffer(buffer);     // fill zero in a range [0, BUFFER_SIZE]

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

void mc_socket_write(int handler, string content){
    send(handler, content.c_str(), content.length() + 1, 0);
}

void mc_zerofill_buffer(char * buffer){
    for (int i = 0; i <= BUFFER_SIZE; ++i){
        *(buffer + i) = 0;
    }
}

string mc_socket_address_to_string(socket_address * address){
    ostringstream buffer;
    buffer << inet_ntoa(address->sin_addr) << ":" << ntohs(address->sin_port);
    return buffer.str();
}

void mc_mutex_lock(pthread_mutex_t *mutex){
    if (pthread_mutex_lock(mutex)){
        cout << ERROR_THREAD << endl;
        exit(1);
    }
}

void mc_mutex_unlock(pthread_mutex_t *mutex){
    if (pthread_mutex_unlock(mutex)){
        cout << ERROR_THREAD << endl;
        exit(1);
    }
}

// true if you can lock it
bool mc_mutex_trylock(pthread_mutex_t *mutex){
    if (pthread_mutex_trylock(mutex)){
        return true;
    }
    return false;
}

void mc_check_null(void * object){
    if (object == NULL){
        cout << ERROR_MEMORY << endl;
        exit(1);
    }
}