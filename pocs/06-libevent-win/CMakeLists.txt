# POC 06: libevent on Windows
# Validates libevent functionality on Windows

find_package(Libevent CONFIG REQUIRED)

add_executable(libevent_poc libevent_poc.c)

target_link_libraries(libevent_poc PRIVATE
    libevent::core
    libevent::extra
    ws2_32
    advapi32
)
