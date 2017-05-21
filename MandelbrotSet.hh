/*
 * Mandelbrot set viewer
 * Copyright (C) 2014 Lukas Schwarz
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http: *www.gnu.org/licenses/>.
 */
#ifndef MANDELBROTSET_HH
#define MANDELBROTSET_HH

#include <complex>
#include <pthread.h>

typedef long double dtype;
typedef std::complex<dtype> cdtype;

/**
 * Definition of the Mandelbrot set:
 * 
 * Consider sequence
 * 
 * z_{n+1} = (z_n)² + c
 * z_0 = 0
 * 
 * The Mandelbrot set are all complex values c, for which z_{n} stays finite
 * for n -> inf
 * 
 */
class MandelbrotSet
{
    public:
        
        /**
         * Setup Mandelbrot set calculator
         */
        MandelbrotSet(size_t iter_max, size_t width, size_t height,
            dtype center_x=-0.75, dtype center_y=0, dtype unit=1, dtype zoom=1,
            size_t threads=4);
        
        ~MandelbrotSet();
        
        
        /**
         * Set new image size
         */
        void set_size(size_t width, size_t height);
        
        
        /**
         * Set new view size
         */
        void set_view(dtype center_x, dtype center_y, dtype zoom);
        
        
        /**
         * Set new maximum iteration count
         */
        void set_iter_max(size_t iter_max);
        
        
        /**
         * Set new threads count
         */
        void set_threads(size_t threads);
        
        /**
         * Set new color mode
         */
        void set_color_mode(size_t c);
        
        /**
         * Calculate and return image with the currently set parameters
         */
        const unsigned char* calc();
        
        /**
         * Access methods for class properties
         */
        size_t get_iter_max() const { return this->iter_max; }
        size_t get_width() const { return this->width; }
        size_t get_height() const { return this->height; }
        dtype get_center_x() const { return this->center_x; }
        dtype get_center_y() const { return this->center_y; }
        dtype get_zoom() const { return this->zoom; }
        dtype get_coord_fac() { return this->x_step; }
        dtype get_color_mode() { return this->color_mode; }
        dtype get_threads() { return this->threads; }
        
    
    private:
        // maximum iteration count ( = infinity)
        size_t iter_max;
        
        // width/height in px of image
        size_t width;
        size_t height;
        
        // values of c which mark the centrum of the image
        dtype center_x;
        dtype center_y;
        
        // unit of which corresponds to the width of the image
        dtype unit;
        
        // zoom factor of the unit
        dtype zoom;
        
        // scaled unit of c in x/y direction
        dtype x_unit;
        dtype y_unit;
        
        // value of c at the left border
        dtype x_min;
        dtype y_min;
        
        // one pixel in x/y direction corresponds to x_step/y_step
        dtype x_step;
        dtype y_step;
        
        // internal flag, whether image has to be recalculated
        // (e.g. if parameters have changed)
        bool recalc;
        
        // flag indicating the color mode
        // 0 = greyscale
        // 1 = inverted
        // 2 = color
        size_t color_mode;
        
        // rgb image data
        unsigned char* data;
        
        // threads
        pthread_mutex_t mutex;
        size_t threads;
        struct thread_param
        {
            MandelbrotSet* mb;
            size_t n;
        };
        
        /**
         * Thread callback which performs the actual mandelbrot set iteration
         */
        static void* calc_thread(void* param);
    
};

#endif
