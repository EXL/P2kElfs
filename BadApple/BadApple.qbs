Application {
	Depends { name: "sdk" }
	files: [ "BadApple.c" ]

	cpp.defines: [ "EP1", "FPS_30" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
