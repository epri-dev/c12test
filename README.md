# c12test

The ANSI C12.18 standard describes an infrared communications protocol for electricity meters.  It is very commonly used for electricity meters in North America and some other parts of the world.  This software is used to test compliance with the ANSI C12.18 standard.  It is intended to be used on a Linux-based computer such as a Raspberry Pi, using either a commercially available optical probe or a low cost [open source hardware probe](https://github.com/beroset/Optopod).  

The code is largely based on the open source [C12Adapter](https://github.com/ElsterSolutionsOpensource/C12Adapter) project which is a set of libraries developed at Elster Solutions (now part of Honeywell).  See the [LICENSE](LICENSE) for licensing details.

## Using the software
To use the software, it's most convenient to create a configuration file for the particular meter that is to be read.  A sample configuration file, arbitrarily named `meter.ini` is shown below.

To use the software with that configuration file to read some [ANSI C12.19 tables](https://webstore.ansi.org/preview-pages/NEMA/preview_ANSI+C12.19-2012.pdf) one could use this command:

    c12test --config=meter.ini ST0 ST1 ST2 ST5

### meter.ini

```
[channel]
TYPE=CHANNEL_OPTICAL_PROBE
PORT_NAME=/dev/ttyUSB0

[protocol]
TYPE=PROTOCOL_ANSI_C12_18
PASSWORD="55555555555555555555"
MAXIMUM_NUMBER_OF_PACKETS=255
SESSION_BAUD=19200
PACKET_SIZE=128
ISSUE_SECURITY_ON_START_SESSION=FALSE
ISSUE_NEGOTIATE_ON_START_SESSION=TRUE
```

## Building

The software uses [CMake](https://cmake.org/) to simplify building the software.  After downloading the source code, the simplest way to create the software is to navigate to the `c12test` directory use these two commands:

    cmake -S . -B build
    cmake --build build

