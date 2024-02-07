In this tutorial we're going to breadboard a starting point of using an SD card with the Daisy Seed. We're using the SDMMC example from the DaisyExamples as a starting point, so you can work from there as soon as you get this up and running.

Requirements;

- Daisy seed
- An SD card, formatted in FAT32 (?). Not sure if there are any max. storage requirements
- STLINK-V3MINIE
- SD card breakout board (I used this one from Popolu; https://opencircuit.shop/product/breakout-board-for-microsd-card)
- VS code with Cortex debug extension
- DaisyExamples repository on your computer

1. Setup the breadboard as shown in diagram (TODO) and connect the debugger to the Daisy Seed
2. Alter the SDMMC example by adding this line before the line that specifies "sd.Init(sd_cfg);";
   `sd_cfg.speed = daisy::SdmmcHandler::Speed::SLOW;`
   This will make the SD card run at a slower speed, which required when using breadboard wires. This can later be removed when you start using a PCB with proper traces.
3. Set a breakpoint at the line that is as follows; "if(f_open(&SDFile, TEST_FILE_NAME, (FA_CREATE_ALWAYS) | (FA_WRITE)) == FR_OK)"
4. Run SDMMC example in debug mode (using F5 in vscode with cortex debug & STLINK-v3-MINIE)
5. When the executer is at this line, "step into" and "step over" until you reach "if (res == FR_OK)"
6. Check the status by hovering over "res";
   a. FR_DISK_ERR: SD card was not found (check your wiring on the breadboard and ensure the SD card is properly inserted)
   b. FR_NO_FILESYSTEM: SD card was found, but appears not to be formatted correctly. I had this with both FAT and ExFAT, formatted using Mac.

Notes;

- don't be misled by the blinking LED in this example while debugging. It is not always a sign of a success in this case, as the execution won't stop on failure while debugging.
