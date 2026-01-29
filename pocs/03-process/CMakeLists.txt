# POC 03: Process Management
# Validates CreateProcess as replacement for fork()+exec()

add_executable(process_poc process_poc.c)

# No additional libraries needed - Process APIs are part of Windows SDK
