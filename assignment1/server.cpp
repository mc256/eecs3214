//
//  server.cpp
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include "server.hpp"

using namespace std;

int main(int argc, char ** argv) {
    //Print the menu
    cout << "===============================================" << endl;
    cout << "       EECS3214 Assignment 1 Server" << endl;
    cout << "-----------------------------------------------" << endl;
    cout << " Command:" << endl;
    cout << "    LIST    - display yourself on the LIST" << endl;
    cout << "    CLOSE   - close this program" << endl;
    cout << "    message - anything you want to send" << endl;
    cout << "===============================================" << endl;

    //Configure the address
    socket_address * mc_address = new socket_address;
    mc_check_null(&mc_address);
    mc_address->sin_family = AF_INET;                            // IPv4 is OK
    mc_address->sin_addr.s_addr = inet_addr(BIND_ADDR);          // Address
    mc_address->sin_port = htons(BIND_PORT);                     // Port
    socklen_t mc_address_size = sizeof(* mc_address);            // size of the listen address information

    //Create socket
    int mc_connection;
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
    pthread_mutex_init(&mc_user_list_mutex, NULL);
    mc_user_list = new client_structure;
    mc_check_null(&mc_user_list);
    mc_user_list->address = NULL;
    mc_user_list->next = mc_user_list->prev = NULL;
    

    //Server-end Menu
    cout << "OK!!! Server is ready." << endl;
    pthread_create(&mc_server_menu_thread, NULL, mc_server_menu, (void *) &mc_connection);
    pthread_create(&mc_listener_thread, NULL, mc_listener, (void *) &mc_connection);

    pthread_join(mc_server_menu_thread, NULL);

    return 0;
}

void * mc_listener(void * connection){
    while (true) {
        int mc_handler;
        socket_address * mc_remote_address = new socket_address;
        mc_check_null(&mc_remote_address);
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
    while (true){
        string msg;
        getline(cin, msg);
        if (msg.length() == 0){
            continue;
        }
        if (msg == "LIST"){
            cout << mc_list_clients(true) << endl;
        }else if (msg == "CLOSE"){
            mc_spread_out_message(mc_user_list, "System Announcement >> System is shutting down.");
            cout << "Sending notifications to clients..." << endl;
            sleep(SHUTDOWN_TIMEOUT);
            break;
        }else{
            mc_spread_out_message(mc_user_list, "System Announcement >> " + msg);
        }
    }

    //Remove All the connected Client
    mc_mutex_lock(&mc_user_list_mutex);

    client_structure * pointer = mc_user_list;
    while (pointer->next != NULL){
        pointer = pointer->next;
        close(pointer->handler);
    }

    mc_mutex_unlock(&mc_user_list_mutex);

    close(*(int *) connection);
    cout << "Bye!" << endl;
}


int mc_create_client(socket_address * address, int handler){
    //Init
    client_structure * mc_client = new client_structure;
    mc_check_null(&mc_client);
    mc_client->address = address;
    mc_client->handler = handler;
    mc_client->active = false;
    mc_client->message_buffer = "";
    pthread_mutex_init(&(mc_client->message_mutex),NULL);

    //Add user to linked list
    mc_mutex_lock(&mc_user_list_mutex);

    mc_client->next = mc_user_list->next;
    mc_client->prev = mc_user_list;
    if (mc_user_list->next != NULL){
        mc_user_list->next->prev = mc_client;
    }
    mc_user_list->next = mc_client;

    mc_mutex_unlock(&mc_user_list_mutex);


    //Process
    return pthread_create(&(mc_client->thread), NULL, mc_message_handler, (void *) mc_client);
}

int mc_remove_client(client_structure * current_client){
    mc_mutex_lock(&mc_user_list_mutex);
    
    if (current_client->next != NULL){
        current_client->next->prev = current_client->prev;
    }
    current_client->prev->next = current_client->next;

    mc_mutex_unlock(&mc_user_list_mutex);

    current_client->prev = current_client->next = NULL;

    delete current_client->address;
    delete current_client;

    return 0;
}

string mc_list_clients(bool show_unactive){
    ostringstream buffer;
    int i = 0;
    client_structure * pointer = mc_user_list;
    while (pointer->next != NULL) {
        pointer = pointer->next;
        if (show_unactive || pointer->active){
            buffer << "[" << ++i << "] "<< mc_socket_address_to_string(pointer->address);
                if (show_unactive) {
                    buffer << " " << (pointer->active ? "ACTIVE" : "UNACTIVE" );
                }
            buffer << endl;
        }
    }
    buffer << i << " user(s) online." << endl;
    return buffer.str();
}

void * mc_message_handler(void * current_client){
    client_structure * mc_client = (client_structure *) current_client;
    cout << "Connected   : " << mc_socket_address_to_string(mc_client->address) << endl;
    // Handle Request
    while (true){
        if (mc_client->message_buffer.length() == 0){
            mc_socket_write(mc_client->handler, "RUOK");
        }else{
            mc_mutex_lock(&(mc_client->message_mutex));            
            mc_socket_write(mc_client->handler, mc_client->message_buffer);
            mc_client->message_buffer = "";
            cout << "Message Sent: " << mc_socket_address_to_string(mc_client->address) << endl;
            mc_mutex_unlock(&(mc_client->message_mutex));
        }
            

        string msg = mc_socket_read(mc_client->handler);
        if (msg.length() == 0 || msg == "CLOSE"){
            break;
        }else if (msg == "IMOK"){
            continue;
        }else if (msg == "JOIN"){
            mc_client->active = true;
            mc_mutex_lock(&(mc_client->message_mutex));
            mc_client->message_buffer += "JOINED FROM " + mc_socket_address_to_string(mc_client->address) + "\n";
            mc_mutex_unlock(&(mc_client->message_mutex));
        }else if (msg == "LEAVE"){
            mc_client->active = false;
            mc_mutex_lock(&(mc_client->message_mutex));
            mc_client->message_buffer += "LEFT\n";
            mc_mutex_unlock(&(mc_client->message_mutex));
        }else if (msg == "LIST"){
            mc_mutex_lock(&(mc_client->message_mutex));
            mc_client->message_buffer += mc_list_clients(false);
            mc_mutex_unlock(&(mc_client->message_mutex));
        }else if (msg.substr(0,3) == "MSG"){
            mc_spread_out_message(mc_client, msg.substr(3,msg.length() - 3));
            mc_mutex_lock(&(mc_client->message_mutex));
            mc_client->message_buffer += "MESSAGE SENT\n";
            mc_mutex_unlock(&(mc_client->message_mutex));
        }
    }
    //Finish Request
    cout << "Disconnected: " << mc_socket_address_to_string(mc_client->address) << endl;
    close(mc_client->handler);
    mc_remove_client(mc_client);
}

void mc_spread_out_message(client_structure * current_client, string message){
    ostringstream buffer;
    //Lock the user list
    if (current_client->address != NULL) {
        buffer << mc_socket_address_to_string(current_client->address) << " >> ";
    }
    buffer << message << endl;

    mc_mutex_lock(&mc_user_list_mutex);
    
    client_structure * pointer;

    // go forward
    pointer = current_client;
    while ((pointer->prev != NULL) && (pointer->prev->address != NULL)){
        pointer = pointer->prev;
        mc_mutex_lock(&(pointer->message_mutex));
        pointer->message_buffer += buffer.str();
        mc_mutex_unlock(&(pointer->message_mutex));
    }

    // go backward
    pointer = current_client;
    while (pointer->next != NULL){
        pointer = pointer->next;
        mc_mutex_lock(&(pointer->message_mutex));        
        pointer->message_buffer += buffer.str();
        mc_mutex_unlock(&(pointer->message_mutex));
    }

    mc_mutex_unlock(&mc_user_list_mutex);
}

