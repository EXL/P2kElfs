Application {
	Depends { name: "sdk" }
	files: [ "data.c", "draw.c", "yeti.c", "Yeti3D.c" ]

	cpp.defines: [ "EP1", "ROT_0", "FPS_30" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
