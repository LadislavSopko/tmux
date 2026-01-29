cmake_minimum_required(VERSION 3.20)
project(tmux-windows VERSION 3.5.0 LANGUAGES C)

# Windows 10+ required for ConPTY
set(CMAKE_SYSTEM_VERSION 10.0)

# C99 standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(TMUX_ENABLE_SIXEL "Enable Sixel image support" OFF)
option(TMUX_ENABLE_UTF8PROC "Enable UTF8PROC for Unicode" OFF)

# Find dependencies (vcpkg)
find_package(Libevent CONFIG REQUIRED)
find_package(unofficial-pdcurses CONFIG REQUIRED)

# tmux source directory (parent)
set(TMUX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Windows-specific sources (to be implemented)
set(WIN32_SOURCES
    src/osdep-win32.c
    src/pty-win32.c
    src/ipc-win32.c
    src/proc-win32.c
    src/signal-win32.c
    src/daemon-win32.c
    src/imsg-win32.c
)

# Core tmux sources (platform-independent)
set(TMUX_CORE_SOURCES
    ${TMUX_SRC_DIR}/alerts.c
    ${TMUX_SRC_DIR}/arguments.c
    ${TMUX_SRC_DIR}/attributes.c
    ${TMUX_SRC_DIR}/cfg.c
    ${TMUX_SRC_DIR}/client.c
    ${TMUX_SRC_DIR}/cmd.c
    ${TMUX_SRC_DIR}/cmd-attach-session.c
    ${TMUX_SRC_DIR}/cmd-bind-key.c
    ${TMUX_SRC_DIR}/cmd-break-pane.c
    ${TMUX_SRC_DIR}/cmd-capture-pane.c
    ${TMUX_SRC_DIR}/cmd-choose-tree.c
    ${TMUX_SRC_DIR}/cmd-command-prompt.c
    ${TMUX_SRC_DIR}/cmd-confirm-before.c
    ${TMUX_SRC_DIR}/cmd-copy-mode.c
    ${TMUX_SRC_DIR}/cmd-detach-client.c
    ${TMUX_SRC_DIR}/cmd-display-menu.c
    ${TMUX_SRC_DIR}/cmd-display-message.c
    ${TMUX_SRC_DIR}/cmd-display-panes.c
    ${TMUX_SRC_DIR}/cmd-find.c
    ${TMUX_SRC_DIR}/cmd-find-window.c
    ${TMUX_SRC_DIR}/cmd-if-shell.c
    ${TMUX_SRC_DIR}/cmd-join-pane.c
    ${TMUX_SRC_DIR}/cmd-kill-pane.c
    ${TMUX_SRC_DIR}/cmd-kill-server.c
    ${TMUX_SRC_DIR}/cmd-kill-session.c
    ${TMUX_SRC_DIR}/cmd-kill-window.c
    ${TMUX_SRC_DIR}/cmd-list-buffers.c
    ${TMUX_SRC_DIR}/cmd-list-clients.c
    ${TMUX_SRC_DIR}/cmd-list-keys.c
    ${TMUX_SRC_DIR}/cmd-list-panes.c
    ${TMUX_SRC_DIR}/cmd-list-sessions.c
    ${TMUX_SRC_DIR}/cmd-list-windows.c
    ${TMUX_SRC_DIR}/cmd-load-buffer.c
    ${TMUX_SRC_DIR}/cmd-lock-server.c
    ${TMUX_SRC_DIR}/cmd-move-window.c
    ${TMUX_SRC_DIR}/cmd-new-session.c
    ${TMUX_SRC_DIR}/cmd-new-window.c
    ${TMUX_SRC_DIR}/cmd-paste-buffer.c
    ${TMUX_SRC_DIR}/cmd-pipe-pane.c
    ${TMUX_SRC_DIR}/cmd-queue.c
    ${TMUX_SRC_DIR}/cmd-refresh-client.c
    ${TMUX_SRC_DIR}/cmd-rename-session.c
    ${TMUX_SRC_DIR}/cmd-rename-window.c
    ${TMUX_SRC_DIR}/cmd-resize-pane.c
    ${TMUX_SRC_DIR}/cmd-resize-window.c
    ${TMUX_SRC_DIR}/cmd-respawn-pane.c
    ${TMUX_SRC_DIR}/cmd-respawn-window.c
    ${TMUX_SRC_DIR}/cmd-rotate-window.c
    ${TMUX_SRC_DIR}/cmd-run-shell.c
    ${TMUX_SRC_DIR}/cmd-save-buffer.c
    ${TMUX_SRC_DIR}/cmd-select-layout.c
    ${TMUX_SRC_DIR}/cmd-select-pane.c
    ${TMUX_SRC_DIR}/cmd-select-window.c
    ${TMUX_SRC_DIR}/cmd-send-keys.c
    ${TMUX_SRC_DIR}/cmd-server-access.c
    ${TMUX_SRC_DIR}/cmd-set-buffer.c
    ${TMUX_SRC_DIR}/cmd-set-environment.c
    ${TMUX_SRC_DIR}/cmd-set-option.c
    ${TMUX_SRC_DIR}/cmd-show-environment.c
    ${TMUX_SRC_DIR}/cmd-show-messages.c
    ${TMUX_SRC_DIR}/cmd-show-options.c
    ${TMUX_SRC_DIR}/cmd-show-prompt-history.c
    ${TMUX_SRC_DIR}/cmd-source-file.c
    ${TMUX_SRC_DIR}/cmd-split-window.c
    ${TMUX_SRC_DIR}/cmd-swap-pane.c
    ${TMUX_SRC_DIR}/cmd-swap-window.c
    ${TMUX_SRC_DIR}/cmd-switch-client.c
    ${TMUX_SRC_DIR}/cmd-unbind-key.c
    ${TMUX_SRC_DIR}/cmd-wait-for.c
    ${TMUX_SRC_DIR}/colour.c
    ${TMUX_SRC_DIR}/control.c
    ${TMUX_SRC_DIR}/control-notify.c
    ${TMUX_SRC_DIR}/environ.c
    ${TMUX_SRC_DIR}/file.c
    ${TMUX_SRC_DIR}/format.c
    ${TMUX_SRC_DIR}/format-draw.c
    ${TMUX_SRC_DIR}/grid.c
    ${TMUX_SRC_DIR}/grid-reader.c
    ${TMUX_SRC_DIR}/grid-view.c
    ${TMUX_SRC_DIR}/hyperlinks.c
    ${TMUX_SRC_DIR}/input.c
    ${TMUX_SRC_DIR}/input-keys.c
    ${TMUX_SRC_DIR}/job.c
    ${TMUX_SRC_DIR}/key-bindings.c
    ${TMUX_SRC_DIR}/key-string.c
    ${TMUX_SRC_DIR}/layout.c
    ${TMUX_SRC_DIR}/layout-custom.c
    ${TMUX_SRC_DIR}/layout-set.c
    ${TMUX_SRC_DIR}/log.c
    ${TMUX_SRC_DIR}/menu.c
    ${TMUX_SRC_DIR}/mode-tree.c
    ${TMUX_SRC_DIR}/names.c
    ${TMUX_SRC_DIR}/notify.c
    ${TMUX_SRC_DIR}/options.c
    ${TMUX_SRC_DIR}/options-table.c
    ${TMUX_SRC_DIR}/paste.c
    ${TMUX_SRC_DIR}/popup.c
    ${TMUX_SRC_DIR}/proc.c
    ${TMUX_SRC_DIR}/regsub.c
    ${TMUX_SRC_DIR}/resize.c
    ${TMUX_SRC_DIR}/screen.c
    ${TMUX_SRC_DIR}/screen-redraw.c
    ${TMUX_SRC_DIR}/screen-write.c
    ${TMUX_SRC_DIR}/server.c
    ${TMUX_SRC_DIR}/server-acl.c
    ${TMUX_SRC_DIR}/server-client.c
    ${TMUX_SRC_DIR}/server-fn.c
    ${TMUX_SRC_DIR}/session.c
    ${TMUX_SRC_DIR}/spawn.c
    ${TMUX_SRC_DIR}/status.c
    ${TMUX_SRC_DIR}/style.c
    ${TMUX_SRC_DIR}/tmux.c
    ${TMUX_SRC_DIR}/tty.c
    ${TMUX_SRC_DIR}/tty-acs.c
    ${TMUX_SRC_DIR}/tty-draw.c
    ${TMUX_SRC_DIR}/tty-features.c
    ${TMUX_SRC_DIR}/tty-keys.c
    ${TMUX_SRC_DIR}/tty-term.c
    ${TMUX_SRC_DIR}/utf8.c
    ${TMUX_SRC_DIR}/utf8-combined.c
    ${TMUX_SRC_DIR}/window.c
    ${TMUX_SRC_DIR}/window-buffer.c
    ${TMUX_SRC_DIR}/window-client.c
    ${TMUX_SRC_DIR}/window-clock.c
    ${TMUX_SRC_DIR}/window-copy.c
    ${TMUX_SRC_DIR}/window-customize.c
    ${TMUX_SRC_DIR}/window-tree.c
    ${TMUX_SRC_DIR}/xmalloc.c
)

# Compat sources (portable utilities)
set(COMPAT_SOURCES
    ${TMUX_SRC_DIR}/compat/strlcpy.c
    ${TMUX_SRC_DIR}/compat/strlcat.c
    ${TMUX_SRC_DIR}/compat/strndup.c
    ${TMUX_SRC_DIR}/compat/strsep.c
    ${TMUX_SRC_DIR}/compat/asprintf.c
    ${TMUX_SRC_DIR}/compat/err.c
    ${TMUX_SRC_DIR}/compat/getopt_long.c
    ${TMUX_SRC_DIR}/compat/reallocarray.c
    ${TMUX_SRC_DIR}/compat/recallocarray.c
    ${TMUX_SRC_DIR}/compat/explicit_bzero.c
    ${TMUX_SRC_DIR}/compat/freezero.c
    ${TMUX_SRC_DIR}/compat/vis.c
    ${TMUX_SRC_DIR}/compat/unvis.c
    ${TMUX_SRC_DIR}/compat/base64.c
    ${TMUX_SRC_DIR}/compat/strtonum.c
    ${TMUX_SRC_DIR}/compat/getprogname.c
    ${TMUX_SRC_DIR}/compat/setenv.c
)

# Optional: Sixel support
if(TMUX_ENABLE_SIXEL)
    list(APPEND TMUX_CORE_SOURCES
        ${TMUX_SRC_DIR}/image.c
        ${TMUX_SRC_DIR}/image-sixel.c
    )
    add_definitions(-DENABLE_SIXEL)
endif()

# Create executable
add_executable(tmux
    ${WIN32_SOURCES}
    ${TMUX_CORE_SOURCES}
    ${COMPAT_SOURCES}
)

# Include directories
target_include_directories(tmux PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${TMUX_SRC_DIR}
    ${TMUX_SRC_DIR}/compat
)

# Compile definitions
target_compile_definitions(tmux PRIVATE
    _WIN32
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    TMUX_VERSION="${PROJECT_VERSION}"
    TMUX_CONF="~/.tmux.conf"
)

# Link libraries
target_link_libraries(tmux PRIVATE
    libevent::core
    libevent::extra
    unofficial::pdcurses::pdcurses
    kernel32
    advapi32
    user32
    ws2_32
)

# MSVC-specific settings
if(MSVC)
    # Force include compat-win32.h before any other header
    set(COMPAT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/compat-win32.h")
    target_compile_options(tmux PRIVATE
        /W3
        /wd4996
        /wd4005
        /wd4273
        "SHELL:/FI ${COMPAT_HEADER}"
    )
else()
    target_compile_options(tmux PRIVATE -Wall -Wextra -include compat-win32.h)
endif()

# Install
install(TARGETS tmux RUNTIME DESTINATION bin)
