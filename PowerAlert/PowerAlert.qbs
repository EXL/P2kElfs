CppApplication {
	Depends { name: "sdk" }
	files: [ "PowerAlert.c" ]
	cpp.staticLibraries: ["Lib_L2_L6"]
}
