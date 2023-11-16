Application {
	Depends { name: "sdk" }
	files: [ "BadApple.c" ]

	cpp.defines: [ "EP1", "ROT_90", "FPS_15" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
