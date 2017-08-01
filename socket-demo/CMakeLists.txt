cmake_minimum_required(VERSION 2.6)
project("test")

set(targets
	server
	client
)

add_executable(server server.c)
add_executable(client client.c)

