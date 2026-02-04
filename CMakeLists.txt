cmake_minimum_required(VERSION 3.20)
project(MyOS C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# ========================
# Load .config
# ========================
if(EXISTS "${CMAKE_SOURCE_DIR}/config/.config")
    file(STRINGS "config/.config" CONFIG_LINES)
    foreach(line ${CONFIG_LINES})
        if(line MATCHES "^CONFIG_")
            string(REPLACE "=" ";" kv ${line})
            list(GET kv 0 key)
            list(GET kv 1 val)
            add_compile_definitions(${key}=${val})
        endif()
    endforeach()
else()
    message(FATAL_ERROR "No config/.config found. Run make menuconfig")
endif()

# ========================
# Subsystems
# ========================
add_subdirectory(core)
add_subdirectory(boot)
