Application {
	Depends { name: "sdk" }
	files: [ "Spout.c", "Piece.c" ]

	cpp.defines: [ "EP1" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
