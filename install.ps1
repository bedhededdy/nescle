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

# Download ImGui from GitHub as a zip archive
Invoke-WebRequest -Uri 'https://github.com/ocornut/imgui/archive/refs/heads/master.zip' -OutFile ./imgui.zip
Expand-Archive ./imgui.zip .
Rename-Item ./imgui-master ./imgui
Remove-Item ./imgui.zip

# Download tinyfiledialogs from SourceForge as a zip archive
# NOTE: You MUST set the UserAgent flag to "Wget" or it will not
#       work properly
Invoke-WebRequest -UserAgent "Wget" -Uri 'https://sourceforge.net/projects/tinyfiledialogs/files/latest/download/' -OutFile ./tinyfiledialogs.zip
Expand-Archive ./tinyfiledialogs.zip .
Remove-Item ./tinyfiledialogs.zip

# Download SDL2-devel-2.26.2 for MSVC as a zip archive
Invoke-WebRequest 'https://github.com/libsdl-org/SDL/releases/download/release-2.26.2/SDL2-devel-2.26.2-VC.zip' -OutFile ./SDL2-devel.zip
Expand-Archive ./SDL2-devel.zip .
Rename-Item "./SDL2-2.26.2" ./SDL2
Remove-Item ./SDL2-devel.zip
