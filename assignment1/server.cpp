//
//  server.cpp
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include "server.h"

using namespace std;

int main(int argc, char ** argv) {
    //Configure the address
    socket_address * mc_address = new socket_address;
    mc_address->sin_family = AF_INET;                            // IPv4 is OK
    mc_address->sin_addr.s_addr = inet_addr(BIND_ADDR);          // Address
    mc_address->sin_port = htons(BIND_PORT);                     // Port
    socklen_t mc_address_size = sizeof(* mc_address);            // size of the listen address information

    //Create socket
    if ( (mc_connection = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << ERROR_SOCKET << endl;
        return 1;
    }

    //Bind to address
    if ( bind(mc_connection, (socket_address_system *) mc_address, mc_address_size) ){
        cout << ERROR_BIND << endl;   
        return 1;
    }

    //Listen
    if ( listen(mc_connection, 0)  ){                           //A backlog argument of 0 may allow the socket to accept connections
        cout << ERROR_LISTEN << endl;
        return 1;
    }

    //User List
    mc_user_list = new client_structure;
    mc_user_list->next = mc_user_list->prev = NULL;


    //Server-end Menu
    pthread_create(&mc_server_menu_thread, NULL, mc_server_menu, (void *) &mc_connection);
    pthread_create(&mc_listener_thread, NULL, mc_listener, (void *) &mc_connection);

    pthread_join(mc_server_menu_thread, NULL);

    return 0;
}

void * mc_listener(void * connection){
    while (true) {
        int mc_handler;
        socket_address * mc_remote_address = new socket_address;
        socklen_t mc_remote_address_size = sizeof(* mc_remote_address);

        //Accept
        if ( (mc_handler = accept(*(int *) connection, (socket_address_system *) mc_remote_address, &mc_remote_address_size)) < 0){
            cout << ERROR_ACCEPT << endl;
            break;
        }

        //Create new thread for Handling
        if ( mc_create_client(mc_remote_address, mc_handler) ){
            cout << ERROR_SERVER << endl;
            continue;
        }
    }
}

void * mc_server_menu(void * connection){
    //TODO!!!!!!!
    while (true){
        int action;
        cin >> action;
        if (action == 1){
            printOnlineUsers();
        }else if (action == 0){
            //need to go through all the user and close
            close(*(int *) connection);
            break;
        }
    }
}


int mc_create_client(socket_address * address, int handler){
    //Init
    client_structure * mc_client = new client_structure;
    mc_client->address = address;
    mc_client->handler = handler;

    //Add user to linked list
    if ( pthread_mutex_lock(&mc_user_list_mutex) ){
        cout << ERROR_THREAD << endl;
        return 1;
    }

    mc_client->next = mc_user_list->next;
    mc_client->prev = mc_user_list;
    if (mc_user_list->next != NULL){
        mc_user_list->next->prev = mc_client;
    }
    mc_user_list->next = mc_client;

    if ( pthread_mutex_unlock(&mc_user_list_mutex) ){
        cout << ERROR_THREAD << endl;
        return 1;
    }


    //Process
    return pthread_create(&(mc_client->thread), NULL, mc_message_handler, (void *) mc_client);
}

int mc_remove_client(client_structure * current_client){
    if ( pthread_mutex_lock(&mc_user_list_mutex) ){
        cout << ERROR_THREAD << endl;
        return 1;
    }
    
    if (current_client->next != NULL){
        current_client->next->prev = current_client->prev;
    }
    current_client->prev->next = current_client->next;

    if ( pthread_mutex_unlock(&mc_user_list_mutex) ){
        cout << ERROR_THREAD << endl;
        return 1;
    }

    current_client->prev = current_client->next = NULL;

    delete current_client->address;
    delete current_client;

    return 0;
}

void * mc_message_handler(void * current_client){
    client_structure * mc_client = (client_structure *) current_client;

    // Handle Request
    // Should send confirm message to client in a period. if get 0-length response, then close the connection
    while (true){
        string msg = mc_socket_read(mc_client->handler);
        cout << ">" << msg << endl;
        break;
    }
    //Finish Request
    close(mc_client->handler);
    mc_remove_client(mc_client);
}


//TEST METHODS



void printOnlineUsers(){
    int i = 0;
    client_structure * pointer = mc_user_list;
    cout << "--------------------------------------------" << endl;
    while (pointer->next != NULL) {
        pointer = pointer->next;
        cout << "user(" << ++i << ")"<< mc_socket_address_to_string(pointer->address) << endl;
    }
}