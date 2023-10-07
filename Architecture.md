# NESCLE Architecture
## Core Emulation
### APU
* Responsible for NES audio
### Bus
* Responsible for handling linking all the NES hardware together
* Think of the bus as a standin for the NES as a whole
### Cart
* Responsible for holding cartridge data and loading the ROM
### CPU
* Responsible for CPU emulation
### PPU
* Responsible for NES graphics

## Bridge (i.e. Emulator.cpp)
* Will provide an interface into the core emulation functionality
* Will also hold emulator metadata (i.e. settings, etc.)
* Will hold meta functions (i.e. save states, etc.)
* Nobody should interface with the core emulation directly, all calls should go through the emulator

## UI
### Backend
#### WindowManager
* Static class
* This will be the main loop
* It will spawn the emulation window when created and call the show function for all windows
* Will init everything
* Will have the render loop that is called by main
* Will determine which Window is focused
#### Event Manager
* Static class
* This will process SDL events and publish them to subscribers
#### Notification Manager
* Static class
* This will manage all notifications (such as save state loaded)
* Notification events will be published using the event manager

### Frontend
#### EmulationWindow
* This will be the main window that shows the main menu and the acutal game emulation
* Will be responsible for actually clocking the emulation

### Misc.
#### Util
* Holds utility functions used by eveything
* Will be its own library
