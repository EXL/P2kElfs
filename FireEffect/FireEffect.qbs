Application {
	Depends { name: "sdk" }
	files: [ "FireEffect.c" ]

	cpp.defines: [ "EP1", "ROT_0", "FPS_30", "FPS_METER" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
