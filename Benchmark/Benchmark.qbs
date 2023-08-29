Application {
	Depends { name: "sdk" }
	files: [ "Benchmark.c", "Phases.c" ]
	cpp.defines: [ "EP1" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
