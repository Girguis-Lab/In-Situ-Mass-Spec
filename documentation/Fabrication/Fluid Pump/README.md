# Oil Compensated Fluid Pump - Build Guide

![Finished Fluid Pump](<./Images/Finished Fluid Pump.jpg>)

CAD Here: [Link](<https://cad.onshape.com/documents/9e45e871d74b8539d6687f02/w/a3384ad6c22d56b5ea88b888/e/1e5f54e24d5f977d3ebdd65c?renderMode=0&uiState=6a5fcc581d87097ece0e154f>)

### Parts

 listed in Fluid Pump section within the overall parts list for the ISMS

3D print the following part:

- [Fluid Pump Mount - Pump Mount for BR 3- Housing.stl](<./3D Print Models/Fluid Pump Mount - Pump Mount for BR 3- Housing.stl)

Laser Cut the following parts from the 23/64" thick accrylic sheet:

-  (Optional Jig x1)  [BR end cap drilling guide jig top plate.dxf](<./Laser Cut Designs/BR end cap drilling guide jig top plate.dxf)

- (Optional Jig x1)  [BR end cap drilling guide jig bottom plate.dxf](<./Laser Cut Designs/BR end cap drilling guide jig bottom plate.dxf)

- (Optional Jig x2)  [BR end cap drilling guide jig spacer plate.dxf](<./Laser Cut Designs/BR end cap drilling guide jig spacer plate.dxf)

-  [WTE3-P-END-CAP-R1 - End Cap Plate W Diaphragm Opening.dxf](<../Laser Cut Designs/Fluid Pump/WTE3-P-END-CAP-R1 - End Cap Plate W Diaphragm Opening.dxf)

- [Fluid Pump End Cap O-Ring Groove.eps](<Laser Cut Designs/Fluid Pump End Cap O-Ring Groove.eps>)

  - Special Note: This end cap requres laser engraving or machining out the o-ring groves to a depth of approximately 1.9 mm or 3D printing the matching 3d model: [Customized_WTE3-M-END-CAP-4-HOLE-R1 - Fluid Pump End Cap plate W penetrator holes.stl](<Laser Cut Designs/Customized_WTE3-M-END-CAP-4-HOLE-R1 - Fluid Pump End Cap plate W penetrator holes.stl>)  with a watertight but durable method like SLA printing with a non-brittle resin.

    On a 75 watt Universal Systems CO2 Laser Cutter these parameters worked best - you will likely need to experiment with your laser cutter to get appropriate results before making a full cutout:

    ​	raster engrave settings for 1.9mm deep cut: 19% Power, 15% Speed, 1000 PPI, 0.359" z-axis height.

    See the original CAD for the desired result (end cap shown in black): https://cad.onshape.com/documents/9e45e871d74b8539d6687f02/w/a3384ad6c22d56b5ea88b888/e/1e5f54e24d5f977d3ebdd65c

### Cut the diaphragm

Cut the diaphram from the Buna-N rubber 1/8" thick sheet either with a water jet or using the jigs:

- Cut out of the rubber sheet a rough circle larger than the final shape with an exacto knife.
- Clamp the rubber circle in a stack consisting of the jig stack cut out of 1/4" thick accrylic: Drilling guide template bottom plate > Bunna Rubber sheet > Drilling guide template spacer plate (x2) > Drilling guide template top plate
- Screw together jig with 8-32 x 2" screws and nuts and clamp in a vice
- Drill holes with a 1/8" extended shank drill bit or 1/8" milling drill bit
- Cut exact outer perimeter with an Exacto knife

### Make diaphragm end cap

1. Bevel the inner edge of the Accryclic end cap plate with diaphragm opening using an exacto knife so the diaphragm will not rub against a sharp edge when expanding.
2. Sandwich the diaphragm against the Blue Robotics black aluminum end cap flange along with the face seal o-ring included w the blue robotics flange using 6 m3 x 16mm screws.
   ![Fluid Pump Pressure Compensation Diaphgram](<./Images/Fluid Pump Pressure Compensation Diaphgram.png>)

### Swap micropump factory case for 3D-Printed mount

Remove the micropump-provided housing by unscrewing the two screws on the pump head and firmly twist the pump head to release it from the plastic housing. Pinch and remove the grommet so the wires can come throught the port in the housing. Slide the Micropump electronics into the 3D printed mount, aligning the screw holes as shown. Re-attach the pump head and reinstall the screws holding the pump head to the electronic drive.

![Micropump Mounting Sandwich](<./Images/Micropump Mounting Sandwich.jpeg>)

![Mcropump Mounting Sandwich Screws](<./Images/Mcropump Mounting Sandwich Screws.jpeg>)

### Install penetrators in oposite end cap.

Assemble the WTE based on the BR guide: https://bluerobotics.com/learn/wte-vent-assembly-guide/

All penetrators should be oriented witht the nut on the inside surface and the o-ring on the outside surface of the accrylic sheet with a thin layer of molykote 111 or 44 medium grease on the o-rings

The fluid path bulkheads should be installed just like  the other bulkheads, except the o-ring fits into the groove in the acrylic not the bulkhead.

Tighten all penetratrators firmly like described in the blue robotics WTE guide

Use m3 Screws to fasten the end cap onto the flange with the face o-ring (lightly greased)

![Penetrator End Cap](<./Images/Penetrator End Cap.jpg>)

4. Splice electrical connections

Following the wire color chart for the external side fluid pump in the [ISMS V3 SubCon Octopus Cable Key.pdf](<../Wiring/ISMS V3 SubCon Octopus Cable Key.pdf), splice each wire from the fluid pump to the Subconn Bulkhead with individual heat shink to prevent shorts. You can unplug the wires from the pump drive electronics for soldering. Tie back the unused wires and string using heat shrink and seal with glue to avoid shorts.

Pass cable through tubing and reconnect to pump drive connector on other end. Connect pump to diapragm end cap flange screw holes with 2 M3 x 16mm screws in the pocket holes.

![Wiring Splice](<./Images/Wiring Splice.jpeg>)

 ![Micropump Wire Join](<./Images/Micropump Wire Join.jpeg>)

![Wire Connection](<./Images/Wire Connection.jpeg>)

![Pump Pocket Screws](<./Images/Pump Pocket Screws.jpeg>)

### Connect fluid path tubing to Micropump

Wrap the two 90* Elbow fittings with 1/4" standard thickness (non-ss) teflon tape for 1 & 3/4 wraps each. Screw into pump inlet & outlet until each are facing straight relative to the central axis of the Blue Robotics housing. If tightening becomes difficult or is too loose at this angle, try a different number of teflon tape wraps.

Attach two 7" long sections of 1/8" OD Nylon tubing (in red color here) to the ports using swagelok nylon ferrules instead of the stainless steel ones from the fittings.

![Micropump NPT Elbow Fitting Orientation](<./Images/Micropump NPT Elbow Fitting Orientation.jpeg>)

Connect to swagelok bulkheads on end cap, again swapping plastic ferrules (and/or nuts) for the stainless steel ones from the swagelok bulkheads.

Install Locking cords

### Test

Connect subconn to main ISMS bottle and test Fluid pump with water.

### Mineral Oil Fill

Fill the pump housing with oil using a vacuum fill if possible.
Vacumm connection instructions using a hand vacuum pump: https://bluerobotics.com/learn/using-the-vacuum-test-plug/#testing-the-test-setup

To do a vacuum fill you'll need a tubing, a secondary vacuum-safe sealed jar or container with two tubes to act as a oil reservoir and trap and a vacuum pump .

1. Fill the housing about 3/4 with mineral oil & then close and seal the housing.
2. Fill the trap container with enough oil to fill the remaining volume inside the housing without covering the outlet tube.

2. Connect a tube from the pressure relief valve with the red backfill adapter screwed on and the Vacuum plug in place to the oil reservoir / trap so the tube sits at the bottom in the oil in the trap.
3. Connect the other tube from the headspace of the trap to the vacuum pump.
4. Pump a vacuum in trap - which should pull a sufficient vacuum in the enclosure as well
5. wait for degassing to occur.
6. Re-pressurize the vacuum line slightly to force the oil into the enclosure with slight positive pressure so the diaphragm bows outwards .
7. Unscrew the backfill adapter to seal the oil in - some may dribble out.

![Vaccum Fill Setup](<./Images/Vaccum Fill Setup.jpg>)

## Next Build Section [pH Probe](<../pH Sensor/README.md>)
