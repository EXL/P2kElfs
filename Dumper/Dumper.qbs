Application {
	Depends { name: "sdk" }
	files: [ "Dumper.c" ]

	cpp.defines: [ "EP1", "USE_MME" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
