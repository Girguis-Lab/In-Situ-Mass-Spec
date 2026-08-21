# pH Probe Notes



Current design involves a fully standalone AquapHOx-LX pH meter that is correctly timestamped to the same time zone as the operator computer, so data logs can be correlated after recovery.

### Installing the AquapHOx-LX pH logger:

The AquapHOx-LX pH logger sits after the ph probe on the fuid path.

3D print the flow cell attachment for the AquapHOx-LX using a thick wall or 100% fill FDM print.

Once printed thread tap the input and output ports with a 1/8" NPT pipe tap 

Install 1/8" npt to swagelok 1/4" pipe adapters into the NPT holes we just tapped with 2 wraps of PTFE tape.

### Using the AquapHOx-LX pH meter:

See the pyroscience software documentation - use the usb-to-subconn adapter cable included w the AquapHOx-LX.

### Alternative designs:

The onboard PCB was orignally designed to be electrically pin compatible with the [Pyroscience Pico pH Sub](https://www.pyroscience.com/en/products/all-meters/pico-ph-sub) pH Probe, however this probe requires waterproofing the back end of the bulkhead part. As such, it would need aditional oil-compendsated enclosure design and subconn or integration into the main housing end cap, which was deemed to tricky while maintaining structural integrity of the main end cap at the time.





