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
#include <iostream>
#include <iomanip>
#include <math.h>
#include <GL/freeglut.h>
#include "MandelbrotSet.hh"

/**
 * see
 * http://www-h.eng.cam.ac.uk/help/tpl/graphics/using_glut.html
 * for info about opengl
 */

MandelbrotSet* mb = 0;

unsigned int width = 0;
unsigned int height = 0;

int move_start_x = -1;
int move_start_y = -1;
long double move_start_posx = -1;
long double move_start_posy = -1;

GLuint texture;
bool redraw = true;

bool autozoom = false;
double autozoomfac = 1.05;
int autozoomtime = 0;
double autozoomstart = 1;


/**
 * Called if window gets resized
 */
void resize(int w, int h)
{
    mb->set_size(w,h);
    width = w;
    height = h;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glScalef(1, -1, 1);
    glTranslatef(0, -h, 0);
    redraw = true;
}


/**
 * Render image
 */
static void render()
{
    double atime = glutGet(GLUT_ELAPSED_TIME)-autozoomtime;
    if(autozoom) {
        mb->set_view(
            mb->get_center_x(), mb->get_center_y(),
            autozoomstart * pow(autozoomfac, atime/1000.)
        );
        redraw = true;
    }
    
    if(redraw) {
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, mb->get_width(), mb->get_height(), 0,
            GL_RGB, GL_UNSIGNED_BYTE, mb->calc()
        );
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex3f(0,0,0);
            
            glTexCoord2f(1,0);
            glVertex3f(width,0,0);
            
            glTexCoord2f(1,1);
            glVertex3f(width,height,0);
            
            glTexCoord2f(0,1);
            glVertex3f(0,height,0);
        glEnd();
        
        glutSwapBuffers();
        
        redraw = false;
        
        std::cout << "---\n";
        std::cout << "Iterations: " << mb->get_iter_max() << "\n";
        std::cout << "Pos center: " << mb->get_center_x();
        std::cout <<  ", " << mb->get_center_y() << "\n";
        std::cout << "Width: " << (mb->get_coord_fac()*width) << "\n";
        std::cout << "Height: " << (mb->get_coord_fac()*height) << "\n";
        std::cout << "Zoom: " << mb->get_zoom() << "\n";
        std::cout << "Color: " << mb->get_color_mode() << "\n";
        std::cout << "Auto zoom: " << autozoom << "\n";
        std::cout << "Auto zoom fac: " << autozoomfac << "\n";
        std::cout << "Threads: " << mb->get_threads() << "\n";
    }
}

/**
 * Called if mouse has moved
 */
void mousemove(int x, int y)
{
    if(move_start_x != -1) {
        long double fac = mb->get_coord_fac();
        long double move_x = (move_start_x-x)*fac;
        long double move_y = (move_start_y-y)*fac;
        mb->set_view(
            move_start_posx + move_x, move_start_posy + move_y, mb->get_zoom()
        );
        redraw = true;
    }
}

/**
 * Called if mouse button has pressed
 */
void mousebutton(int button, int state, int x, int y)
{
    // move
    if(button == 0 && state == 1) {
        move_start_x = -1;
        move_start_y = -1;
    }
    else if(button == 0 && state == 0) {
        move_start_x = x;
        move_start_y = y;
        move_start_posx = mb->get_center_x();
        move_start_posy = mb->get_center_y();
    }
    
    // zoom in
    if(button == 3 && state == 1) {
        if(autozoom) {
            autozoomfac += 0.1;
            autozoomtime = glutGet(GLUT_ELAPSED_TIME);
            autozoomstart = mb->get_zoom();
        } else {
            mb->set_view(
                mb->get_center_x(), mb->get_center_y(), mb->get_zoom()*1.1
            );
        }
        redraw = true;
    }
    
    // zoom out
    else if(button == 4 && state == 1) {
        if(autozoom) {
            if(autozoomfac > 0.1) {
                autozoomfac -= 0.1;
                autozoomtime = glutGet(GLUT_ELAPSED_TIME);
                autozoomstart = mb->get_zoom();
            }
        } else {
            mb->set_view(
                mb->get_center_x(), mb->get_center_y(), mb->get_zoom()*0.9
            );
        }
        redraw = true;
    }
}

/**
 * Called if key has pressed
 */
void keypress(unsigned char key, int, int)
{
    switch(key) {
        // increase iterations
        case '+':
            mb->set_iter_max(mb->get_iter_max()+50);
            redraw = true;
            break;
        
        // decrease iterations
        case '-':
            if(mb->get_iter_max() > 50) {
                mb->set_iter_max(mb->get_iter_max()-50);
                redraw = true;
            }
            break;
        
        // cycle through color modes
        case 'c':
            if(mb->get_color_mode() == 2) {
                mb->set_color_mode(0);
            } else {
                mb->set_color_mode(mb->get_color_mode()+1);
            }
            redraw = true;
            break;
        
        // reset settings
        case 'r':
            mb->set_view(-0.75, 0,1);
            mb->set_iter_max(250);
            autozoom = false;
            autozoomfac = 1.05;
            autozoomstart = 1;
            redraw = true;
            break;
        
        // toggle autozoom
        case 'z':
            autozoom = !autozoom;
            autozoomtime = glutGet(GLUT_ELAPSED_TIME);
            autozoomstart = mb->get_zoom();
            redraw = true;
            break;
    }
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Mandelbrot set viewer");
    
    mb = new MandelbrotSet(250, 800, 600);
    
    int threads = 4;
    if(argc == 2) {
        threads = std::stoi(argv[1]);
    }
    mb->set_threads(threads);
    
    glEnable(GL_TEXTURE_2D);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
    
    glutDisplayFunc(render);
    glutReshapeFunc(resize);
    glutMouseFunc(mousebutton);
    glutMotionFunc(mousemove);
    glutKeyboardFunc(keypress);
    glutIdleFunc(render);
    
    glutMainLoop();
    
    delete mb;
    return 0;
}
