# In-Situ Mass Spectrometer V3

NOTE this branch is specifically for the 2026 ISMS made for IFEMER.
The main branch is for the current standard version of the ISMS made by the Girguis Lab

IFREMER-specific documentation and notes can be found in the Ifremer-documentation folder.
 - [Ifremer 2026 ISMS Notes Doc](<Ifremer-isms-documentation/Ifremer 2026 ISMS Notes.docx>)
 - [Ifremer 2026 ISMS Operation Doc](Ifremer-isms-documentation/Ifremer_2026_ISMS_Operation.docx)
 - [Ifremer 2026 ISMS Modern Firmware Doc](<Ifremer-isms-documentation/Ifremer 2026 ISMS Modern Firmware Guide.docx>)

This branch contains two separate firmwares for the ISMS:</br>
For notes and upload tips see the [Ifremer 2026 ISMS Notes Doc](<Ifremer-isms-documentation/Ifremer 2026 ISMS Notes.docx>)
 1. Legacy-interface compatible version

    Source code is found in the Legacy-ISMS-firmware-Ifremer-2026 folder. Open in Arduino IDE and compile/upload to the onboard Arduino Mega.

 2. Modern interface with support for Serial Studio dashboard

    Source code is found in the Src and Lib folders. To compile this firmware, open the full repository (ifremer-2026-build branch) in the VSCode IDE with the Platform.io extension and use the upload button provided by the extension (small right arrow in lower left corner of IDE, or type "> PlatformIO: Upload" in the command pallet) to upload firmware to the onboard Arduino Mega.

         > NOTE: To upload either firmware, attach a USB-B cable to the Arduino without the rest of the  instrument powered on. You must DISCONNECT the red/orange serial cable at the Arduino pins 0 & 1 before uploading code. Once done, reconnect the serial cable to the Arduino mega shield with the red wire in Arduino pin 0 (RX0) and the orange wire in pin 1 (TX1) /
         The serial connection over USB may not show the correct output, use the RS232 connection from serial pins 0 & 1 for proper CLI usage.


![ISMS in box](Ifremer-isms-documentation/Hardware.jpg)
----
