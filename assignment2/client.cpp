//
//  client.cpp
//  eecs3214a2
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include "client.hpp"

using namespace std;

/*
    Main method.

    @param argc number of the arguments.
    @param argv the arguments.
*/
int main(int argc, char ** argv) {
    // Get the Server-end IP address
    cout << "========================================================" << endl;
    cout << "            EECS3214 Assignment 2 Client" << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << " Usage: " << string(argv[0]) << " [ip_address]" << endl;
    cout << " Command:" << endl;
    cout << "    JOIN               - display yourself on the LIST" << endl;
    cout << "    LEAVE              - hide yourself from the LIST" << endl;
    cout << "    LIST               - list all the users" << endl;
    cout << "    CLOSE              - close this program" << endl;
    cout << "    BROADCAST message  - anything you want to send to all the other peers" << endl;
    cout << "    CONNECT id         - The peer you want to connect (use LIST to fetch the list first)" << endl;
    cout << "    message            - Sent message to the peer you have connected." << endl;
    cout << "========================================================" << endl;
    cout << " I am hosting the server-end on my server." << endl;
    cout << " You can try 106.185.43.242." << endl;
    cout << "========================================================" << endl;
    in_addr_t bind_address = INADDR_NONE;
    if (argc == 2) {
        bind_address = inet_addr(argv[1]);
    }
    while (bind_address == INADDR_NONE) {
        cout << "Please enter a valid IP address:" << flush;   
        string bind_address_string;
        getline(cin, bind_address_string);
        bind_address = inet_addr(bind_address_string.c_str());
    }

    // Configure the address
    socket_address * mc_address = new socket_address;
    mc_check_null(&mc_address);
    mc_address->sin_family = AF_INET;
    mc_address->sin_addr.s_addr = bind_address;
    mc_address->sin_port = htons(PORT_NUMBER);
    socklen_t mc_address_size = sizeof(* mc_address);


    // Create socket
    int mc_connection;
    if ( (mc_connection = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << ERROR_SOCKET << endl;
        return 1;
    }


    // Connect
    if ( connect(mc_connection, (socket_address_system *) mc_address, mc_address_size) != 0){
        cout << ERROR_ACCEPT << endl;   
        return 1;
    }


    // Handle Request
    pthread_mutex_init(&mc_holding_mutex,NULL);
    mc_mutex_lock(&mc_holding_mutex);

    cout << "CONNECTED!!!" << endl;
    
    // User Interaction
    pthread_create(&mc_client_thread, NULL, mc_client_response, (void *) &mc_connection);
    // Heart-beat
    pthread_create(&mc_heartbeat_thread, NULL, mc_heartbeat_response, (void *) &mc_connection);

    // Finish
    pthread_join(mc_client_thread, NULL);        
    return 0;
}

/*
    Thread: User interaction
    This thread handles the user input. And it sends then message to the server if it obtains the lock.

    @param connection the socket file descriptor.
*/
void * mc_client_response(void * connection){

    while (true){
        // Read the command
        string msg;
        getline(cin,msg);

        // Wait for the gap and process the command
        mc_mutex_lock(&mc_holding_mutex);
        if (msg == "CLOSE"){
            break;
        }else if (msg == "JOIN"){
            //TODO: Should send the server the listening port.
            mc_socket_write(*(int *)connection, msg);            
        }else if (msg == "LEAVE" || msg == "LIST"){
            mc_socket_write(*(int *)connection, msg);
        }else{
            mc_socket_write(*(int *)connection, "MSG" + msg);
        }            
        
    }
    close(*(int *)connection);
    cout << "Bye!" << endl;
    exit(0);
}

/*
    Thread: Heart-beat thread
    This Thread responds to the server periodically. Therefore the program can switch between read-mode and write-mode.
    This thread will unlock the mutex lock which allows user to send command to the server.

    @param connection the socket file descriptor.
*/
void * mc_heartbeat_response(void * connection){

    cout << ">" << flush;
    while (true){
        // Read from client
        string msg = mc_socket_read(*(int *)connection);

        // Handle response
        if (msg.length() == 0){
            break;
        }else if (msg == "RUOK"){
            mc_mutex_unlock(&mc_holding_mutex);
            usleep(INTERVAL);
        }else{
            cout << msg << ">" << flush;
            mc_mutex_unlock(&mc_holding_mutex);
        }

        // Respond if no others respond      
        if (mc_mutex_trylock(&mc_holding_mutex) == 0) {
            mc_socket_write(*(int *)connection, "IMOK");
        }
        
    }
    close(*(int *)connection);
    cout << ERROR_LOSTCNT << endl;
    exit(0);
}
