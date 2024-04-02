# tiltfive-diagnostic-cpp

This project uses v1.4.1 of the Tilt Five drivers and NDK.

If you experience errors, make sure the following are correct:
  * the `tiltfive-diagnostic-cpp\src\lib\win\x86_64\TiltFiveNative.dll` has been added to the same folder as your `.exe`
	file (i.e. the `tiltfive-diagnostic-cpp\x64\Debug` folder)
  * the solution properties linker is pointing to the `TiltFiveNative.dll.if.lib` file
