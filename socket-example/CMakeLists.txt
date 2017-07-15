cmake_minimum_required(VERSION 2.6)
project("test")

set(targets
    keyboard
    select-server
    client

)

add_executable(keyboard test_keyboard.c)
add_executable(select-server test_select_server.c)
add_executable(server test_server.c)
add_executable(client test_client.c)
