Application {
	Depends { name: "sdk" }
	files: [ "data.c", "draw.c", "yeti.c", "Yeti3D.c" ]

	cpp.defines: [ "EP1", "ROT_0", "FPS_30", "VIEWPORT_WIDTH=82", "VIEWPORT_HEIGHT=122" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
