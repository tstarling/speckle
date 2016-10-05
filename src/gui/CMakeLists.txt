add_executable(gui
	gui.cpp
	MainWindow.cpp
	${SPECKLE_SOURCES})

UseOpenCV(gui)
UseQt(gui)
UseFreenect(gui)
UseThreads(gui)
