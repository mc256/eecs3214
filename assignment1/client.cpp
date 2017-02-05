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
    int mc_connection;
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
    while (true){
        int action;
        string message;


        cout << "action:(0-exit 1-send 2-recv)" << flush;
        cin >> action;
        if (action == 0){
            cout << "bye!" << endl;
            break;
        }else if (action == 1){
            cout << "send:" << flush;
            cin >> message;
            mc_socket_write(mc_connection, message);
        }else if (action == 2){
            message = mc_socket_read(mc_connection);
            cout << "recv:["<< message.length() <<"]" << message << endl;
        }
    }

    //Finish Request
    close(mc_connection);
    
    
    
    return 0;
}

