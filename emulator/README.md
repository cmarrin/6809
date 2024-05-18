# BOSS9

BOSS9 (Basic Operating System Services for 6809) provides functions and a monitor for debugging programs.

## Monitor

You can start the emulator with a -m flag which will enter the monitor after loading the srecord file, at the start address. 

### Commands:

        B(reak)    <cr>  List breakpoints along with breakpoint number (used for delete)
                   NNNN  Set breakpoint at address
                   -n    Clear breaking with the passed id number
                   -     Clear all breakpoints

        M(emory)   <cr>  Display 16 bytes of memory at the current memory window and advance window by 16
                   NNNN  Display 16 bytes at address and set memory window to address + 16

