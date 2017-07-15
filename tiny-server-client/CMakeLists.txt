cmake_minimum_required(VERSION 2.6)
project("tiny-coap-demo")

# coap-1 why???
set(libs coap-1)

#set(targets simple-server)

add_definitions(-DWITH_POSIX)
add_executable(server server.c)
target_link_libraries(server ${libs})

add_definitions(-DWITH_POSIX)
add_executable(client  client.c)
target_link_libraries(client ${libs})
