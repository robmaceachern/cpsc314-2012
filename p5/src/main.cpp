#if defined(__APPLE_CC__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif defined(WIN32)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdint.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define width 250
#define height 250

void idle(int value);

void init(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, width, height);
 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.0001, 1000.0);
 
    glMatrixMode(GL_MODELVIEW);
}

void setOrthographicProjection() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
       gluOrtho2D(0, width, 0, height);
    glScalef(1, -1, 1);
    glTranslatef(0, -height, 0);
    glMatrixMode(GL_MODELVIEW);
} 

void resetPerspectiveProjection() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
} 

void writeText(int x, int y, char* text)
{

    setOrthographicProjection();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.5, 0.4, 1);

    glRasterPos2i(100,100);
    GLfloat rp[4];

    GLint valid;
    glGetIntegerv(GL_CURRENT_RASTER_POSITION_VALID, &valid);
    printf("Current raster position valid, %d\n", valid);

    glGetFloatv(GL_CURRENT_RASTER_POSITION, rp);
    printf("current rasterPos (%f, %f, %f, %f)\n", rp[0], rp[1], rp[2], rp[3]);
    // GL_CURRENT_RASTER_POSITION
    
    for( int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);  
    }

    resetPerspectiveProjection();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// not exactly a callback, but sets a timer to call itself
// in an endless loop to update the program
void idle( int value ){    
    // set a timer to call this function again after the
    // required number of milliseconds
    float dt = 1000.0f*1.0f/30.0f;
    glutPostRedisplay();
    glutTimerFunc( dt, idle, 0 );
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    writeText(0,0, "Hello");

    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    // create window and rendering context
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize( width, height );
    glutCreateWindow( "Wall Buster" );
    
    glutDisplayFunc( display );

    idle( 0 );

    glutMainLoop();
    
    return 0;       // never reached
}

