# pH Probe Notes



Current design involves a fully standalone AquapHOx-LX pH meter that is correctly timestamped to the same time zone as the operator computer, so data logs can be correlated after recovery.

### Installing the AquapHOx-LX pH logger:

The AquapHOx-LX pH logger sits after the ph probe on the fuid path.

3D print the flow cell attachment for the AquapHOx-LX using a 100% fill FDM print, alternatively use a hollow print and fill with epoxy resin from the bottom - below are my 3d printer slicer settings for doing that:

![SlicerResult1](/Users/ky/Documents/Github/ISMS_Github/documentation/Fabrication/pH Sensor/Images/SlicerResult1.png)

![SlicerSettings1](/Users/ky/Documents/Github/ISMS_Github/documentation/Fabrication/pH Sensor/Images/SlicerSettings1.png)

Once printed, cut away any internal bridging filaments, flip upside down and fill with a slow cure epoxy or resin such as [MG Chemicals 832HD](https://www.digikey.com/en/products/detail/mg-chemicals/832HD-50ML/9658009)

Once printed and hardened tap threads in the input and output ports with a 1/8" NPT pipe tap

Install 1/8" NPT to Swagelok 1/4" tube fitting adapters into the NPT holes we just tapped with NPT threads wrapped in 2+ wraps of PTFE tape.

### Using the AquapHOx-LX pH meter:

See the pyroscience software documentation - use the usb-to-subconn adapter cable included w the AquapHOx-LX to initialize it and begin logging - the  AquapHOx-LX is not yet integrated into the software of the ISMS and is fully standalone logger.

### Alternative designs:

The onboard PCB was orignally designed to be electrically pin compatible with the [Pyroscience Pico pH Sub](https://www.pyroscience.com/en/products/all-meters/pico-ph-sub) pH Probe, however this probe requires waterproofing the back end of the bulkhead part. As such, it would need an additional oil-compensated enclosure design and subconn or integration into the main housing end cap, which was deemed to tricky while maintaining structural integrity of the main end cap at the time. Software has not been written for any pH probe but should be fairly trivial to integrate if desired following the same patterns as the other serial sensors in the codebase.

## Next Build Section: [Vehicle Integration Guide](../ISMS%20V3%20Vehicle%20Integration%20Guide.md)
