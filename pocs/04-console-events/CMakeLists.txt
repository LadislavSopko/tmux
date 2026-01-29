# POC 04: Console Events - Signal Replacement
# Validates console event handling as replacement for POSIX signals

add_executable(events_poc events_poc.c)

# No additional libraries needed - Console APIs are part of Windows SDK
