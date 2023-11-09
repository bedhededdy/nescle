/*
 * Copyright 2023 Edward C. Pinkston
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NESCLE_TYPES_H_
#define NESCLE_TYPES_H_

namespace NESCLE {
// Mappers
class Mapper;
class Mapper000;
class Mapper001;
class Mapper002;
class Mapper003;
class Mapper004;
class Mapper007;
class Mapper066;

// Windows
class NESCLEWindow;
class WindowManager;
class EventManager;

// Other GUI
class NESCLENotification;
class NESCLETexture;
class RetroText;

// Emulation
class APU;
class Bus;
class Cart;
class CPU;
class Emulator;
class PPU;
}
#endif // NESCLE_TYPES_H_
