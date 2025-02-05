# Surroundings

## Author

Laurens Lamberts

## Description

Ambience sample looper

## Prerequisites

- wavplayer.cpp in libDaisy has an error, and does not properly locate the WAV file. This has to be fixed using the following change at line 38;

```cpp
strcpy(file_info_[file_cnt_].name, search_path);
if(search_path[strlen(search_path) - 1] != '/')
{
    strcat(file_info_[file_cnt_].name, "/");
}
```

The first line above is already there. The other lines have to be added.

## How to run

1. Install the Toolchain; https://github.com/electro-smith/DaisyWiki/wiki/1b.-Installing-the-Toolchain-on-Mac
2. Ensure you opened just the "Surroundings" folder in VSCode, not a parent
3. CMD-P --> "task build_all"
4. Did you connect STLINK-V3MINIE? Then run CMD-P --> "task build_and_program". Otherwise, bring the Daisy Seed to DFU mode and run CMD-P --> "task build_and_program_dfu".

## Debugging

1. Ensure [Cortex Debug Extension](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) is installed in VSCode.
2. Attach your Daisy to the debug probe
3. Press (fn) F5, to enter debug mode, and flash your Daisy. VS Code will launch into the debugger, and halt at the default breakpoint. Press the continue arrow in the small debug menu to continue running the example.

Under the hood, this command builds all libraries, builds your example, flashes via openocd, then connects the debug environment.

## Troubleshooting

Any issues with RGB? It might be handy to run 'seed/Template'.
That example gives a smooth RGB light rainbow.
In Surroundings there's probably too much going on, so that the light does not get updated frequently enough.
