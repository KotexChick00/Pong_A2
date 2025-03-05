cmake_minimum_required(VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
      "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(Pong_A2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Alias for vcpkg path
set(VCPKG_DIR "C:/Users/kotex/vcpkg")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
endif()

# Add executable
add_executable(Pong_A2 
    "Pong_A2.cpp" 
    "Pong_A2.h"
)

# Include directories
target_include_directories(Pong_A2 PRIVATE
    "${VCPKG_DIR}/installed/x64-windows/include"
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(Pong_A2 PRIVATE
    "${VCPKG_DIR}/installed/x64-windows/lib/SDL2.lib"
    "${VCPKG_DIR}/installed/x64-windows/lib/SDL2main.lib"
    "${VCPKG_DIR}/installed/x64-windows/lib/SDL2_ttf.lib"
)

# Copy required DLLs to output directory
add_custom_command(TARGET Pong_A2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${VCPKG_DIR}/installed/x64-windows/bin/SDL2.dll"
    "$<TARGET_FILE_DIR:Pong_A2>"
)

add_custom_command(TARGET Pong_A2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${VCPKG_DIR}/installed/x64-windows/bin/SDL2_ttf.dll"
    "$<TARGET_FILE_DIR:Pong_A2>"
)

# Optional: Copy font file to output directory
add_custom_command(TARGET Pong_A2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/arial.ttf"  # Replace with actual font path
    "$<TARGET_FILE_DIR:Pong_A2>"
)
