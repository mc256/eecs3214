//
//  client.cpp
//  eecs3214a1
//
//  Created by Jun Lin Chen on 2017/2/2.
//  Copyright 2017 masterchan.me. All rights reserved.
//

#include "client.hpp"

using namespace std;

int main(int argc, char ** argv) {
    cout << "=============================================================================" << endl;
    cout << "                         EECS3214 Assignment 1 Client" << endl;
    cout << "-----------------------------------------------------------------------------" << endl;
    cout << "By Jun Lin Chen" << endl;
    cout << "" << endl;
    cout << "Command:" << endl;
    cout << "    JOIN    - display yourself on the LIST" << endl;
    cout << "    LEAVE   - hide yourself from the LIST" << endl;
    cout << "    LIST    - list all the users" << endl;
    cout << "    CLOSE   - close this program" << endl;
    cout << "    message - anything you want to send" << endl;
    cout << "=============================================================================" << endl;
    cout << "I am hosting the server-end program on my server. You may try 106.185.43.242." << endl;
    cout << "Please enter the server IP address:" << flush;
    string bind_address;
    getline(cin, bind_address);

    //Configure the address
    socket_address * mc_address = new socket_address;
    mc_check_null(&mc_address);
    mc_address->sin_family = AF_INET;
    mc_address->sin_addr.s_addr = inet_addr(bind_address.c_str());
    mc_address->sin_port = htons(PORT_NUMBER);
    socklen_t mc_address_size = sizeof(* mc_address);


    //Create socket
    int mc_connection;
    if ( (mc_connection = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << ERROR_SOCKET << endl;
        return 1;
    }


    //Connect
    if ( connect(mc_connection, (socket_address_system *) mc_address, mc_address_size) != 0){
        cout << ERROR_ACCEPT << endl;   
        return 1;
    }


    //Handle Request
    pthread_mutex_init(&mc_holding_mutex,NULL);
    mc_mutex_lock(&mc_holding_mutex);

    cout << "CONNECTED!!!" << endl;
    
    //Menu
    pthread_create(&mc_client_thread, NULL, mc_client_response, (void *) &mc_connection);
    pthread_create(&mc_heartbeat_thread, NULL, mc_heartbeat_response, (void *) &mc_connection);

    pthread_join(mc_client_thread, NULL);        
    return 0;
}

void * mc_client_response(void * connection){

    while (true){
        string msg;
        getline(cin,msg);
        mc_mutex_lock(&mc_holding_mutex);
        if (msg == "CLOSE"){
            break;
        }else if ( msg == "JOIN" || msg == "LEAVE" || msg == "LIST" ){
            mc_socket_write(*(int *)connection, msg);
        }else{
            mc_socket_write(*(int *)connection, "MSG" + msg);
        }            
        
    }
    close(*(int *)connection);
    cout << "Bye!" << endl;
    exit(0);
}


void * mc_heartbeat_response(void * connection){

    cout << ">" << flush;
    while (true){
        string msg = mc_socket_read(*(int *)connection);

        mc_mutex_unlock(&mc_holding_mutex);
        //RESPOND HEART BEATS
        if (msg.length() == 0){
            break;
        }else if (msg == "RUOK"){
            usleep(INTERVAL);
        }else{
            cout << msg << ">" << flush;
        }

        //SET HOLDING        
        if (mc_mutex_trylock(&mc_holding_mutex) == 0) {
            mc_socket_write(*(int *)connection, "IMOK");
        }
        
    }
    close(*(int *)connection);
    cout << ERROR_LOSTCNT << endl;
    exit(0);
}
