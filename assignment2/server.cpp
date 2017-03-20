//
//  server.cpp
//  eecs3214a2
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include "server.hpp"

using namespace std;

/*
    Main procedure

    @param argc number of the arguments.
    @param argv the arguments.
*/
int main(int argc, char ** argv) {
    // Print the menu
    cout << "===============================================" << endl;
    cout << "       EECS3214 Assignment 2 Server" << endl;
    cout << "-----------------------------------------------" << endl;
    cout << " Command:" << endl;
    cout << "    LIST    - display yourself on the LIST" << endl;
    cout << "    CLOSE   - close this program" << endl;
    cout << "    message - anything you want to send" << endl;
    cout << "===============================================" << endl;

    int connection;
    if ((connection = mc_create_server(inet_addr(BIND_ADDR), BIND_PORT)) < 0){
        cout << "FAILED!!! Cannot create server. Program exits." << endl;
        return 1;
    }

    // User List
    pthread_mutex_init(&mc_user_list_mutex, NULL);
    mc_user_list = new client_structure;
    mc_check_null(&mc_user_list);
    mc_user_list->address = NULL;
    mc_user_list->next = mc_user_list->prev = NULL;
    

    // Server-end Menu
    cout << "OK!!! Server is ready." << endl;
    pthread_create(&mc_server_menu_thread, NULL, mc_server_menu, (void *) &connection);
    pthread_create(&mc_listener_thread, NULL, mc_listener, (void *) &connection);

    pthread_join(mc_server_menu_thread, NULL);

    return 0;
}

/*
    Thread: Listener
    This thread waits for new connections. And it create new thread for new connection.

    @param connection the socket file descriptor.
*/
void * mc_listener(void * connection){
    while (true) {
        int mc_handler;
        socket_address * mc_remote_address = new socket_address;
        mc_check_null(&mc_remote_address);
        socklen_t mc_remote_address_size = sizeof(* mc_remote_address);

        // Accept
        if ( (mc_handler = accept(*(int *) connection, (socket_address_system *) mc_remote_address, &mc_remote_address_size)) < 0){
            cout << ERROR_ACCEPT << endl;
            break;
        }

        // Create new thread for Handling
        if ( mc_create_client(mc_remote_address, mc_handler) ){
            cout << ERROR_SERVER << endl;
            continue;
        }
    }
}

/*
    Thread: User interactions
    This thread handles the user input. And it sends then message to the server if it obtains the lock.

    @param connection the socket file descriptor.
*/
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

    // Remove All the connected Client
    mc_mutex_lock(&mc_user_list_mutex);

    client_structure * pointer = mc_user_list;
    while (pointer->next != NULL){
        pointer = pointer->next;
        close(pointer->handler);
    }

    mc_mutex_unlock(&mc_user_list_mutex);

    // Close
    close(*(int *) connection);
    cout << "Bye!" << endl;
}

/*
    Create new user structure
    This function initializes the user structure for new connection and then put the client structure into the user list.

    @param address the remote address of the connection.
    @param handler the connection file descriptor.
*/
int mc_create_client(socket_address * address, int handler){
    // Initialize
    client_structure * mc_client = new client_structure;
    mc_check_null(&mc_client);
    mc_client->address = address;
    mc_client->handler = handler;
    mc_client->active = false;
    mc_client->message_buffer = "";
    pthread_mutex_init(&(mc_client->message_mutex),NULL);

    // Add user to linked list
    mc_mutex_lock(&mc_user_list_mutex);

    mc_client->next = mc_user_list->next;
    mc_client->prev = mc_user_list;
    if (mc_user_list->next != NULL){
        mc_user_list->next->prev = mc_client;
    }
    mc_user_list->next = mc_client;

    mc_mutex_unlock(&mc_user_list_mutex);


    // Process
    return pthread_create(&(mc_client->thread), NULL, mc_message_handler, (void *) mc_client);
}

/*
    Remove user from the user list
    This function remove the user structure from the user list. It will NOT close the connection.

    @param current_client the client structure.
*/
int mc_remove_client(client_structure * current_client){
    // Update user list
    mc_mutex_lock(&mc_user_list_mutex);
    
    if (current_client->next != NULL){
        current_client->next->prev = current_client->prev;
    }
    current_client->prev->next = current_client->next;

    mc_mutex_unlock(&mc_user_list_mutex);

    // Release the object itself
    current_client->prev = current_client->next = NULL;

    delete current_client->address;
    delete current_client;

    return 0;
}

/*
    List all the users
    This function will list all the user and put it into a string.

    @param show_inactive true if you want to show all the users including inactive user
*/
string mc_list_clients(bool show_inactive){
    ostringstream buffer;
    int i = 0;
    client_structure * pointer = mc_user_list;
    buffer << "LIST" << endl;
    while (pointer->next != NULL) {
        pointer = pointer->next;
        if (show_inactive || pointer->active){
            buffer << "[" << ++i << "] "<< mc_socket_address_to_string(pointer->address);
            if (show_inactive) {
                buffer << " " << (pointer->active ? "ACTIVE" : "INACTIVE" );
            }
            if (pointer->active){
                buffer << " << "<< pointer->listening_port;
            }
            buffer << endl;
        }
    }
    buffer << i << " on-line user(s)." << endl;
    return buffer.str();
}

 
/*
    Thread: handle the connected clients
    This Thread responds to the server periodically. Therefore the program can switch between read-mode and write-mode.
    The JOIN and LEAVE command will change the "active" state in the user structure.
    All the message will be save in the "message_buffer" in the user structure. And the access will be managed by the "message_mutex".

    @param current_client user structure that contains all the message we need for handling the connection.
*/
void * mc_message_handler(void * current_client){
    client_structure * mc_client = (client_structure *) current_client;
    cout << "Connected   : " << mc_socket_address_to_string(mc_client->address) << endl;
    // Handle Request
    while (true){
        // Write to client
        // If the buffer does not have message, just simply ask the client if it is okay.
        // If the buffer dose have message, send the message to the client.
        if (mc_client->message_buffer.length() == 0){
            mc_socket_write(mc_client->handler, "RUOK");
        }else{
            // Avoid read and write the buffer at the same time.
            mc_mutex_lock(&(mc_client->message_mutex));            
            mc_socket_write(mc_client->handler, mc_client->message_buffer);
            mc_client->message_buffer = "";
            cout << "Message Sent: " << mc_socket_address_to_string(mc_client->address) << endl;
            mc_mutex_unlock(&(mc_client->message_mutex));
        }
            
        // Read from client
        string msg = mc_socket_read(mc_client->handler);
        if (msg.length() == 0 || msg == "CLOSE"){
            break;
        }else if (msg == "IMOK"){
            continue;
        }else if (msg.substr(0,4) == "JOIN"){
            mc_client->active = true;
            // Avoid read and write the buffer at the same time.
            mc_mutex_lock(&(mc_client->message_mutex));
            string port_number_str = msg.substr(4,string::npos);
            mc_client->listening_port = atoi(port_number_str.c_str());
            mc_client->message_buffer += "JOINED FROM " + mc_socket_address_to_string(mc_client->address) + " LISTENING ON PORT " + port_number_str + "\n";
            mc_mutex_unlock(&(mc_client->message_mutex));
        }else if (msg == "LEAVE"){
            mc_client->active = false;
            // Avoid read and write the buffer at the same time.
            mc_mutex_lock(&(mc_client->message_mutex));
            mc_client->message_buffer += "LEFT\n";
            mc_mutex_unlock(&(mc_client->message_mutex));
        }else if (msg == "LIST"){
            // Avoid read and write the buffer at the same time.
            mc_mutex_lock(&(mc_client->message_mutex));
            mc_client->message_buffer += mc_list_clients(false);
            mc_mutex_unlock(&(mc_client->message_mutex));
        }else if (msg.substr(0,10) == "BROADCAST "){
            mc_spread_out_message(mc_client, msg.substr(10, string::npos));
            // Avoid read and write the buffer at the same time.
            mc_mutex_lock(&(mc_client->message_mutex));
            mc_client->message_buffer += "MESSAGE SENT\n";
            mc_mutex_unlock(&(mc_client->message_mutex));
        }
    }
    // Finish Request
    cout << "Disconnected: " << mc_socket_address_to_string(mc_client->address) << endl;
    close(mc_client->handler);
    mc_remove_client(mc_client);
}

/*
    Write message to other clients' buffer
    This function will iterating through the user list by following the "prev" and "next" pointer.
    While iterating, this function will append the message to the message_buffer.

    @param current_client user structure that in the user list.
    @param message message you want to send.
*/
void mc_spread_out_message(client_structure * current_client, string message){
    ostringstream buffer;
    // Lock the user list
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
        // Avoid read and write the buffer at the same time.
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

