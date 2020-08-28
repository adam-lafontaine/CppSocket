#!/bin/bash
# $ chmod 755 compile.sh

flags="-Wall -O0"
libraries="-lpthread"
std="-std=c++17"

cpp_files="launcher_main.cpp ../Server/SocketServer.cpp ../Client/SocketClient.cpp"

exe="launcher"

g++ -o $exe $flags $cpp_files $std $libraries