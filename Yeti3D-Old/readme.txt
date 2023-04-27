The Yeti 3D Engine - Wednesday August 13th, 2003
================================================

Information
===========

This is the second public release of the newly named "Yeti 3D Engine". The 
Yeti engine is a fully featured 3d engine designed for low cost portable devices, 
namely the GameBoy Advanced, mobile phones and Palm units. It should therefore 
run on just about anything.

Currently, the code is not for commercial usage or distribution, but that
will change when the source code is stable and I organise a license. 

Feel free to read, compile and use the code for your own personal enjoyment.

How to compile
==============

You will need to download and install GCC for the GBA. I currently use 
DevKit Advanced: http://devkitadv.sourceforge.net/index.html

Once you have GCC installed, run the make.bat script to create a new build.

I have GCC installed in the folder "C:\devkitadv\", so if you install somewhere
else then you will need to change the make.bat script a little.

Contacts
========

If you would like a copy of the latest code or to ask what my future plans are,
email myself at:

annikapaige@bigpond.com

At some stage I'd like to optimize the texture mapper for the GBA, but that will
be after im 100% sure the C++ is as fast as it can be.

Contact me if you would like to contribute to the project. The latest version can be
downloaded from:

http://www.theteahouse.com.au/gba/index.html

Engine Technical Notes
======================

These are design notes for the Yeti engine. Other engines use very different algorithms.
Im still refining the engine to improve quality while maintaining a acceptable rendering
speed.

- The current viewport is 120x80 pixels, 15bit.
- All textures are 64x64 8bit. 
- Textures are converted to 15bit via a pre-calculated lighting LUT.
- Polygons can be any convex shape. Only squares are currently used.
- Each vertex is described as X, Y, Z, U, V and brightness.
- The renderer uses 24:8 fixed point maths.
- Polygons are clipped in 3D space using 45 degree planes. Distance to plane
  calculations therefore use only additions and subtractions.
- Polygon edges are clipped using one divide and 6 multiplies.
- 4 clipping planes are used. No front plane is required. No back plane is used.
- No per-span clipping is used. Fixed point errors are hidden offscreen.
- Ray-casting is used to build a visablity list and valid polygon rendering order.
- Models are merged into the VIS without sorting.
- No Z-buffers are used. Rendering is back-to-front (painters algorithm).
- The is an acceptable level of overdraw. Complete polygons are culled. Polygon edges
  are drawn faster than using a per span clipper. 
- Lighting is pre-calculated on startup. Lighting can be moved at runtime.
- Lighting is expanded per vertex and interpolated along polygon edges.
- A reciprocal table is used to eliminate all divides from the DDA texture mapper.
- The affine texture loop is unrolled and renders blocks of 32 pixels.

Kinda Regards
Derek Evans
Wednesday August 13th, 2003
