Application {
	Depends { name: "sdk" }
	files: [
		"Benchmark.c",
		"Phases.c",
		"dhry_1.c",
		"dhry_2.c",
		"FireEffect.c"
	]
	cpp.defines: [ "__P2K__", "EP1" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
