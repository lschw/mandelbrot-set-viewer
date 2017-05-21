# Mandelbrot set viewer
This is a small tool to visualize the Mandelbrot set rendered in real time with
OpenGL.

## Dependencies
* libgl
* glu
* freeglut

## Usage

    mandelbrot [threads]
    
    [threads] = thread count, default = 4

## Control
* click and move mouse to adjust view area
* scroll mouse wheel: zoom in/out (in autozoom mode: increase/decrease zoom speed)
* +/- key: increase/decrease iteration count
* c key : cycle through different color modes
* r key : reset all settings
* z key : toggle auto zoom mode
