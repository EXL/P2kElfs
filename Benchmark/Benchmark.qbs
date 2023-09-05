Application {
	Depends { name: "sdk" }
	files: [
		"Benchmark.c",
		"Delay_armv4t_ADS.s",
		"Delay_armv4t_GCC.s",
		"Phases.c",
		"dhry_1.c",
		"dhry_2.c",
		"FireEffect.c",
	]
	cpp.defines: [ "__P2K__", "EP1", "PALMOS_BOGOMIPS" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
