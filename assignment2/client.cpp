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
    Main procedure.

    @param argc number of the arguments.
    @param argv the arguments.
*/
int main(int argc, char ** argv) {
    // Get the Server-end IP address
    cout << "===========================================================================" << endl;
    cout << "                     EECS3214 Assignment 2 Client" << endl;
    cout << "---------------------------------------------------------------------------" << endl;
    cout << " Usage: " << string(argv[0]) << " [directory_server_ip_address] [bind_port]" << endl;
    cout << " Command:" << endl;
    cout << "    JOIN               - display yourself on the LIST" << endl;
    cout << "    LEAVE              - hide yourself from the LIST" << endl;
    cout << "    LIST               - list all the users" << endl;
    cout << "    CLOSE              - close this program" << endl;
    cout << "    BROADCAST message  - anything you want to send to all the other peers." << endl;
    cout << "    CONNECT id         - Establish P2P connection." << endl;
    cout << "                         You must use LIST command to fetch the list first." << endl;
    cout << "                         The number of id is provided in the list." << endl;
    cout << "    DISCONNECT         - Close the P2P connection." << endl;
    cout << "    message            - Sent message to the peer you have connected." << endl;
    cout << "===========================================================================" << endl;
    cout << " I am hosting the directory server on my VPS." << endl;
    cout << " You can try 106.185.43.242." << endl;
    cout << "===========================================================================" << endl;

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
    if ( (server_connection = mc_connect_server(remote_address, BIND_PORT)) < 0){
        cout << "FAILED!!! Cannot connect to directory server. Program exits." << endl;
        return 1;
    }

    //Create server for other peers    
    if ( (mc_peer_connection = mc_create_server(inet_addr(BIND_ADDR), mc_peer_port)) < 0){
        cout << "FAILED!!! Cannot create server for other peers. Program exits." << endl;
        return 1;
    }

    //Peer List
    mc_peer_list = new peer_structure;
    mc_peer_list->id = 0;
    mc_peer_list->next = NULL;

    // Initialize MUTEX
    pthread_mutex_init(&mc_holding_mutex,NULL);
    pthread_mutex_init(&mc_peer_holding_mutex,NULL);
    pthread_mutex_init(&mc_p2p_mutex, NULL);
    mc_mutex_lock(&mc_holding_mutex);

    cout << "CONNECTED!!!" << endl;
    cout << "LISTENING ON PORT: " << mc_peer_port << endl;

    // Heart-beat
    pthread_create(&mc_heartbeat_thread, NULL, mc_heartbeat_response, (void *) &server_connection);
    // Peer connection
    pthread_create(&mc_peer_passive_thread, NULL, mc_peer_passive_response, (void *) &mc_peer_connection);
    // User Interaction
    pthread_create(&mc_client_thread, NULL, mc_client_response, (void *) &server_connection);

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

        if (msg == "CLOSE"){
            mc_mutex_lock(&mc_holding_mutex);
            break;
        }else if (msg == "JOIN"){
            mc_mutex_lock(&mc_holding_mutex);
            ostringstream buffer;
            buffer << "JOIN" << mc_peer_port;
            mc_socket_write(*(int *)connection, buffer.str());            
        }else if (msg.substr(0,7) == "CONNECT"){
            mc_connect_peer(atoi(msg.substr(7,string::npos).c_str()));
        }else if (msg == "LEAVE" || msg == "LIST"){
            mc_mutex_lock(&mc_holding_mutex);
            mc_socket_write(*(int *)connection, msg);
        }else if (msg.substr(0,10) == "BROADCAST "){
            mc_mutex_lock(&mc_holding_mutex);
            mc_socket_write(*(int *)connection, msg);
        }else if (msg == "DISCONNECT"){
            if (mc_peer_mode == 0) {
                cout << "You did not establish P2P connection." << "\n>" << flush;
            }else{
                mc_peer_mode = 0;
                cout << "Closing..." << "\n>" << flush;
            }
        }else{
            if (mc_peer_mode == 0) {
                cout << "You did not establish P2P connection." << "\n>" << flush;
            }else{
                mc_mutex_lock(&mc_peer_holding_mutex);
                mc_socket_write(mc_peer_handler, "MSG " + msg);
                cout << ">" << flush;
            }
        }            
        
    }
    close(*(int *)connection);
    if (mc_mutex_trylock(&mc_p2p_mutex)){
        close(mc_peer_handler);
    }
    close(mc_peer_connection);
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
        if (mc_mutex_trylock(&mc_holding_mutex)) {
            mc_socket_write(*(int *)connection, "IMOK");
        }
        
    }
    close(*(int *)connection);
    cout << ERROR_LOSTCNT << endl;
    exit(0);
}

/*
    Thread: P2P server thread
    This procedure waits for the new connection coming from the other peers.
    If it accept the connection from the peer, it will check the peer periodically.

    @param connection the socket file descriptor.
*/
void * mc_peer_passive_response(void * connection){

    while (true) {
        int handler;
        socket_address * remote_address = new socket_address;
        mc_check_null(&remote_address);
        socklen_t remote_address_size = sizeof(* remote_address);

        // Accept
        if ( (handler = accept(*(int *) connection, (socket_address_system *) remote_address, &remote_address_size)) < 0){
            cout << ERROR_ACCEPT << endl;
            break;
        }
        if (mc_mutex_trylock(&mc_p2p_mutex)){
            cout << "Accepted connection from " << mc_socket_address_to_string(remote_address) << ".\n>" << flush;
            mc_peer_handler = handler;
            mc_peer_mode = -1;
            //Keep Reading
            while (true){
                //Heart-beat request
                if (mc_mutex_trylock(&mc_peer_holding_mutex)){
                    mc_socket_write(mc_peer_handler, "RUOK");
                }

                string msg = mc_socket_read(mc_peer_handler);

                if (msg.length() == 0){
                    break;
                }else if (msg == "IMOK") {
                    // DO NOTHING
                }else if (msg.substr(0,3) == "MSG"){
                    cout << "P2P Message: " << msg.substr(4, string::npos) << "\n>" << flush;
                }

                mc_mutex_unlock(&mc_peer_holding_mutex);
                
                if (mc_peer_mode == 0){
                    break;
                }
                
                usleep(INTERVAL);
            }
            mc_mutex_unlock(&mc_p2p_mutex); 
            mc_mutex_unlock(&mc_peer_holding_mutex);
        }else{
            cout << ">ERROR!!! You cannot connect to yourself. Please try another peer." << "\n>" << flush;
        }
        close(mc_peer_handler);
        cout << "P2P connection closed. (S)" << "\n>" << flush;
        mc_peer_mode = 0;
    }
}


/*
    Thread: P2P client thread
    This procedure initiative creates the connection to another peer. 

    @param connection the socket file descriptor.
*/
void * mc_peer_active_response(void * connection){
    cout << ">" << flush;
    while (true){
        //TODO
        if (mc_peer_mode == 0){
            break;
        }

        // Read from client
        string msg = mc_socket_read(mc_peer_handler);

        // Handle response
        if (msg.length() == 0){
            break;
        }else if (msg == "RUOK"){
            mc_mutex_unlock(&mc_peer_holding_mutex);
            usleep(INTERVAL);
        }else if (msg.substr(0,3) == "MSG"){
            cout << "P2P Message: " << msg.substr(4, string::npos) << "\n>" << flush;
            mc_mutex_unlock(&mc_peer_holding_mutex);
        }

        // Respond if no others respond      
        if (mc_mutex_trylock(&mc_peer_holding_mutex)) {
            mc_socket_write(mc_peer_handler, "IMOK");
        }
    }
    close(mc_peer_handler);
    mc_mutex_unlock(&mc_p2p_mutex);
    mc_mutex_unlock(&mc_peer_holding_mutex);
    cout << "P2P connection closed. (C)" << "\n>" << flush;
    mc_peer_mode = 0;
}


/*
    Update the list of all the peers.
    This procedure handles the result from the LIST command.

    @param data the respond data that starts with the "LIST" identifier.
*/
void mc_update_peer_list(string data){
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
            pointer->next = NULL;
        }
    }
}


/*
    Establish P2P connection
    This procedure tries to connect to specific peer by looking the peer list store in the program.
    Once the connection has been established, this procedure will pass the connection descriptor to 
    the mc_peer_active_response() thread.

    @param data the respond data that starts with the "LIST" identifier.
*/
void mc_connect_peer(int id){
    // id > 0
    if (mc_mutex_trylock(&mc_p2p_mutex)){
        if (id <= 0){
            cout << "ID should be greater than 0." << "\n>" << flush;
            mc_mutex_unlock(&mc_p2p_mutex);
            return;
        }

        if (mc_peer_list->next == NULL){
            cout << "Please use LIST command to obtain the list of peers from directory server." << "\n>" << flush;
            mc_mutex_unlock(&mc_p2p_mutex);
            return;
        }

        peer_structure * pointer = mc_peer_list;
        while (pointer->next != NULL){
            pointer = pointer->next;
            if (pointer->id == id){
                //Try to connect to the peer
                int connection;
                if ((connection = mc_connect_server(pointer->ip_address, pointer->port)) < 0){
                    cout << "Cannot establish P2P connection. Please try again." << "\n>" <<endl;
                }
                mc_peer_mode = 1;
                mc_peer_handler = connection;
                mc_mutex_lock(&mc_peer_holding_mutex);
                pthread_create(&mc_peer_active_thread, NULL, mc_peer_active_response, (void *) &connection);
                return;
            }
        }
        cout << "Cannot find the peer with ID-" << id << ". Please try again." << "\n>" << flush;
        mc_mutex_unlock(&mc_p2p_mutex);
    }else{
        cout << "You have already established P2P connection." << "\n>" << flush;
    }

}