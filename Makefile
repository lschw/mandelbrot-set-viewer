viewer: MandelbrotSet.cc MandelbrotSet.hh viewer.cc
	g++ -Wall -Wextra -Wunused -pedantic -std=c++11 MandelbrotSet.cc viewer.cc -lGL -lGLU -lglut -pthread -o mandelbrot 
