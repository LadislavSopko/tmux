# POC 01: ConPTY - Pseudo Console API
# Validates ConPTY as replacement for forkpty()

add_executable(conpty_poc conpty_poc.c)

# No additional libraries needed - ConPTY is part of Windows SDK
