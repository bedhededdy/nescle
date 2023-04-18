#
# Copyright 2023 Edward C. Pinkston
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Require version 3.15 for generator expressions
cmake_minimum_required(VERSION 3.15)

# Force C/C++ verison globally to ISO Standard C11+ and C++11+
set(CMAKE_CXX_VERSION 11)
set(CMAKE_CXX_VERSION_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_VERSION 11)
set(CMAKE_C_VERSION_REQUIRED true)
set(CMAKE_C_EXTENSIONS OFF)

# Fetch nativefiledialog and jansson as a CMake subproject
include(FetchContent)
FetchContent_Declare(nativefiledialog
  GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
  GIT_TAG        master
)
FetchContent_MakeAvailable(nativefiledialog)

FetchContent_Declare(jansson
  GIT_REPOSITORY https://github.com/akheron/jansson.git
  GIT_TAG master
)
FetchContent_MakeAvailable(jansson)

# Figure out what compiler we are using
# TODO: CHANGE TO USE GENERATOR EXPRESSIONS

project(nescle)

# Download imgui as a zip (if it isn't here)
if (NOT EXISTS ${PROJECT_SOURCE_DIR}/imgui)
   file(DOWNLOAD
      "https://github.com/ocornut/imgui/archive/refs/heads/docking.zip"
      ${PROJECT_SOURCE_DIR}/imgui.zip
      STATUS download_status)

   if (NOT download_status)
      message(FATAL_ERROR "Failed to download imgui")
   endif()

   # Unzip imgui
   message(STATUS "Unzipping imgui...")
   execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${PROJECT_SOURCE_DIR}/imgui.zip
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
   execute_process(COMMAND ${CMAKE_COMMAND} -E rename imgui-docking imgui
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
   # Remove imgui.zip
   file(REMOVE ${PROJECT_SOURCE_DIR}/imgui.zip)
endif()

# Build ImGui as a library
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
set(IMGUI_BACKENDS_DIR ${IMGUI_DIR}/backends)
set(IMGUI_SRC imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui.cpp)
set(IMGUI_INC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
set(IMGUI_BACKENDS imgui_impl_sdl2.cpp imgui_impl_opengl3.cpp)

# Function to prepend directory to the source
# Stolen from StackOverflow (https://stackoverflow.com/questions/4346412/how-to-prepend-all-filenames-on-the-list-with-common-path)
FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(IMGUI_SRC ${IMGUI_DIR} ${IMGUI_SRC})
PREPEND(IMGUI_BACKENDS ${IMGUI_BACKENDS_DIR} ${IMGUI_BACKENDS})

# imgui recommends against building it as a shared lib
add_library(imgui STATIC ${IMGUI_SRC} ${IMGUI_BACKENDS})

# Download SDL2 libraries (don't build from source)
if (NOT EXISTS ${PROJECT_SOURCE_DIR}/SDL2)
   file(DOWNLOAD
      "https://www.libsdl.org/release/SDL2-devel-2.26.4-VC.zip"
      ${PROJECT_SOURCE_DIR}/SDL2.zip
      STATUS download_status)

   if (NOT download_status)
      message(FATAL_ERROR "Failed to download SDL2")
   endif()

   # Unzip SDL2
   message(STATUS "Unzipping SDL2...")
   execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${PROJECT_SOURCE_DIR}/SDL2.zip
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
   execute_process(COMMAND ${CMAKE_COMMAND} -E rename SDL2-2.26.4 SDL2
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
   # Remove SDL2.zip
   file(REMOVE ${PROJECT_SOURCE_DIR}/SDL2.zip)
endif()

# Add SDL2 as a package
set(SDL2_DIR ${PROJECT_SOURCE_DIR}/SDL2/cmake)
find_package(SDL2 REQUIRED CONFIG REQUIRED SDL2)
# FIXME: MAYBE THIS IS NOT FINDING SDL2main BECAUSE I GET WARNINGS
#        ABOUT WMAIN AND MAIN FROM THE LINKER
find_package(SDL2 REQUIRED CONFIG REQUIRED SDL2main)

# Build glad as a library (it must already be here)
set(GLAD_SRC ${PROJECT_SOURCE_DIR}/glad/src/glad.c)
set(GLAD_INC ${PROJECT_SOURCE_DIR}/glad/include)
add_library(glad STATIC ${GLAD_SRC})

# Add OpenGL
find_package(OpenGL REQUIRED)

# Add nescle as executable
set(NESCLE_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(MAPPER_SRC_DIR ${NESCLE_SRC_DIR}/mappers)
set(NESCLE_SRC Bus.c Cart.c CPU.c Emulator.c RetroText.cpp NESCLETexture.cpp
      EmulationWindow.cpp PatternWindow.cpp Main.cpp Mapper.cpp PPU.c APU.c
      DisassemblerWindow.cpp OAMWindow.cpp SettingsWindow.cpp ControllerWindow.cpp
      MixerWindow.cpp Util.c NESCLENotification.cpp)
set(MAPPER_SRC Mapper000.cpp Mapper001.cpp Mapper002.cpp Mapper003.cpp
        Mapper004.cpp Mapper007.cpp Mapper066.cpp)

PREPEND(NESCLE_SRC ${NESCLE_SRC_DIR} ${NESCLE_SRC})
PREPEND(MAPPER_SRC ${MAPPER_SRC_DIR} ${MAPPER_SRC})

set(SRC ${NESCLE_SRC} ${MAPPER_SRC})
set(INC ${TINY_DIR} ${SDL2_INCLUDE_DIRECTORIES})

add_executable(nescle ${SRC})

# Include directories
# FIXME: INCLUDE ALL LIBS AS SYSTEM HEADERS TO AVOID THEIR HEADERS
# PROPOGATING ERRORS TO MY CODE
target_include_directories(imgui PRIVATE ${SDL2_INCLUDE_DIRECTORIES}
                                 PUBLIC ${IMGUI_INC})
target_include_directories(glad PUBLIC ${GLAD_INC})
target_include_directories(nescle PRIVATE ${INC} ${GLAD_INC} ${JANSSON_INCLUDE_DIRS})

# Set compile options for each library
set(mycc undefined)
# FIXME: CHANGE THIS IMMEDIATELY
set(mycc COMPILER_CLANGCL)

if (mycc STREQUAL COMPILER_CLANGCL)
   target_compile_options(imgui PRIVATE -w)
   target_compile_options(glad PRIVATE -w)
   target_compile_options(nescle PRIVATE -Wall
   -Wextra -Wpedantic -Wimplicit
   -Wshadow -Wimplicit-function-declaration
   -Wnarrowing -Wdeprecated-declarations -Wcast-qual -Wformat=2
   -Wwrite-strings -Wnull-dereference -Wformat-security
   -Wcstring-format-directive

   -Wno-unused-parameter)

elseif(mycc STREQUAL COMPILER_MSVC)
   message(STATUS "Using MSVC FLAGS NEEDED TO BE ADDED")
elseif(mycc STREQUAL COMPILER_GNU)
   target_compile_options(imgui PRIVATE -w)
   target_compile_options(glad PRIVATE -w)
   target_compile_options(nescle PRIVATE -Wall
   -Wextra -Wpedantic -Wimplicit
   -Wshadow -Wimplicit-function-declaration
   -Wnarrowing -Wdeprecated-declarations -Wcast-qual -Wformat=2 -Wformat-signedness
   -Wwrite-strings -Wnull-dereference -Wformat-overflow -Wformat-security
   -Wcstring-format-directive

   -Wno-unused-parameter)
else()
   message(FATAL_ERROR "Unknown compiler")
endif()

# Link libraries
target_link_libraries(imgui PRIVATE ${SDL2_LIBRARIES} opengl32)
target_link_libraries(glad PRIVATE opengl32)
target_link_libraries(nescle PRIVATE glad nfd ${JANSSON_LIBRARIES}
   PUBLIC imgui ${SDL2_LIBRARIES} opengl32)
