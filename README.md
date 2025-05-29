# Aetherion-2350
### *(Ostrich 2.0 Emulation for RP2350 - aka "BitStream")*

![License: BSD-3-Clause](https://img.shields.io/badge/License-BSD--3--Clause-blue.svg)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Platform: RP2040](https://img.shields.io/badge/platform-RP2040-orange)
![Version](https://img.shields.io/badge/version-1.2.8-informational)

## Version 1.2.8 additions and subtractions:
- **Manual Reset**: Drag and drop contents from /testing/manual_reset/ flash this onto a seperate RP2 device and connect the wires accordingly to your emulator.
- **Fixed bug**: Removed start_log function, piped COMPORT datalog datum to read_and_forward function (ostrich.c). Performs complete mediation and not partial mediation.
- **Feature**: Corrected "Waiting for x:/ to open..." loop when developer COMPORT is occupied.
- **Feature**: Added in signal lights for error, read/write, and USB activity.
- **Major correction**: injection.c had huge risky bug where data could be reset mid way (probably could never happen although it was corrected)
- **Major feature**: Included "core alive" detection for when device eventually fails. Error handling and detection for both cores.
- **Minor removal**: Removed, setting function to time critical for developer resets (developer_reset.c). This effectively cannot serve a real purpose beyond developement.
- **Minor bug**: Fixed bug around bank settings. (perhaps more to follow?)
- **Minor Feature**: start up LED animation
- **Other minor improvments**: code correction, refactoring, optimization, etc... Should work better with customized boards.

## Future Version:
- **Add button to Deployment GUI for Manual Resetting**
- **Clean up code and comments**
- **Add code comments to python files**
- **Refactor More**

> Fully emulated Ostrich 2.0 Protocol stack for RP2350 devices. Features developer tooling, flashing utilities, and RAM injection for high-speed tuning. 
  --- *Replace that which is old, with cutting edge.*

---

## What is Aetherion?

Aetherion is a 100% RP2350-compatible firmware implementing the Ostrich 2.0 protocol. Designed for developers, tuners, and hobbyists, it enables:

- **Developer Tools** for flashing, debugging, and binary downloads
- **RAM Injection** with nanosecond write capability
- **Non-interupting Flash Saving**, flash memory with minimal wear designed in
- Bundled Python tools and a GUI for easy deployment

Say goodbye to overpriced proprietary systems like "Snake", "Hondavert", "Demon", and "Hondata" - **Aetherion firmware levels the playing grounds and makes tuning fun again!**.

---

## Features

- Full Ostrich 2.0 emulation over USB
- On board RAM writing with **ns-scale** timing
- Dual bank mode for switching between two tunes
- Developer serial port (customizable in `developer_tools.h`)
- Byte-based device control:
  - `0x2202` = Reset  
  - `0x2201` = Full Wipe
- /testing/manual_reset:
  - `r\r` = Reset Device from PuTTY or Script  
  - `b\r` = Bootload Device from PuTTY or Script
- Overclock support up to **+200 MHz** (**5 nanosecond execution**)
- Secondary RP2040 **Datalog Emulation** device
- Custom USB descriptors - personalize your plug-in name!
- Zero-driver install - plug & play with BMTune (bye bye sketchy Snake drivers)
- Optional startup lighting & Bluetooth hooks (code them in! lets see what you got!)

---

## Installation

### Requirements

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- Python 3.9+ (Install via Microsoft Store or python.org)
- Visual Studio Code with CMake support

### Quick Start

```bash
# Clone the repository
git clone https://github.com/dlewis0001/Aetherion-2350.git
cd Aetherion-2350

# Install Pico SDK and dependencies
# (VSCode + CMake button UI handles build process)

# Launch GUI
cd testing/
python deploy_ui.py
```

---

## Usage Example

- Connect your RP2350-MCU Board to your computer
- Use `deploy_ui.py` to build a firmware and drag and drop your UF2 file into the RP2350 drive.
- Optionally connect a second RP2040 for Datalog sim, connect TX/RX from the 2040 to the 2350
- Plug it into BMTune and connect, datalog, change values, upload, dowload, disconnect and do it again.

---

## File Structure

```
Aetherion-2350/
├── .vscode/
├── build/
├── images/
├── src/
└── testing/
    └── GUI, python tools
```

---

## License

Aetherion contains portions from other open source efforts and is covered under multiple BSD-3-Clause licenses and the copyright from Keith Daigle (Ostrich protocol creator).

<details>
<summary><strong>Keith Daigle (2012) License</strong></summary>

```text
Copyright (c) 2012, Keith Daigle
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.  Redistributions in binary
form must reproduce the above copyright notice, this list of conditions and
the following disclaimer in the documentation and/or other materials
provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
Covered files:
  - ostrich.c
  - ostrich.h
```
</details>

<details>
<summary><strong>Dennis B. Lewis (2025) License</strong></summary>

```text
SPDX-License-Identifier: BSD-3-Clause
Copyright (c) 2025, Dennis B. Lewis

Copyright (c) 2025, Dennis B. Lewis
All rights reserved.
This file contains modifications to software originally licensed under the
BSD-3-Clause license by the Raspberry Pi Foundation.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the Raspberry Pi Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
Covered files:
 - abstract_layer.c
 - abstract_layer.h
 - descriptors.c
 - developer_reset.c
 - developer_reset.h
 - flash_memory.c
 - flash_memory.h
 - injection.c
 - injection.h
 - injection.pio
 - mutexes.c
 - mutexes.h
 - main.c
 - deploy_ui.py
 - bin_reader.py
```
</details>

---

## Contributing

Pull request rejected (as is), feel free to fork.

Contact: [dlewis0001@proton.me](mailto:dlewis0001@proton.me)

---

## Screenshots & Media for Testing and Development

![Deploy GUI](/images/deploy_gui.png "Deployment GUI with features!")
You must have DEVELOPER_CONSOLE set to 1 for this to work and the firmware built and uploaded.
DEVELOPER_CONSOLE {constant} found in: /src/developer_tools.h/

![Engine SIM](/images/bmtune_engine_sim.png "Real simulation for engine")
You must have a secondary RP2 device and circuit python firmware. drag and drop the CONTENTS of /testing/data_logging_RP2/
into your circuit python MSD drive and connect the TX/RX irrespectively of the emulating device.
(datalogging abstraction can be found in lib of the data_logging_RP2 folder.)

---

## Legacy Warning

> “LIMP MODE” warnings from BMTune or HTS are now a legacy term.  
> The Ostrich Emulator you build yourself is faster, simpler, and yours.

---
## Futer Developement
> Research paper about reverse engineering and development.
> Future for tuning research and development.
> Phoenix Protocol.
> HTS replacement.

*The strongest prison ever constructed was made with imaginary bars --- with you as the architect.*  
- _Dennis B. Lewis, 2025 (Uhhdennis)_ *duh!*
