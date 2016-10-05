add_executable(capture
	capture.cpp
	KinectCapture.cpp)

UseBoost(capture)
UseFreenect(capture)
UseTiff(capture)
UseOpenCV(capture)

