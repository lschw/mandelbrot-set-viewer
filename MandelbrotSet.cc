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
#include "MandelbrotSet.hh"

/**
 * Setup Mandelbrot set calculator
 */
MandelbrotSet::MandelbrotSet(size_t iter_max, size_t width, size_t height,
    dtype center_x, dtype center_y, dtype unit, dtype zoom, size_t threads
) : iter_max(iter_max), width(width), height(height), center_x(center_x),
    center_y(center_y), unit(unit), zoom(zoom), recalc(true), color_mode(0),
    data(0), mutex(PTHREAD_MUTEX_INITIALIZER), threads(threads)
{
    set_size(width, height);
}


/**
 * Free internal memory
 */
MandelbrotSet::~MandelbrotSet()
{
    if(data != 0) {
        delete[] data;
    }
}

/**
 * Set image size
 */
void MandelbrotSet::set_size(size_t width, size_t height)
{
    pthread_mutex_lock(&this->mutex);
    if(data != 0) {
        delete[] data;
    }
    this->width = width;
    this->height = height;
    data = new unsigned char[width*height*3];
    this->recalc = true;
    pthread_mutex_unlock(&this->mutex);
}


/**
 * Set image view
 */
void MandelbrotSet::set_view(dtype center_x, dtype center_y, dtype zoom)
{
    pthread_mutex_lock(&this->mutex);
    this->center_x = center_x;
    this->center_y = center_y;
    this->zoom = zoom;
    this->recalc = true;
    pthread_mutex_unlock(&this->mutex);
}


/**
 * Set color mode
 */
void MandelbrotSet::set_color_mode(size_t c)
{
    pthread_mutex_lock(&this->mutex);
    this->color_mode = c;
    this->recalc = true;
    pthread_mutex_unlock(&this->mutex);
}

/**
 * Set maximum iteration count
 */
void MandelbrotSet::set_iter_max(size_t iter_max)
{
    pthread_mutex_lock(&this->mutex);
    this->iter_max = iter_max;
    this->recalc = true;
    pthread_mutex_unlock(&this->mutex);
}


/**
 * Set threads count
 */
void MandelbrotSet::set_threads(size_t threads)
{
    pthread_mutex_lock(&this->mutex);
    this->threads = threads;
    pthread_mutex_unlock(&this->mutex);
}

/**
 * Calculate and return image with the currently set parameters
 */
const unsigned char* MandelbrotSet::calc()
{
    pthread_mutex_lock(&this->mutex);
    
    // only perform calculation if needed
    if(!this->recalc) {
        pthread_mutex_unlock(&this->mutex);
        return data;
    }
    
    // calculate effective unit
    x_unit = unit/zoom;
    y_unit = (dtype)height/width*x_unit; // ensure equal aspect ratio
    
    // image boundaries
    x_min = center_x - x_unit;
    y_min = center_y - y_unit;
    
    // unit of one pixel
    x_step = 2*x_unit/(width-1);
    y_step = 2*y_unit/(height-1);
    
    // create and start calculation threads
    pthread_t* pthreads = new pthread_t[threads];
    thread_param* params = new thread_param[threads];
    for(size_t i = 0; i < threads; ++i) {
        params[i].mb = this;
        params[i].n = i;
        pthread_create(&pthreads[i], 0, calc_thread, &params[i]);
    }
    
    // wait for all threads to finish
    for(size_t i = 0; i < threads; ++i) {
        pthread_join(pthreads[i], 0);
    }
    delete[] pthreads;
    delete[] params;
    this->recalc = false;
    pthread_mutex_unlock(&this->mutex);
    return data;
}

/**
 * Thread callback which performs the actual mandelbrot set iteration
 */
void* MandelbrotSet::calc_thread(void* param)
{
    thread_param* tp = (thread_param*)param;
    MandelbrotSet* mb = tp->mb;
    
    // determine x range of current thread
    size_t ix_min = tp->n*mb->width/mb->threads;
    size_t ix_max = (tp->n+1)*mb->width/mb->threads;
    
    // loop through all pixels
    dtype c_re = mb->x_min + ix_min*mb->x_step;
    dtype c_im = mb->y_min;
    for(size_t ix = ix_min; ix < ix_max; ++ix) {
        c_re += mb->x_step;
        c_im = mb->y_min;
        for(size_t iy = 0; iy < mb->height; ++iy) {
            c_im += mb->y_step;
            
            // iterate for current pixel
            size_t value = mb->iter_max;
            dtype z_re = 0;
            dtype z_im = 0;
            dtype buf;
            for(size_t i = 0; i < mb->iter_max; ++i) {
                buf = z_re;
                z_re = z_re*z_re - z_im*z_im + c_re;
                z_im = 2*buf*z_im + c_im;
                if(z_re*z_re + z_im*z_im > 4) {
                    value = i;
                    break;
                }
            }
            
            // convert value to grey scale black/white
            if(mb->color_mode == 0) {
                int color = 255-(int)((long double)(value)/mb->iter_max*255);
                mb->data[ix*3 + iy*3*mb->width] = color;
                mb->data[ix*3 + iy*3*mb->width + 1] = color;
                mb->data[ix*3 + iy*3*mb->width + 2] = color;
            }
            
            // convert value to grey scale white/black
            else if(mb->color_mode == 1) {
                int color = (int)((long double)(value)/mb->iter_max*255);
                mb->data[ix*3 + iy*3*mb->width] = color;
                mb->data[ix*3 + iy*3*mb->width + 1] = color;
                mb->data[ix*3 + iy*3*mb->width + 2] = color;
            }
            
            // convert value to rgb color
            else if(mb->color_mode == 2) {
                int color = (int)((long double)(value)/mb->iter_max*16777216);
                mb->data[ix*3 + iy*3*mb->width] = color;
                mb->data[ix*3 + iy*3*mb->width + 1] = color >> 8;
                mb->data[ix*3 + iy*3*mb->width + 2] = color >> 16;
            }
        }
    }
    
    return 0;
}

