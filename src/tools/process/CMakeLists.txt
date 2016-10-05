add_executable(process
	process.cpp
	${SPECKLE_SOURCES})

UseBoost(process)
UseTiff(process)
UseOpenCV(process)

