CppApplication {
	Depends { name: "sdk" }
	files: [ "VibroHaptic.c" ]
	cpp.staticLibraries: ["Lib_L2_L6"]
}
