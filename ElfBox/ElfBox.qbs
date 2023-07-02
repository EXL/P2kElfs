Application {
	Depends { name: "sdk" }
	files: [ "ElfBox.c" ]

	cpp.defines: [ "EP1", "FTR_V300" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
