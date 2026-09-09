# Upasnes
A SNES emulator with sub-cycle-accurate Ricoh 5A22 and SPC-700 emulation and support for SuperFX coprocessor games. This is the first installment of my series of emulator projects, with the ultimate aim to combine these into a multi-system-emulator. This is currently able to support most major titles with no difficulty. This is currently only available for use on Linux.

## Set-up
1. Clone via `git clone github.com/kctblackley/snes-emulator`;
2. Due to copyright restrictions, the SNES SPC-700 IPL ROM will need to be manually provided. This can be found online. It is a 64-byte file;
3. Download the IPL ROM. In the emulator's root directory, there is a folder, `ipl`. Place the IPL ROM file in there. Name the file `ipl.rom`;
4. Via the terminal, locate to the root directory for the emulator. Build in Release mode via: `cmake -B build -DCMAKE_BUILD_TYPE=Release ; cmake --build build`
5. This completes the set-up for base hardware games. If you want to play a game that requires a coprocessor, see below...

### SuperFX Games
No set-up required. Commercially-released or known prototypes will be assigned the correct SuperFX revision automatically. ROM hacks will be assumed to use the GSU2 revision if hardware detection of the correct revision fails.

So far, the SuperFX is the only supported coprocessor. Work is focused on implementing the remaining coprocessors in the following order.

### DSP-n/ST010/ST011/ST018 Games
For these games, download all of the firmware as was used by bsnes for versions v0.087 or later from [this website](https://caitsith2.com/snes/dsp/). Place, ensuring the names match exactly what is seen on the website, into the `firmware` folder.

### SA-1/CX4/OBC1/S-DD1/SPC7110 Games
No set-up required.

## Usage
1. To use, download a USA/Japan ROM for a game of your choosing. Place the downloaded file in the `rom` folder;
2. Rename the ROM to a simple name (e.g. `Zelda - A Link to the Past.sfc` should become `zelda.sfc`)
3. To run the emulator, navigate to the root directory. Run `./build/snes_emulator ___.sfc` (for example: `./build/snes_emulator zelda.sfc`)

