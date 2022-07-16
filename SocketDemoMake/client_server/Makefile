GPP := g++-10 -std=c++17 -Wall -O3
FLAGS := -lpthread

build := ./build
src  := ../../src
os_socket := $(src)/os_socket
client := $(src)/Client
server := $(src)/Server
launcher := $(src)/Launcher

program_exe := $(build)/client_server

os_socket_include_h := $(os_socket)/os_socket_include.hpp

os_socket_server_h := $(os_socket)/os_socket_server.hpp
os_socket_server_h += $(os_socket_include_h)

os_socket_client_h := $(os_socket)/os_socket_client.hpp
os_socket_client_h += $(os_socket_include_h)

server_h := $(server)/SocketServer.hpp

client_h := $(client)/SocketClient.hpp


main_c       := $(launcher)/launcher_main.cpp
main_o       := $(build)/main.o
object_files := $(main_o)

server_c := $(server)/SocketServer.cpp
server_o := $(build)/server.o
object_files += $(server_o)

client_c := $(client)/SocketClient.cpp
client_o := $(build)/client.o
object_files += $(client_o)


$(main_o): $(main_c) $(server_h) $(client_h)
	@echo "\n main"
	$(GPP) -o $@ -c $<

$(server_o): $(server_c) $(server_h) $(os_socket_server_h)
	@echo "\n server"
	$(GPP) -o $@ -c $<

$(client_o): $(client_c) $(client_h) $(os_socket_client_h)
	@echo "\n client"
	$(GPP) -o $@ -c $<


$(program_exe): $(object_files)
	@echo "\n client_server"
	$(GPP) -o $@ $+ $(FLAGS)


build: $(program_exe)

run: build
	$(program_exe)
	@echo "\n"


setup:
	mkdir -p $(build)
	@echo "\n"

clean:
	rm -rfv $(build)/*
	@echo "\n"