# Main Sled Build

The ISMS is built onto a sled that slides into the main housing and is attached to the housing endcap with the inlet and other ports.

### Assembling Inlet

Parts required:

- Inlet Base (Ti)
- Inlet cap (Ti)
- Frit
- Membrane Material (backed teflon or mesh reinforced PDMS)

Tools Requried:

* 16mm or 5/8" hollow cuttout tool and hard teflon "cutting board"
* Hammer or cutting press.

Cut out a disk from the membrane using the cutout tool (or laser cutter for PDMS composite membranes - ask kyle for laser cutter settings used)



![p00001](<./Build Images/Inlet Assembly/p00001.png>)

Screw the 1/4" NPT to Swagelok 1/4 tube fittings into the inlet top cap fluid ports with ~2 wraps of standard PTFE pipe wrap tape on each fitting.

![p00002](<./Build Images/Inlet Assembly/p00002.png>)

Lightly grease o-rings and Add to their respective grooves (o-ring sizes smallest to largest:  TODO fill in )

Add frit in center of inlet base, taking note of any bevel on the edge of the frit, which should go DOWN if present so the surface of the frit and machined base surface (shown in blue) are as flush as possible.

![p00002A](<./Build Images/Inlet Assembly/p00002A.png>)

Another view of this step^



![p00003](<./Build Images/Inlet Assembly/p00003.png>)

![SideView-Inlet](<./Build Images/Inlet Assembly/SideView-Inlet.jpg>)

Sandwich the Membrane on top of the frit - centered so it does not touch the greased o-rings - it should extend past the edges of the frit slightly. Holding the inlet cap above the base - align the screws and begin tightening - the grease of the smallest inner o-ring should allow it to cling to the groove in the underside of the cap. When you have the screws aligned and partially screwed in - slide down the cap onto the base checking that all o-rings stay centered in the groves and not pinched - in this way the smallest o-ring should be centered on the membrane without shifting.



![p00004](<./Build Images/Inlet Assembly/p00004.png>)

Screw the BSPP to 1/8" tubing (yorlock brand) fiting into the vacuum side port and tighten so the rubber gasket makes a good seal.

> [!IMPORTANT]
>
> Since this side will be part of the vacuum chamber, wear gloves and avoid getting any residues on the interior of the inlet or fittings, also do NOT use Teflon tape as this is not a tappered NPT fitting.

![p00005](<./Build Images/Inlet Assembly/p00005.png>)

Cut a roughly 101mm length of the PEEK plastic 1/8" OD tubing - this will serve as electrical isolation between the internal sled/vacuum chamber ground and the metal of the inlet/main housing/ocean. This isolation is nescesary for some host sub vehicles to detect electrical problems.

![p00006](<./Build Images/Inlet Assembly/p00006.png>)

Connect the PEEK tubing to the inlet adapter fitting using brass swagelok ferrules (brass or stainless steel nut is ok and Yorlock is cross compatible with swagelok components/ferrules)

> [!IMPORTANT]
>
> While 1/8" swagelok/yorlock fittings should be tightened 3/4" turn past "finger tight" (See [Swagelok Fitting Installation](https://stavanger.swagelok.com/en/resources/tube-fitting-installation)), PEEK tubing is especially fragile, so err on the side of less tight and tighten as needed once you are able to test the capped vacuum performance. Replace if overtightened or visibly strained and try again with new brass ferrules - the brass is slightly softer than stainless ferules so that should help with over-tightening.

### Assembling Primary Housing Endcap

![p00001](<./Build Images/Endcap Assembly/p00001.png>)

Screw in prevco dual seal vent plug ensuring both o-rings are present and lightly greased.

![p00002](<./Build Images/Endcap Assembly/p00002.png>)

Screw in zincoid sacrificial anode (Do not use tef-gel or other insulating material between anode and endcap threads as this defeats the purpose)

![p00003](<./Build Images/Endcap Assembly/p00003.png>)

Screw in Subcon 16pin male bulkhead with red FEMALE locking sleeve.
Make sure the o-ring provided with the Subcon bulkhead is lightly greased and dust/hair free before inserting.

Fill the 16 pin dummy plug holes with some Molykote 44 Medium grease and push onto the bulkhead to protect the bulkhead pins.

![p00004](<./Build Images/Endcap Assembly/p00004.png>)

Apply teff-Gel to the surfaces marked in teal  (but not inside the o-ring grooves) to prevent galvanic corrosion/electron exchange between the inlet base and the main enclosure end cap while in seawater.

![p00005](<./Build Images/Endcap Assembly/p00005.png>)

Add lightly greased o-rings to inlet avoiding getting tef-gel on the o-rings.

![p00006](<./Build Images/Endcap Assembly/p00006.png>)

Apply a generous amount of teff-Gel to the threads of the 1/4-20 x 1" bolts with washers and affix to main end cap, making sure tubing ports are facing in a direction suitable to route the external fluid path.



### Assembling Sled Chassis

Machined Parts required:

- 4x rails (Machined brass)
- 1x electronics base plate (Machined brass)
- 2x center chassis spacer disks  (machined black Delrin plastic)
- 2x end chassis spacer disks  (machined black Delrin plastic)

![p00001](<./Build Images/Sled Assembly Steps/p00001.png>)



![p00002](<./Build Images/Sled Assembly Steps/p00002.png>)



![p00003](<./Build Images/Sled Assembly Steps/p00003.png>)

The four bottom holes in the roughing pump will need to be tapped with an 10-32 thread tap to accept the screws. Apply a small amount of Swagelok VacGoop to the BSPP threads of the BSPP to YORLOK fitting and screw into inlet port of the roughing pump following BSPP fastening norms (screw till mostly tight then finish by screwing down nut over o-ring flange)

![p00004](<./Build Images/Sled Assembly Steps/p00004.png>)

![p00005](<./Build Images/Sled Assembly Steps/p00005.png>)

![p00006](<./Build Images/Sled Assembly Steps/p00006.png>)

![p00007](<./Build Images/Sled Assembly Steps/p00007.png>)

![p00008](<./Build Images/Sled Assembly Steps/p00008.png>)

![p00009](<./Build Images/Sled Assembly Steps/p00009.png>)

Screw in terminal blocks to laser cut mounting plate with M2.5 x 16mm screws and lock nuts, then ziptie plate to rails centered over the large flange of the turbo pump. The terminal slot marked "1" on the laser cut mount plate should be on the left side in this photo.

![p00010](<./Build Images/Sled Assembly Steps/p00010.png>)

Attach main end cap with 5 screws to the end ring (Note how these screws screw INTO the endcap from the backside of the end ring).

![p00011](<./Build Images/Sled Assembly Steps/p00011.png>)

Completed sled.

### Connecting Inlet to Vacuum

As of writing this part was the least settled. Two options exist:

- A) Add a lenth of small diameter capilary tubing between the inlet and vacuum chamber to reduce the total gas load on the RGA and hopefully achive tighter mass spectra peeks
- B) Go straight from inlet to vacuum chamber side port

In either case, the length of PEEK tubing that provides electrical insulation must still be there along with a union Swagelok tube fitting to connect the PEEK tubing to the capilary or a length of stainless tubin that can take a tighter bend than the pure PEEK tubing to go into the vacuum chamber side port.

Option A Shown here:

![Option A](<./Inlet Connection/Option A.png>)

> [!TIP]
>
> Cap the vacuum chamber for now at the side port using a swagelok fitting plug. Once you have established that the capped vacuum is tight, then go back and finnish installing this connection

## Next build section: [Wiring](<../Wiring/README.md>)
