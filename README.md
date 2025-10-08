# Harte Tests 6502 (and 65c02) emulator  
This project implements the documented opcodes for the 6502, and Synertek 65c02, processor.  At this stage, the undocumented opcodes are not implemented on the 6502.  The implementation is cycle accurate.  Each `step` of the emulated computer advances the 6502 CPU by one opcode.  This is V2 of the core - V1 advanced by 1 cycle but I made this code use a case statement, with 1 opcode advanced, and all code inlined.  It is still cycle accurate, but it is much faster than the V1 core.  
    
The emulation is in the context of a machine with RAM, ROMS and IO Ports, but for this specific application, the machine is configured with 64K of RAM, no banking, no ROMS and all RAM considered an IO Port.  This is because IO port access calls a callback function and this is the easiet way to trap RAM access and values read/written from/to RAM, which is needed to run the tests.  
  
# The files
| Name | Description
|---|---
| src/65* | All the code to emulate a 6502 or 65c02 based computer
| src/cJSON.? | cJSON to parse the Harte tests
| src/harte.c | The Harte "Computer". Run the tests, etc. `main()`
| src/log.? |  A simple (not well tested) channel log utility
| CMakeLists.txt |  There are a few switches that control output and testing
  
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
In Powershell, that would be:  
```
for ($i = 0; $i -le 255; $i++) {
    # Format number as two-digit hex (00..ff)
    $hex = '{0:x2}' -f $i
    $file = "$hex.json"
    $url  = "https://raw.githubusercontent.com/SingleStepTests/65x02/main/6502/v1/$file"

    Write-Host "Downloading $file..."
    Invoke-WebRequest -Uri $url -OutFile $file
}
```
The URLs for the Synertek 65c02 are:  
```
Bash:
url="https://raw.githubusercontent.com/SingleStepTests/65x02/main/synertek65c02/v1/${hex}.json"
or
Powershell:
$url  = "https://raw.githubusercontent.com/SingleStepTests/65x02/main/synertek65c02/v1/$file"
```
  
# How does this emulation fare?  
  
## 6502  
```
harte_6502> .\build\6502.exe 65x02/6502/*.json
...
TEST RESULTS:
Ran    : 1510000
Passed : 1508482
Failed : 1518
Skipped: 1050000
Percent: 99.94%
```
All 1518 tests that it fails are in `adc`, in `BCD` mode, where my emulator differs only in terms of the `N` and sometimes also the `V` flag results.  I also think, but I didn't carefully verify, that the discrepancy between my results and the tests are when `adc` is not given valid `BCD` values'.  Furthermore, I have been told that the actual 6502 in the Apple II, for example, will also give results that differ, in that aspect, from the tests.  So, at this point, I am happy with the performance.

## 65c02 - Synertek  
The 65c02 emulation appears to do a bit worse, but it all has to do with the `V` flag in `BCD` and `adc`.
```
harte_6502> .\build\6502.exe - 65x02/65c02/*.json
TEST ff: Ran: 2560000 Passed: 2554150 Failed:  5850 Skipped:     0 Percent:   99.77%
```
    
# Why and what's next?  
I did this just because I was curious and wanted to try making a 6502 emulator.  

Using the V1 core, I made a very small Apple II emulator that can run the Manic Miner I made for the Apple II.  I call it the Manic Miner Machine.  I used the V1 6502 code as-is.  The Speaker audio on the Manic Miner Machine emulator is horrible but otherwise it works great, so I can confirm this CPU emulation works well enough in a real application.  You can see more about that here: [Manic Miner Machine](https://github.com/StewBC/mminer-apple2/tree/master/src/mmm)  
  
I then, using that same core, made a more proper Apple II+ emulator.  You can see more about that here: [a2m](https://github.com/StewBC/a2m).  I have upgraded a2m to the V2 inline core.

Next I would like to take the 65c02 inline core, and then put that also into [a2m](https://github.com/StewBC/a2m) and extend the emulator to a full Apple //e emulator.  That might be a while, though.   
  
Feel free to contact me at swessels@email.com if you have thoughts or suggestions.  

Thank you  
Stefan Wessels  
8 September 2024 - Initial Revision