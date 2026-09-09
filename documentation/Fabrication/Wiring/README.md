## Internal Wiring

### Cut Wires

Cut the following sections of 24 AWG Silicone wire (or other flexible, stranded wire):

**RGA Power Wires** - 6 inches (5.5in once twisted together)

*Connectors: 2pin vertical female Micro-fit 3.0 | 5.5mm x 2.1mm center+ locking barrel plug  (Provided w RGA)*

- Black
- Red

**RGA Serial Wires** - 26.5 inches (24.75in once twisted together)

*Connector: Male DB9 with hood - fake handshake wiring will be added*

- Black
- Blue
- White with Black stripe

**Roughing Pump Power Wires** - 14.5 inches (13.5in once twisted together)

- Red
- Black

**Turbo Pump Power & Serial Wires** - 17.75 inches (16.5in once twisted together)

- Red
- Black
- Brown
- Orange

**Topside Power & Serial Wires** - 28.4 inches (26.5in once twisted together)

- Black
- White
- Red
- Green
- Orange
- Red with Black stripe

**Fluid Pump Wires** - 13 inches (12.25in once twisted together)

- Green with Black stripe
- Orange with Black stripe
- Blue with Black stripe
- Black with White stripe

**Fluid Pump Direction Control Add-on Wires**

- Yellow - 13.5 inches (not twisted>)
- 6x yellow solid core wire - 1.75 inches

**pH probe wires** (optional) - 15.5 inches (14.5in once twisted together)

- Red with White stripe
- Green with White stripe
- Blue with White Stripe
- Black with Red Stripe

Twist or braid wires into cables so they form nice bundles like so - a shortcut to do this is to leave some extra wire at one end to insert in the jaws of a power drill on low speed, with the other end clamped, then cut off the part that got mangled in the jaws once twisted sufficiently. Best practice would be to center the ground conductor in the larger cable bundles, but this is not necessary for the short distances involved:

![WiringTwistExamples](<./Build Images/Internal/WiringTwistExamples.jpeg>)

### Adding Connectors

All connectors on either end of the Main PCB are molex Micro-fit 3.0 female connectors consisting of pin sockets which must be crimped and inserted into connector bodies of various pin count.

Crimping molex pins and sockets is fairly simple, but there are many things to keep in mind to ensure a solid connection, with proper strip length, that doesn't bend parts of the connnector that should stay straight. Watch these videos to learn how to crimp: https://www.youtube.com/watch?v=pTJ77xNArZA & https://www.youtube.com/watch?v=2CHQWd-844Y and keep in mind the molex guidelines on page 17 of this guide https://tools.molex.com/pdm_docs/ats/TM-638000029.pdf

Gold plated connectors are recommended as these have slightly more durability. Even still keep plugging/unplugging to a minimum - many connectors are only rated to 10's of plug/unplug cycles.



#### Main PCB Connections

Create pigtails (connector at one end with bare ends at the other) for each used connector on the PCB.

Follow the wire color pin order to make each connector pigtail. Color coding is shown in the [ISMS PCB Pinouts Annotated](<./Build Images/Internal/ISMS PCB Pinouts Annotated.png>) file shown below. The wire color coding for each connector pin is shown below the label in the corresponding pin location as a line.

![ISMS PCB Pinouts Annotated](<./Build Images/Internal/ISMS PCB Pinouts Annotated.png>)

#####

##### pH Probe Connection (UNUSED):

​	4x1 PIN female Micro-fit 3.0

Currently the pH probe is not connected because we are using a standalone pH logger (PyroScience AquapHOx-LX) that doesn't need power or data. Theoretically realtime data can be got from the AquapHOx-LX, but it requires a USB host intermediary like a raspberry pi to read the serial data (format found in the [protocol docs]) if wired per the USB output wires shown here: ![AquapHOx-LX Subconn Pinout](<./Build Images/External/AquapHOx-LX Subconn Pinout.png>)

##### RGA Power:

​	2x1 PIN vertical female Micro-fit 3.0

##### Turbo Pump Power & Serial Wires:

​	2x2 PIN female Micro-fit 3.0

##### Topside Power + Console:

​	 3x2 PIN female Micro-fit 3.0

##### Custom Fluid Pump:

​	3x1 PIN female Micro-fit 3.0

##### Roughing Pump Power:

​	2x1 horizontal female Micro-fit 3.0

##### Extra Digital Pins

​	1x6 PIN male & female Micro-fit 3.0 connector

(used for adding direction forward/reverse control to Fluid Pump):

Solder 6 short connections from the PCB to a free-floating 6 pin male Micro-fit 3.0 connector and another to the yellow wire on pin 6 as shown here at the bottom of the photo. The 6 pin setup is to allow future expansion and structural integrity.

![ExtraDigitalPinsWiring](<./Build Images/Internal/ExtraDigitalPinsWiring.jpg>)

##### Extra Power Taps (UNUSED):

 5x2 PIN female Micro-fit 3.0

##### ICSP Headers (X2, BOOTLOADER TROUBLESHOOTING ONLY):

3x2 standard 2.54mm pitch pin header (not Molex>)



#### Other Connectors

##### RGA Power Plug

- **Side A (Already Done):** 2pin vertical female Micro-fit 3.0

- **Side B:** 5.5mm x 2.1mm center+ locking barrel plug  (Provided w RGA)*

  - Solder connector like so: https://electronics.stackexchange.com/questions/77671/how-to-solder-wires-on-to-this-power-barrel-plug. Use Heat shrink to insulate inner contact. When installing, the tabs on this barrel plug twist lock into the RGA body - Do not force it in or out!

  ![RGA Power Connector Internals](<./Build Images/Internal/RGA Power Connector Internals.jpeg>)

##### RGA Serial Connection

Does not go through PCB, instead it goes directly to terminal block and out of main housing up tether or to vehicle

![RGA Serial DB9 Wiring - RGA Connector](<./Build Images/Internal/RGA Serial DB9 Wiring - RGA Connector.png>)

![RGA DB9 Connector Internals](<./Build Images/Internal/RGA DB9 Connector Internals.jpeg>)

##### **Roughing Pump Power**

The roughing pump uses a male DB15 plug and also requires some jumpers behind the connector that tell the roughing pump to start up inmediately once power is supplied. See page 23 of the [roughing pump manual](<../../Operation/Component Manuals/Roughing Pump Manual pfeiffer_mvp_010_3_dc.pdf) for an explanation of this wiring.

![Roughing Pump Power DB15 Wiring](<./Build Images/Internal/Roughing Pump Power DB15 Wiring.png>)

![Roughing Pump DB15 Connector Internals](<./Build Images/Internal/Roughing Pump DB15 Connector Internals.jpeg>)

##### Turbo Pump Connector

The Turbo Pump connector is simpler but follows a similar pinout as the roughing pump. See the turbo pump TC80 controller manual page 18 for an explanation of pinout:   [Turbo Pump Control Unit Manual Pfeiffer-TC-80_Operating-Instructions.pdf](<../../Operation/Component Manuals/Turbo Pump Control Unit Manual Pfeiffer-TC-80_Operating-Instructions.pdf)

![Turbo Pump DB15 Wiring](<./Build Images/Internal/Turbo Pump DB15 Wiring.png>)

![Turbo Pump DB15 Connector Internals](<./Build Images/Internal/Turbo Pump DB15 Connector Internals.jpeg>)

### Wire Routing

Recommended wiring routing layout using zipties

![WireRoutingTopView](<./Build Images/Internal/WireRoutingTopView.jpg>)

Roughing pump power cable is tucked underneath lower rail and held with black zipties:

![WireRoutingSideView](<./Build Images/Internal/WireRoutingSideView.jpeg>)

The terminal block plate sits over the turbo pump and connects the 16pin subconn external connection wires to the isms internals. The mounting plate should be laser cut & engraved from 1/4" thick accrylic using the file: [Terminal Block Mounting Plate Laser Cut.eps](<Laser Cut Designs/Terminal Block Mounting Plate Laser Cut.eps) Every wire from the 16 pin external connection matches the color of the corresponding wire internally except for the fluid pump forward/reverse wire, which is the only yellow wire in the system and connects to the black wire with white stripe in position 12 on the terminal blocks (and 16 pin subconn).

![Terminal Block Plate Wiring](<./Build Images/Internal/Terminal Block Plate Wiring.jpeg>)

Note that the pH probe connections remain empty due to the afformentioned lack of need for power or communication with the aquapHOx LX pH logger. If a non-standalone pH probe is added in the future these wires can be used for that (or another accesory like a cryopump). They would connect to the pH probe power and serial plug on the PCB shown in [ISMS PCB Pinouts Annotated.png](<./Build Images/Internal/ISMS PCB Pinouts Annotated.png) (assuming the pH probe was 5v powered and used ttl serial communication>)

## Topside Wiring

For now we'll ignore the wiring in between the ISMS and focus on the connectors a topside operator or vechicle needs to power and communicate with the ISMS and onboard RGA. In the next section we'll connect those connectors to the actual ISMS for testing!

### Power plugs

We recommend using Molex Micro-fit connectors to connect to your power supplies for consistancy with the rest of the connectors inside the instrument however this is amenable to using whatever plug your power supply already comes with. Independent power supplies are recommended but not necessary.



### ISMS Main Comms Serial Plug

![Topside Main Comms DB9 Wiring](<./Build Images/Topside/Topside Main Comms DB9 Wiring.svg>)

The main comms of the ISMS talks to the onboard arduino-like chip and provides overall control and status of the instrument. When wiring this plug, leave loose ends in the colors shown noting that this plug acts as a "Null Modem" swapping RX and TX from the corresponding pins inside the ISMS to provide proper serial flow.

### RGA Comms Serial Plug

![Topside RGA Comms DB9 Wiring](<./Build Images/Topside/Topside RGA Comms DB9 Wiring.svg>)

The RGA comms of the ISMS 'talks' directly to the onboard RGA providing the mass spectra and tuning of the RGA via the windows software (or python library). Much like the the main comms this is wired as a "Null Modem" swapping RX and TX for proper flow, however in addition this plug requires some exra jumper wiring to "Fake" and handshake signal normally sent betwen the computer and RGA to keep them from talking over each other. In practice this fake handshake is sufficient as the windows software and RGA always follow a command-response format and so are never talking over each other anyway - this allows us to only require two signal wires to communicate with the RGA instead of 9. These jumper wires should fit into a plastic "Hood" on the back of the DB9 Connector.

### Overall Topside/Operator Wiring

![ISMS Recommended Topside Wiring](<./Build Images/Topside/ISMS Recommended Topside Wiring.svg>)

Shown in this diagram is the recommended overall wiring of the topside / operator side. No need to complete this now, refer to this diagram and table below if needed for correct wiring in the next section.



## External 'Octopus Cable' Wiring

The exterior of the ISMS has a 1in-3out "Octopus" style cable to connect the main housing various external components in a watertight manner. It is made by potting together several subconn connector pigtails. The subconn provided cable pigtails follow a consistant color to PIN number scheme. The wiring of this octopus cable can be figured out from [SubCon Octopus Cable Key.pdf](<./SubCon Octopus Cable Key.pdf>). The black cable in this system overview is the octopus cable:
 ![ISMS-Diagram-System-Overview](/Users/ky/Documents/Github/ISMS_Github/documentation/Images/ISMS-Diagram-System-Overview.png)



### Testing with quick connectors

We will be making a test wiring harness / octopus cable directly from the subconn 16 pin female pigtail. Start by stripping back 2 inches of the outer sheath using either techinique shown in [this guide](https://bluerobotics.com/learn/cable-stripping/) and also strip and twist each conductor about 1/4". Using [these connectors](https://www.digikey.com/en/products/detail/adafruit-industries-llc/5619/17506672) we can quickly make a harness to test what a full topside connection would be like, minus any other connectors between the topside/operator and ISMS and length of wire in-between.

![Test Harness S1](<./Build Images/External/Test Harness S1.jpg>)Connect a quick connector to each of the wires of the 16 pin subcon pigtail following the left column color codes (or 'ISMS Housing Side') of the  [SubCon Octopus Cable Key.pdf](<./SubCon Octopus Cable Key.pdf>) wiring chart

> [!TIP]
>
> It can be helpful to number each quick connector with a sharpie to avoid having to refer to the color chart every time.

![ISMS Recommended Topside Wiring](<./Build Images/Topside/ISMS Recommended Topside Wiring.svg>)

Next connect the quick connectors to the wires from the connectors made in the previous section following the overall wiring chart (or combining the powers and grounds to one power supply if that is what is available).

![Test Wiring Quick Harness](<Build Images/External/Test Wiring Quick Harness.jpg>)

> [!NOTE]
> This photo shows a slightly different wiring layout/schema than the recommended topside wiring diagram above; however, it works well if you only have one (250w or more) power supply to test with! It also has the subconn for the fluid pump attached.

This is a great oportunity to finally test all the systems of the ISMS and make sure everything is tight! If you are having trouble with the RGA or main Serial connection, check the internal connector wiring and/or install a null modem adapter if you didin't correctly cross the RX and TX wires at the topside connector. You can test the RGA by plugging a full off-the-shelf DB9 Serial cable between the RGA and your laptop usb-to-rs232 connector in a pinch.

![Test Wiring Quick Harness](<Build Images/External/Test Harness S2.jpg>)

Next add the Fluid pump (and optional pH probe subconn) following the same [SubCon Octopus Cable Key.pdf](<./SubCon Octopus Cable Key.pdf>) color coding where the color code on the 16 pin connector on the left hand side of the chart are joined to to the wire color code of the fluid pump (and pH) subcons on the right hand side of the chart.

> [!NOTE]
>
> The fluid pump and pH probe subconns expose more pins than can map to the 16 pin ISMS Main Housing subcon. This was done intentionally so a future revision could change which features of the fluid pump are exposed by chaninging the octopus cable to dedicate more pins to the fluid pump or pH probe respectively. Any unterminated wires should be capped/taped to prevent shorts and left unconnected to anything.

### Potting

Use the [molds](https://www.seascapesubsea.com/product/reusable-flexible-casting-molds/?attribute_configuration=1x+%E2%8C%808mm+to+3x+%E2%8C%808mm) and [mold clamps](https://www.seascapesubsea.com/product/casting-mold-clamps/?attribute_size=Standard). Put silicone putty around the outside of the mold and cables to seal any cracks and openings in the sides of the mold!

**Octopus cable potting checklist**:

- [ ] Wire heat shrink slid on each conductor and then soldered
- [ ] Free floating conductors are capped/taped.
- [ ] Correct wire continuity verified.
- [ ] Heat shrink shrunk.
- [ ] Wires taped around central rubber core to provide separation & rigidity.
- [ ] Correct locking collars slid on subconn ends.
- [ ] Cable outer surfaces prepped & sanded.
- [ ] All wires in mold and outer square mold clamps installed.
- [ ] Potential potting drip gaps sealed with silicone non-drying putty and area prepared for pouring.
- [ ] Wires centered in mold cavity (not too near edges or top or bottom).

## Next Build Section: [Fluid Pump](<../Fluid Pump/README.md>)





## Appendix

A full wiring diagram can be found here:
