Application {
	Depends { name: "sdk" }
	files: [ "draw.c", "game.c", "yeti.c", "main_p2k.c" ]

	cpp.defines: [ "EP1", "ROT_0", "FPS_30" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib", "armlib/armlib"]
}
