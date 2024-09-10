# Harte Tests 6502 emulator  
This project implements the documented opcodes for the 6502 processor.  At this stage, the undocumented opcodes are not implemented.  The implementation is cycle accurate.  Each `step` of the emulated computer advances the 6502 CPU by one cycle.  
    
The 6502 emulation is in the context of a machine with RAM, ROMS and IO Ports, but for this specific application, the machine is configured with 64K of RAM, no banking, no ROMS and all RAM considered an IO Port.  This is because IO port access calls a callback function and this is the easiet way to trap RAM access and values read/written from/to RAM, which is needed to run the tests.  
  
# The files
| Name | Description
|---|---
| src/6502.? | All the code to emulate a 6502 based computer
| src/cJSON.? | cJSON to parse the Harte tests
| src/harte.c | The Harte "Computer". Run the tests, etc. `main()`
| src/log.? |  A simple (not well tested) channel log utility
  
# The HARTE tests  
The HARTE tests refer to this repo: https://github.com/SingleStepTests/65x02  
In this repository there are 10,000 tests per 6502 CPU instruction.  The tests are presented in JSON files.  To read the tests I use cJSON.  Here's a single test as an example:  
```
{
    "name": "69 1b 91",
    "initial": {
        "pc": 49119,
        "s": 46,
        "a": 76,
        "x": 36,
        "y": 4,
        "p": 228,
        "ram": [ [49119, 105], [49120, 27], [49121, 145]]    
    },
    "final": {
        "pc": 49121,
        "s": 46,
        "a": 103,
        "x": 36,
        "y": 4,
        "p": 36,
        "ram": [ [49119, 105], [49120, 27], [49121, 145]]  
    },
    "cycles": [
        [
            49119,
            105,
            "read"
        ],
        [
            49120,
            27,
            "read"
        ]
    ]
}
```
  
Run the emulator `./build/6502` (on macOS or Linux - on Windows likely in build/Debug/x64 or build/Release/x64 or something along those lines) and pass a name or wildcard to the tests to run (for example `6502/*.json` or `6502/00.json` where `6502` is whatever folder you choose to put the Harte tests when grabbing them from git).  I grabbed the test files like this on my Mac Mini, so I could put them right where I wanted them:  
```
for i in {0..255}; do
  hex=$(printf "%02x" $i)
  url="https://raw.githubusercontent.com/SingleStepTests/65x02/main/6502/v1/${hex}.json"
  curl -O $url
done
```
  
# How does this emulation fare?  
```
TEST RESULTS:  
Ran    : 1510000
Passed : 1508482
Failed : 1518
Skipped: 105
Percent: 99.90%
```

All 1518 tests that it fails are in `adc`, in `BCD` mode, where my emulator differs only in terms of the `N` and sometimes also the `V` flag results.  I also think, but I didn't carefully verify, that the discrepancy between my results and the tests are when `adc` is not given valid `BCD` values'.  Furthermore, I have been told that the actual 6502 in the Apple II, for example, will also give results that differ, in that aspect, from the tests.  So, at this point, I am happy with the performance of my emulator.  I have yet to use it for something useful, and if I do and I find issues, I'll come and update this (at least the README ;)  
  
# Why and what's next?  
I did this just because I was curious and wanted to try making a 6502 emulator.  I might try to write a portion of a system emulator, likely for the Commodore 64, that can run some basic programs without graphics, sound or sprites.  A minimal C64 emulator.<sup>*</sup>
  
<sup>*</sup>Well, I made a very small Apple II emulator that can run the Manic Miner I made for the Apple II and it works great (this 6502).  The Speaker on the emulation is horrible but it works pretty well, so I can confirm this CPU emulation works well enough in a real application.  You can see more about that here: [Manic Miner Machine](https://github.com/StewBC/mminer-apple2/blob/master/src/mmm/README.md)  
  
Feel free to contact me at swessels@email.com if you have thoughts or suggestions.  

Thank you  
Stefan Wessels  
8 September 2024 - Initial Revision