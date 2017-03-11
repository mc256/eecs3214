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
    cout << " Usage: " << string(argv[0]) << " [directory_server_ip_address] [bind_port]" << endl;
    cout << " Command:" << endl;
    cout << "    JOIN               - display yourself on the LIST" << endl;
    cout << "    LEAVE              - hide yourself from the LIST" << endl;
    cout << "    LIST               - list all the users" << endl;
    cout << "    CLOSE              - close this program" << endl;
    cout << "    BROADCAST message  - anything you want to send to all the other peers" << endl;
    cout << "    CONNECT id         - The peer you want to connect (use LIST to fetch the list first)" << endl;
    cout << "    message            - Sent message to the peer you have connected." << endl;
    cout << "========================================================" << endl;
    cout << " I am hosting the directory server on my VPS." << endl;
    cout << " You can try 106.185.43.242." << endl;
    cout << "========================================================" << endl;

    //Handle parameters and configurations
    in_addr_t remote_address = INADDR_NONE;
    if (argc >= 2) {
        remote_address = inet_addr(argv[1]);
    }
    while (remote_address == INADDR_NONE) {
        cout << "Please enter a valid directory server IP address:" << flush;   
        string input_string;
        getline(cin, input_string);
        remote_address = inet_addr(input_string.c_str());
    }
    
    mc_peer_port = 0;
    if (argc >= 3) {
        mc_peer_port = atoi(argv[2]);
    }
    while (mc_peer_port == 0) {
        cout << "Please enter a valid port number for the listening port:" << flush;   
        string input_string;
        getline(cin, input_string);
        mc_peer_port = atoi(input_string.c_str());        
    }


    //Connect to directory server 
    int server_connection;
    if ( (server_connection = mc_connect_to_server(remote_address, BIND_PORT)) < 0){
        cout << "FAILED!!! Cannot connect to directory server. Program exits.";
        return 1;
    }

    //Create server for other peers    
    int peer_connection;
    if ((peer_connection = mc_create_server(inet_addr(BIND_ADDR), mc_peer_port)) < 0){
        cout << "FAILED!!! Cannot create server for other peers. Program exits.";
        return 1;
    }

    //Peer List
    mc_peer_list = new peer_structure;
    mc_peer_list->id = 0;
    mc_peer_list->next = NULL;

    // Handle Request
    pthread_mutex_init(&mc_holding_mutex,NULL);
    mc_mutex_lock(&mc_holding_mutex);

    cout << "CONNECTED!!!" << endl;
    cout << "LISTENING ON PORT: " << mc_peer_port << endl;

    // User Interaction
    pthread_create(&mc_client_thread, NULL, mc_client_response, (void *) &server_connection);
    // Heart-beat
    pthread_create(&mc_heartbeat_thread, NULL, mc_heartbeat_response, (void *) &server_connection);




    /*
    =======================================
     Prepare peer-to-peer connection
    =======================================
    */


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
            ostringstream buffer;
            buffer << "JOIN" << mc_peer_port;
            mc_socket_write(*(int *)connection, buffer.str());            
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
        }else if (msg.substr(0,5) == "LIST\n"){
            //TODO: handle the list
            mc_update_peer_list(msg);
            cout << msg << ">" << flush;
            mc_mutex_unlock(&mc_holding_mutex);
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


void * mc_update_peer_list(string data){
    stringstream buffer;
    buffer << data;
    peer_structure * pointer = mc_peer_list;
    string msg;
    while (getline(buffer, msg, '\n')){
        int id,port; 
        char address[15];

        if (sscanf(msg.c_str(),"[%d] %15[0-9.]: %*d << %d", &id, address, &port) == 3){
            peer_structure * peer = new peer_structure;
            mc_check_null(peer);
            pointer->next = peer;
            pointer = pointer->next;
            pointer->id = id;
            pointer->ip_address = inet_addr(address);
            pointer->port = port;
        }
    }
}