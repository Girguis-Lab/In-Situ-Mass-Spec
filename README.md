# In-Situ Mass Spectrometer V3

NOTE this branch is specifically for the 2026 ISMS made for IFEMER.
The main branch is for the current standard version of the ISMS made by the Girguis Lab

This branch contains two separate firmwares:
 1. Modern version (to be compiled in the VSCode + Platform.io IDE)
    Source code is found in the ./Src and Lib folders, to compile open the full repository in VSCode with the platformio extension and use the compile and upload button provided by the Platformio extension to upload firmware to the onboard Arduino Mega.
 2. Legacy-interface-compatible version (to be compiled in the Arduino IDE)
    Source code is found in the ./Legacy-ISMS-firmware-Ifremer-2026 folder. Open in Arduino IDE and compile/upload to the onboard Arduino Mega.


IFEMER-specific documentation and notes can be found in the Ifremer-documentation folder.



![Benchtop_Sled_V3](./documentation/Images/Benchtop_Sled_V3.jpg)

*__This repository contains the documentation and code for fabricating and operating version 3 of the Girguis Lab In-Situ Mass Spectrometer.__*

The In-Situ Mass Spectrometer (ISMS) is an open source biogeochemistry instrument for studying how disolved gases and volatile compounds influence ecosystems and processes in the ocean depths. It is designed as a membrane inlet mass spectrometer based on a commercially available quadrupole mass spectrometer. The ISMS is capable of detecting and quantifying volatiles up to 200 daltons at depths down to 4,500 m.


## Repository Organization:

* / [documentation](./documentation/)
</br>All documentation lives in this folder.

    * / [Operation](./documentation/Operation/README.md):</br>
    Operation instructions for using ISMSs in the field and in the lab.

    * / [Fabrication](./documentation/Fabrication/README.md):</br>
    Fabrication instructions to create new ISMSs including BOMs, PCB layouts, CAD designs, and engineering drawings


    * / [Calibration & Testing](<./documentation/Calibration & Testing/README.md>):</br>
    Methods for calibration and testing of the ISMS

* / [src](./src):</br>
The code that runs on the ATMEL microcontroller inside the ISMS lives in the /src folder as well as the /lib and /test folders.

## Contributing

The Girguis Lab is an open-source/open-design facility. We strongly believe that an “open source” approach to disseminating technologies enables more rapid discovery and more efficient use of our community’s limited financial resources. When partnering with commercial entities, we make every effort to encourage “open source” approaches to development, and to ensure that any co-developed technologies are—at the very least—available to academic scientists at a reasonable price.
Contributions and pull requests are welcome, however for major changes we seggest you to fork the repo and make your own version. Reach out to us if your group is building or redesigning an ISMS


## License
TODO - Add license
Please reach out to us at girguislab@oeb.harvard.edu with any questions!
