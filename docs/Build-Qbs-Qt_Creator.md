Build with Qbs and Qt Creator IDE
=================================

Thanks to [@denis-shienkov](https://github.com/denis-shienkov) | [@kuzulis](https://www.linux.org.ru/people/kuzulis/profile) for using Qbs manual.

1. Add the `tcc` or `tcc.exe` as a custom C language compiler from the "Edit => Preferences => Kits => Compilers" with the "Thumb C Compiler, ADS1.2 [Build 848]" name.
2. Change the ABI to "arm-unknown-unknown-unknown-unknown".
3. Set the error parser as "GCC".
4. Click on "Apply" button.
5. Go to "Edit => Preferences => Kits => Kits` tab and add a new kit named as "Motorola P2K ARM".
6. Configure the "Motorola P2K ARM" kit with the following properties:

   * Device type: "Desktop".
   * Device: "Local PC".
   * Build device: "Local PC".

   (or you can choose the bare metal device instead, it is irrelevant).

   * Compiler C: "Thumb C Compiler, ADS1.2 [Build 848]".
   * Compiler C++: "<No compiler>".
   * Debugger: "None".
   * Qt: "None".

7. Click on "Additional Qbs Profile Settings" change button and add the following additional properties:

   * qbs.architecture: `arm`
   * qbs.toolchainType: `tcc`
   * preferences.qbsSearchPaths: `C:\ARM\qbs` or `/opt/arm/qbs`

8. Check that the Qbs propertires are set successfully, goto "Edit => Preferences => Qbs => Profiles", choose the "Motorola P2K ARM" profile and expand items. Check that there are the following:

   * for `cpp` entry:
	   * cCompilerName: `"tcc.exe"`
	   * compilerName: `"tcc.exe"`
	   * toolchainInstallPath: `"C:/ARM"`

   * for `preferences` entry:
	   * qbsSearchPaths: `"C:\\ARM\\qbs"`

   * for `qbs` entry:
	   * architecture: `"arm"`
	   * targetPlatform: `""`
	   * toolchainType: `"tcc"`

9. Just open the "P2kElfs.qbs" project file from the Qt Creator IDE assign the "Motorola P2K ARM" kit, choose the shadow build directories (Release only is OK), and then press the "Build" button.

10. Right now an example of the command line flags are following:

```
C:\ARM\tcc.exe -c C:/Users/exlmo/Desktop/P2kElfs/Screenshot/Screenshot.c -o C:/Users/exlmo/Desktop/build-P2kElfs-Motorola_P2K-Release/Release_Motorola_20544e913c3f0fb1/Screenshot.60c152d9/3a52ce780950d4d9/Screenshot.c.o -D__P2K__ -IC:/ARM/SDK -I/opt/arm/SDK -bigend -apcs /interwork
C:\ARM\armlink.exe -nolocals -reloc C:/Users/exlmo/Desktop/build-P2kElfs-Motorola_P2K-Release/Release_Motorola_20544e913c3f0fb1/Screenshot.60c152d9/3a52ce780950d4d9/Screenshot.c.o -first "Lib_L2_L6.o(Lib)" C:/ARM/lib/Lib_L2_L6.o -o C:/Users/exlmo/Desktop/build-P2kElfs-Motorola_P2K-Release/Release_Motorola_20544e913c3f0fb1/Screenshot.60c152d9/Screenshot.elf

/opt/arm/bin/tcc -c /home/exl/Storage/Projects/Git/Public/P2kElfs/Screenshot/Screenshot.c -o /home/exl/Storage/Projects/Git/Public/build-P2kElfs-Motorola_P2K_ARM-Release/Release_Motorola_042846ed32005607/Screenshot.60c152d9/3a52ce780950d4d9/Screenshot.c.o -D__P2K__ -I/opt/arm/bin/SDK '-I/home/exl/Storage/Projects/Git/Public/P2kElfs/Screenshot/C:\ARM\SDK' -I/opt/arm/SDK -bigend -apcs /interwork
/opt/arm/bin/armlink -nolocals -reloc /home/exl/Storage/Projects/Git/Public/build-P2kElfs-Motorola_P2K_ARM-Release/Release_Motorola_042846ed32005607/Screenshot.60c152d9/3a52ce780950d4d9/Screenshot.c.o -first 'Lib_L2_L6.o(Lib)' /opt/arm/lib/Lib_L2_L6.o -o /home/exl/Storage/Projects/Git/Public/build-P2kElfs-Motorola_P2K_ARM-Release/Release_Motorola_042846ed32005607/Screenshot.60c152d9/Screenshot.elf
```
