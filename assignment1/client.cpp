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

    //Configure the address
    socket_address * mc_address = new socket_address;
    mc_address->sin_family = AF_INET;                            // IPv4 is OK
    mc_address->sin_addr.s_addr = htonl(INADDR_ANY);             // Address
    mc_address->sin_port = htons(PORT_NUMBER);                   // Port
    socklen_t mc_address_size = sizeof(*mc_address);             // size of the listen address information


    //Create socket
    if ( (mc_connection = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << "socket create failed" << endl;
        return 1;
    }
    cout << "socket create success" << endl;


    //Connect
    if ( connect(mc_connection, (socket_address_system *) mc_address, mc_address_size) != 0){
        cout << "connect failed" << endl;   
        return 1;
    }
    cout << "connected" << endl;


    //Handle Request
    if (pthread_mutex_lock(&mc_holding_mutex)){    // Blocking until the greeting from the server
        cout << ERROR_THREAD << endl;
        return 0;
    }

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
        if (msg == "CLOSE"){
            break;
        }else{
            if (pthread_mutex_lock(&mc_holding_mutex)) {
                cout << ERROR_THREAD << endl;
                break;
            }
            mc_socket_write(mc_connection, msg);
        }

    }
    close(mc_connection);
    cout << ERROR_LOSTCNT << endl;
}


void * mc_heartbeat_response(void * connection){

    cout << ">" << flush;
    while (true){
        string msg = mc_socket_read(mc_connection);

        //UNSET HOLDING        
        if (pthread_mutex_unlock(&mc_holding_mutex)){    // Open a window so the message can be sent
            cout << ERROR_THREAD << endl;
            break;
        }

        //RESPOND HEART BEATS
        if (msg.length() == 0){
            break;
        }else if (msg == "RUOK"){
            usleep(INTERVAL);
        }else{
            cout << msg << ">" << flush;
        }


        //SET HOLDING        
        if (pthread_mutex_trylock(&mc_holding_mutex) == 0) {
            mc_socket_write(mc_connection, "IMOK");
        }
        
    }
    close(mc_connection);
    cout << ERROR_LOSTCNT << endl;
    //TODO
    //Ask user if you want to reconnect then a loop
}
