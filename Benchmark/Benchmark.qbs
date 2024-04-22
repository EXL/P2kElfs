Application {
	Depends { name: "sdk" }
	files: [
		"Benchmark.c",
		"FireEffect.c",
		"Phases.c",
		"delay_armv4t_ADS.S",
		"delay_armv4t_GCC.S",
		"delay_mcore340_GCC.S",
		"dhry_1.c",
		"dhry_2.c"
	]
	cpp.defines: [ "__P2K__", "EP1", "MCORE_BOGOMIPS" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
