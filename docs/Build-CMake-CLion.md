Build with CMake and CLion
==========================

Thanks to @The_g00d and @fkcoder for some help.

1. Create and configure various toolchains as in the image below.

    ![CMake & CLion Toolchains](../images/CMake_CLion_Toolchains.png)

    File => Settings => Build, Execution, Deployment => Toolchains.

2. Open or create project and choose proper toolchain, then configure project as in the image below.

    ![CMake & CLion Settings](../images/CMake_CLion_Settings.png)

    File => Settings => Build, Execution, Deployment => CMake.

3. Add Custom Compiler scheme `custom_compiler_ADS.yaml` from `C:\ARM\cmake\` or `/opt/arm/cmake` directories.

    ![CMake & CLion Custom Compiler](../images/CMake_CLion_Custom_Compiler.png)

    File => Settings => Build, Execution, Deployment => Toolchains => Custom Compiler.

4. Reset CMake cache and Reload project.

   Use this function every time you change the toolchain, also check/uncheck "Custome Compiler" option, it only makes sense for ADS compiler.

    Tools => CMake => Reset Cache and Reload Project.

5. Build ELF executable.

    Build => Rebuild project.
