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

# Force appropriate language standards
set(CMAKE_CXX_VERSION 11)
set(CMAKE_CXX_VERSION_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_VERSION 99)
set(CMAKE_C_VERSION_REQUIRED true)
set(CMAKE_C_EXTENSIONS OFF)

project(nescle)

# Set compiler flags

# Build tinyfiledialogs as a library
set(TINY_DIR ${PROJECT_SOURCE_DIR}/tinyfiledialogs)
add_library(tinyfiledialogs STATIC ${TINY_DIR}/tinyfiledialogs.c)

# Build ImGui as a library
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
set(IMGUI_BACKENDS_DIR ${IMGUI_DIR}/backends)
set(IMGUI_SRC imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui.cpp)
set(IMGUI_INC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
set(IMGUI_BACKENDS imgui_impl_sdl.cpp imgui_impl_sdlrenderer.cpp)

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

# FIXME: NOT PROPERLY GETTING THE INCLUDE DIRS FOR BACKEND FOR WHATEVER
#        REASON
add_library(imgui STATIC ${IMGUI_SRC} ${IMGUI_BACKENDS})
#add_library(imgui STATIC ${IMGUI_SRC})

# Locate SDL libraries
set(SDL2_DIR ${PROJECT_SOURCE_DIR}/SDL2/cmake)
find_package(SDL2 REQUIRED CONFIG REQUIRED SDL2)
# FIXME: MAYBE THIS IS NOT FINDING SDL2main BECAUSE I GET WARNINGS
#        ABOUT WMAIN AND MAIN FROM THE LINKER
find_package(SDL2 REQUIRED CONFIG REQUIRED SDL2main)

# Add nescle as executable
set(SRC Bus.c Cart.c CPU.c EmulationWindow.cpp Main.cpp Mapper.c PPU.c APU.c)
set(INC ${TINY_DIR} ${SDL2_INCLUDE_DIRECTORIES})
add_executable(nescle ${SRC})

# Include directories
target_include_directories(imgui PRIVATE ${SDL2_INCLUDE_DIRECTORIES}
                                 PUBLIC ${IMGUI_INC})
#target_include_directories(imgui INTERFACE ${IMGUI_INC})
target_include_directories(nescle PRIVATE ${INC})

# Link libraries
target_link_libraries(imgui PRIVATE ${SDL2_LIBRARIES})
target_link_libraries(nescle PRIVATE tinyfiledialogs
   PUBLIC imgui ${SDL2_LIBRARIES})
