Application {
	Depends { name: "sdk" }
	files: [ "Overclock.c", "Clock.c" ]
	cpp.defines: [ "EP1" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
