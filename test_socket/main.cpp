//
//  main.cpp
//  test_socket
//
//  Created by caoxin on 16/4/20.
//  Copyright © 2016年 caoxin. All rights reserved.
//

#include <iostream>
#include "PCNetServer.h"
using namespace playcrab;
int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    PCNetServer* server = new PCNetServer();
    server->setAddress("127.0.0.1", 6969);
    server->connect();
    
    std::string s = "xxxxxxxx";
    server->pushMsg(s);
    while(1)
    {
        fprintf(stdout,"while\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1150));
    }
    return 0;
}
