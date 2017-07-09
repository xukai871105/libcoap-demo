cmake_minimum_required(VERSION 2.6)
project("simple-server")

# coap-1 why???
set(libs
    coap-1
)

set(targets
    simple-server
)

add_definitions(-DWITH_POSIX)
add_executable(simple-server simple-server.c)
target_link_libraries(simple-server ${libs})
