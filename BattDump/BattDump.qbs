CppApplication {
	Depends { name: "sdk" }
	files: [ "BattDump.c" ]
	cpp.staticLibraries: ["Lib_L2_L6"]
}
