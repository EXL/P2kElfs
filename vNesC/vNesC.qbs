Application {
	Depends { name: "sdk" }
	files: [ "vNesC.c", "vP2k.c" ]

	cpp.defines: [ "EP2", "ROT_0", "FPS_30" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib", "armlib/armlib"]
}
