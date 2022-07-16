GPP := g++-10 -std=c++17 -Wall -O3

build := ./build
src  := ../../src
os_socket := $(src)/os_socket

server_exe := $(build)/server
client_exe := $(build)/client

os_socket_include_h := $(os_socket)/os_socket_include.hpp

os_socket_server_h := $(os_socket)/os_socket_server.hpp
os_socket_server_h += $(os_socket_include_h)

os_socket_client_h := $(os_socket)/os_socket_client.hpp
os_socket_client_h += $(os_socket_include_h)


server_main_c       := $(src)/server_main.cpp
server_main_o       := $(build)/server_main.o
server_object_files := $(server_main_o)

client_main_c       := $(src)/client_main.cpp
client_main_o       := $(build)/client_main.o
client_object_files := $(client_main_o)


$(client_main_o): $(client_main_c) $(os_socket_client_h)
	@echo "\n client_main"
	$(GPP) -o $@ -c $<

$(client_exe): $(client_object_files)
	@echo "\n client"
	$(GPP) -o $@ $+


$(server_main_o): $(server_main_c) $(os_socket_server_h)
	@echo "\n server_main"
	$(GPP) -o $@ -c $<

$(server_exe): $(server_object_files)
	@echo "\n server"
	$(GPP) -o $@ $+


build_server: $(server_exe)

run_server: build_server
	$(server_exe)
	@echo "\n"


build_client: $(client_exe)

run_client: build_client
	$(client_exe)
	@echo "\n"


setup:
	mkdir -p $(build)
	@echo "\n"

clean:
	rm -rfv $(build)/*
	@echo "\n"