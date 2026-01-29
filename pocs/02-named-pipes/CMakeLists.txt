# POC 02: Named Pipes - IPC Replacement
# Validates Named Pipes as replacement for Unix Domain Sockets

add_executable(pipe_server pipe_server.c)
add_executable(pipe_client pipe_client.c)

# No additional libraries needed - Named Pipes are part of Windows SDK
