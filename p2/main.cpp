#if defined(__APPLE_CC__)
#include<OpenGL/gl.h>
#include<OpenGL/glu.h>
#include<GLUT/glut.h>
#elif defined(WIN32)
#include<windows.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glut.h>
#else
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glut.h>
#include<stdint.h>
#endif

#include<iostream>
#include<stdlib.h>

void drawAxis();
void positionCamera(int cameraId);

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Global State Variables ///////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// time increment between calls to idle() in ms,
// currently set to 30 FPS
float dt = 1000.0f*1.0f/30.0f;

// flag to indicate that we should clean up and exit
bool quit = false;

// window handles for mother ship and scout ship
int mother_window, scout_window;

// display width and height
int disp_width=512, disp_height=512;

int day = 0;
int hour = 0;


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Initialization/Setup and Teardown ////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// set up opengl state, allocate objects, etc.  This gets called
// ONCE PER WINDOW, so don't allocate your objects twice!
void init(){
    /////////////////////////////////////////////////////////////
    /// TODO: Put your initialization code here! ////////////////
    /////////////////////////////////////////////////////////////
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    glViewport( 0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_NORMALIZE );
    
    // lighting stuff
    GLfloat ambient[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat diffuse[] = {0.9, 0.9, 0.9, 1.0};
    GLfloat specular[] = {0.4, 0.4, 0.4, 1.0};
    GLfloat position0[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv( GL_LIGHT0, GL_POSITION, position0 );
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    GLfloat position1[] = {-1.0, -1.0, -1.0, 0.0};
    glLightfv( GL_LIGHT1, GL_POSITION, position1 );
    glLightfv( GL_LIGHT1, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT1, GL_SPECULAR, specular );
    
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );
    glEnable( GL_COLOR_MATERIAL );
    
}

// free any allocated objects and return
void cleanup(){
    /////////////////////////////////////////////////////////////
    /// TODO: Put your teardown code here! //////////////////////
    /////////////////////////////////////////////////////////////

}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Callback Stubs ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// window resize callback
void resize_callback( int width, int height ){    
    /////////////////////////////////////////////////////////////
    /// TODO: Put your resize code here! ////////////////////////
    /////////////////////////////////////////////////////////////
}

// keyboard callback
void keyboard_callback( unsigned char key, int x, int y ){
    switch( key ){
        case 27:
            quit = true;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            printf("Keyboard press for geosync on planet: %c\n", key);
            break;
        case '-':
        case '=':
            printf("Keyboard press for speed change: %c\n", key);
            break;
        case 'q':
            printf("Keyboard press for negative yaw: %c\n", key);
            break;
        case 'r':
        case 'g':
        case 'l':
            printf("Key press for mode switch: %c\n", key);
            break;
        case '<':
        case '>':
            printf("Key press for ship control switch: %c\n", key);
            break;
        case 'w':
        case 'e':
        case 'y':
        case 'p':
        case 'a':
        case 's':
        case 'd':
        case 'f':
        case 'z':
        case 'x':
        case 'c':
        case 'b':
            printf("Key press not implemented: %c\n", key);
        default:
            break;
    }
    
    /////////////////////////////////////////////////////////////
    /// TODO: Put your keyboard code here! //////////////////////
    /////////////////////////////////////////////////////////////
    
}

// display callback
void display_callback( void ){
    int current_window;
    
    // retrieve the currently active window
    current_window = glutGetWindow();

    printf("Currently active window: %d \n", current_window);

    hour = (hour + 1) % 240;
    int hourRotation = hour/240.0 * 360;

    day = (day + 1) % 365;
    int rotation = day/365.0 * 360;
    
    // clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 

    /////////////////////////////////////////////////////////////
    /// TODO: Put your rendering code here! /////////////////////
    /////////////////////////////////////////////////////////////
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 70.0f, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.1f, 2000.0f );
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    positionCamera(current_window);

    glPushMatrix();                     // root of solar hierarchy

    glPushMatrix();                     // sun
    glColor3f(1, 1, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end sun

    glPushMatrix();                     // mercury
    glColor3f(1, 0, 0);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-1, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 6, 6);
    glPopMatrix();                      // end mercury

    glPushMatrix();                     // venus
    glColor3f(0.4, 0.3, 0.4);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-1.5, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end venus

    glPushMatrix();                     // earth
    glColor3f(0.9, 0.1, 0.9);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-2, 0, 0);

    glPushMatrix();                     // earth's moon
    glRotatef(rotation, 1, 0, 0);
    glTranslatef(-0.25, -0.25, 0);
    glutSolidSphere(0.1, 10, 10);
    glPopMatrix();

    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end earth

    glPushMatrix();                     // mars
    glColor3f(0.3, 0.5, 0.9);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-2.5, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end mars

    glPushMatrix();                     // jupiter
    glColor3f(0.3, 0.9, 0.9);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-3, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end jupiter

    glPushMatrix();                     // saturn
    glColor3f(0.1, 0.1, 0.9);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-3.5, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end saturn

    glPushMatrix();                     // uranus
    glColor3f(0.6, 0.9, 0.2);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-4, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end uranus

    glPushMatrix();                     // neptune
    glColor3f(0.9, 0.3, 0.9);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-4.5, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end neptune

    glPushMatrix();                     // pluto
    glColor3f(0.3, 0.1, 0.5);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(-5, 0, 0);
    glRotatef(hourRotation, 0, 0.5, 0.5);
    glutSolidSphere(0.3, 10, 10);
    glPopMatrix();                      // end pluto

    glPopMatrix();                      // end sun

    drawAxis();

    // swap the front and back buffers to display the scene
    glutSetWindow( current_window );
    glutSwapBuffers();
}

// not exactly a callback, but sets a timer to call itself
// in an endless loop to update the program
void idle( int value ){
    
    // if the user wants to quit the program, then exit the
    // function without resetting the timer or triggering
    // a display update
    if( quit ){
        // cleanup any allocated memory
        cleanup();
        
        // perform hard exit of the program, since glutMainLoop()
        // will never return
        exit(0);
    }
    
    /////////////////////////////////////////////////////////////
    /// TODO: Put your idle code here! //////////////////////////
    /////////////////////////////////////////////////////////////

    
    // set the currently active window to the mothership and
    // request a redisplay
    glutSetWindow( mother_window );
    glutPostRedisplay();
    
    // now set the currently active window to the scout ship
    // and redisplay it as well
    glutSetWindow( scout_window );
    glutPostRedisplay();
    
    // set a timer to call this function again after the
    // required number of milliseconds
    glutTimerFunc( dt, idle, 0 );
}

// Draw a set of coloured cones representing the current local coord system.
// X -> red, Y -> green, Z -> blue.
void drawAxis() {
    float axisHeight = 0.75; // TODO
    float axisBase = 0.075;
    glColor3f(0, 0, 1);
    glutSolidCone(axisBase, axisHeight, 8, 2);
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glColor3f(1, 0, 0);
    glutSolidCone(axisBase, axisHeight, 8, 2);
    glPopMatrix();
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glColor3f(0, 1, 0);
    glutSolidCone(axisBase, axisHeight, 8, 2);
    glPopMatrix();
}

void positionCamera(int cameraId) {

    if (cameraId == 1) {
        gluLookAt(0, 6, 6, 0, 0, 0, 0, 1, 0);
    } else if (cameraId == 2) {
        gluLookAt(0, 1, 1, 0, 0, 0, 0, 1, 0);
    } else {
        printf("Invalid camera id: %d\n", cameraId);
    }
}

// inversion routine originally from MESA
bool invert_pose( float *m ){
    float inv[16], det;
    int i;

    inv[0] = m[5] * m[10] * m[15] -
    m[5] * m[11] * m[14] -
    m[9] * m[6] * m[15] +
    m[9] * m[7] * m[14] +
    m[13] * m[6] * m[11] -
    m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
    m[4] * m[11] * m[14] +
    m[8] * m[6] * m[15] -
    m[8] * m[7] * m[14] -
    m[12] * m[6] * m[11] +
    m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
    m[4] * m[11] * m[13] -
    m[8] * m[5] * m[15] +
    m[8] * m[7] * m[13] +
    m[12] * m[5] * m[11] -
    m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
    m[4] * m[10] * m[13] +
    m[8] * m[5] * m[14] -
    m[8] * m[6] * m[13] -
    m[12] * m[5] * m[10] +
    m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
    m[1] * m[11] * m[14] +
    m[9] * m[2] * m[15] -
    m[9] * m[3] * m[14] -
    m[13] * m[2] * m[11] +
    m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
    m[0] * m[11] * m[14] -
    m[8] * m[2] * m[15] +
    m[8] * m[3] * m[14] +
    m[12] * m[2] * m[11] -
    m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
    m[0] * m[11] * m[13] +
    m[8] * m[1] * m[15] -
    m[8] * m[3] * m[13] -
    m[12] * m[1] * m[11] +
    m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
    m[0] * m[10] * m[13] -
    m[8] * m[1] * m[14] +
    m[8] * m[2] * m[13] +
    m[12] * m[1] * m[10] -
    m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
    m[1] * m[7] * m[14] -
    m[5] * m[2] * m[15] +
    m[5] * m[3] * m[14] +
    m[13] * m[2] * m[7] -
    m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
    m[0] * m[7] * m[14] +
    m[4] * m[2] * m[15] -
    m[4] * m[3] * m[14] -
    m[12] * m[2] * m[7] +
    m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
    m[0] * m[7] * m[13] -
    m[4] * m[1] * m[15] +
    m[4] * m[3] * m[13] +
    m[12] * m[1] * m[7] -
    m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
    m[0] * m[6] * m[13] +
    m[4] * m[1] * m[14] -
    m[4] * m[2] * m[13] -
    m[12] * m[1] * m[6] +
    m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
    m[1] * m[7] * m[10] +
    m[5] * m[2] * m[11] -
    m[5] * m[3] * m[10] -
    m[9] * m[2] * m[7] +
    m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
    m[0] * m[7] * m[10] -
    m[4] * m[2] * m[11] +
    m[4] * m[3] * m[10] +
    m[8] * m[2] * m[7] -
    m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
    m[0] * m[7] * m[9] +
    m[4] * m[1] * m[11] -
    m[4] * m[3] * m[9] -
    m[8] * m[1] * m[7] +
    m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
    m[0] * m[6] * m[9] -
    m[4] * m[1] * m[10] +
    m[4] * m[2] * m[9] +
    m[8] * m[1] * m[6] -
    m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        m[i] = inv[i] * det;

    return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Program Entry Point //////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
int main( int argc, char **argv ){
    // initialize glut
    glutInit( &argc, argv );
    
    // use double-buffered RGB+Alpha framebuffers with a depth buffer.
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
    
    // initialize the mothership window
    glutInitWindowSize( disp_width, disp_height );
    glutInitWindowPosition( 0, 100 );
    mother_window = glutCreateWindow( "Mother Ship" );
    glutKeyboardFunc( keyboard_callback );
    glutDisplayFunc( display_callback );
    glutReshapeFunc( resize_callback );
    
    // initialize the scout ship window
    glutInitWindowSize( disp_width, disp_height );
    glutInitWindowPosition( disp_width+50, 100 );
    scout_window = glutCreateWindow( "Scout Ship" );
    glutKeyboardFunc( keyboard_callback );
    glutDisplayFunc( display_callback );
    glutReshapeFunc( resize_callback );
    
    glutSetWindow( mother_window );
    init();
    glutSetWindow( scout_window );
    init();
    
    // start the idle on a fixed timer callback
    idle( 0 );
    
    // start the blug main loop
    glutMainLoop();
    
    return 0;
}