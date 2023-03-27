Application {
	Depends { name: "sdk" }
	files: [ "Spout.c", "Piece.c" ]

	cpp.defines: [ "EP1", "ROT_90", "FPS_30" ]
	cpp.optimization: "O2"
	cpp.staticLibraries: ["Lib"]
}
