 # NESCLE
 ## What is NESCLE?
 NESCLE (pronounced like Nestlé) is a free and open-source NES (Nintendo Entertainment System) emulator written
 in the C programming language, with minimal parts of the front-end written in C++. It uses SDL2 for graphics, audio,
 and input, and ImGui for the graphical user interface. NESCLE also provides a command line interface for users
 who prefer to use the terminal.

 ### Why C?
 Most commonly used NES emulators are almost exclusively written in C++, and many newer, lesser used emulators are
 written in Rust. Knowing little C++ and no Rust, but still requiring the high-performance of those languages,
 I chose to rely on an old friend, C. This makes NESCLE unique among the existing NES emulators, as the code (mostly)
 maintains the readability of C code compared to the often hard to parse, bloated C++ code. C++ (specifically modern
 C++) contains many problems that make code hard to read for those who are not well versed in its nuances (ex. `std::shared_ptr` and `std::unique_ptr`, references vs. pointers, templates, etc).
 I do not claim to be the only person to write a NES emulator in C,
 but I do claim that popular forms of such emulators are rare.

 ## Requirements and Dependencies
 As of now, NESCLE has not been packaged for any operating system. This means that you have to compile it yourself from the source code.
 To build the code you need a C compiler that supports the C99 and C++11
 standards.

 Requirements:
 * CMake 3.11+
 * A C/C++ compiler that supports C99 and C++11

 This software makes use of the following third-party libraries:
 * SDL2 under the terms and conditions of the [zlib license](https://www.libsdl.org/license.php)
 * ImGUI under the terms and conditions of the [MIT license](https://github.com/ocornut/imgui/blob/master/LICENSE.txt)
 * tinyfiledialogs under the terms and conditions of the zlib license

## Installation
### Windows
1. Run [install.ps1](install.ps1) from a PowerShell prompt
2. Create a directory in which you would like to store your build of nescle
3. cd into that directory and run `cmake --build .`
4. Run NESCLE with `./nescle`

### Linux
1. Run [install.sh](install.sh) from your terminal
2. Create a directory in which you would like to store your build of nescle
3. cd into that directory and run `cmake --build .`
4. Run NESCLE with `./nescle`

 ## Acknowledgements
 I would like to provide a massive thank you to javidx9 (aka the One Lone Coder) for his excellent YouTube series
 on NES emulation and to the SimpleNES repository for providing countless help and inspiration for my own emulator
 implementation. By no means does this software copy verbatim from either of their implementations, nor any other
 NES emulator implementation, this is simply an acknowledgement that those sources were used by me as inspiration.
 The NESdev wiki also deserves a mention as a "one-stop shop" for finding out information about the NES.

 ## Contact Me
 You can contact me at the email address <edward.pinkston@gmail.com>

 ## License
 This software is licensed under the Apache 2.0 License. See [LICENSE.txt](LICENSE.txt) for more information.
