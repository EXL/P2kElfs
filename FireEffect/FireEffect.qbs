Application {
	Depends { name: "sdk" }
	files: [ "FireEffect.c" ]

	cpp.defines: [ "EP1", "ROT_90_DEG_LANDSCAPE", "FPS_15", "FPS_METER" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
