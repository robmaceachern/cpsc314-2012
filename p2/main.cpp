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
#include<math.h>
#include<stdio.h>

enum SpaceShip { SCOUT, MOTHER };
enum NavigationMode {ABSOLUTE, RELATIVE, GEOSYNC};

void drawAxis();
void positionCamera(int cameraId);
void doAdjustEyePoint(float x, float y, float z);
void doAdjustLookatPoint(float x, float y, float z);
void doAdjustUpVector(float x, float y, float z);
void drawOrbit(float radius);
void drawPlanet(float* planet, float r, float g, float b);
void doReset();
void dumpMatrix(float* m);
bool invert_pose(float *m);

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

float scoutShip[9];
float motherShip[9];

float speed = 1;

bool isPaused = false;

// 9 planets
// radius, distanceFromSun, 
float planetInfo[10][2] = {
    {0.5, 0},             // sun
    {0.2, 0.8},           // mercury
    {0.3, 1.3},          // venus
    {0.35, 2.2},
    {0.3, 3.2},
    {0.45, 4.2},
    {0.64, 5.2},
    {0.54, 6.2},
    {0.34, 7.2},
    {0.1, 8.2}
};

SpaceShip currentShipControl = SCOUT;
NavigationMode currentNavigationMode = ABSOLUTE;

GLfloat motherShipViewMatrix[16];
GLfloat scoutShipViewMatrix[16];

float relativeMotion = 0;
float relativeYaw = 0;
float relativeRoll = 0;
float relativePitch = 0;

int geosyncPlanetSelection = 3;

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
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 1;
                }
            }
            break;
        case '2':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 2;
                }
            }
            break;
        case '3':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 3;
                }
            }
            break;
        case '4':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 4;
                }
            }
            break;
        case '5':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 5;
                }
            }
            break;
        case '6':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 6;
                }
            }
            break;
        case '7':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 7;
                }
            }
            break;
        case '8':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 8;
                }
            }
            break;
        case '9':
            {
                if (currentNavigationMode == GEOSYNC) {
                    geosyncPlanetSelection = 9;
                }
            }
            break;
        case 'q':
            {
                if (currentNavigationMode == RELATIVE) {
                    relativeYaw = relativeYaw - speed;
                } 
            }
            break;
        case 'r':
            currentNavigationMode = RELATIVE;
            printf("Navigation mode set to RELATIVE\n");
            break;
        case 'g':
            currentNavigationMode = GEOSYNC;
            printf("Navigation mode set to GEOSYNC\n");
            break;
        case 'l':
            currentNavigationMode = ABSOLUTE;
            printf("Navigation mode set to ABSOLUTE\n");
            break;
        case '<':
            currentShipControl = SCOUT;
            printf("Ship control set to SCOUT \n");
            break;
        case '>':
            currentShipControl = MOTHER;
            printf("Ship control set to MOTHER \n");
            break;
        case 'x':
            {
                if (currentNavigationMode == ABSOLUTE) {
                    doAdjustEyePoint(speed, 0, 0);
                } else if (currentNavigationMode == RELATIVE) {
                    relativePitch = relativePitch - speed;
                }
            }
            break;
        case 'X':
            {
                if (currentNavigationMode == ABSOLUTE) {
                    doAdjustEyePoint(-speed, 0, 0);
                } else {
                    printf("NOT IMPLEMENTED: x\n");
                }
            }
            break;
        case 'y':
            doAdjustEyePoint(0, speed, 0);
            break;
        case 'Y':
            doAdjustEyePoint(0, -speed, 0);
            break;
        case 'z':
            doAdjustEyePoint(0, 0, speed);
            break;
        case 'Z':
            doAdjustEyePoint(0, 0, -speed);
            break;
        case 'a':
            {
                if (currentNavigationMode == ABSOLUTE) {
                    doAdjustLookatPoint(speed, 0, 0);
                } else if (currentNavigationMode == RELATIVE) {
                    relativeRoll = relativeRoll - speed;
                }
            }
            break;
        case 'A':
            doAdjustLookatPoint(-speed, 0, 0);
            break;
        case 'b':
            doAdjustLookatPoint(0, speed, 0);
            break;
        case 'B':
            doAdjustLookatPoint(0, -speed, 0);
            break;
        case 'c':
            {
                if (currentNavigationMode == ABSOLUTE) {
                    doAdjustLookatPoint(0, 0, speed);
                } else if (currentNavigationMode == RELATIVE) {
                    relativePitch = relativePitch + speed;
                }
            }
            break;
        case 'C':
            doAdjustLookatPoint(0, 0, -speed);
            break;
        case 'd':
            {
                if (currentNavigationMode == ABSOLUTE) {
                    doAdjustUpVector(speed, 0, 0);
                } else if (currentNavigationMode == RELATIVE) {
                    relativeRoll = relativeRoll + speed;
                }
            }
            break;
        case 'D':
            doAdjustUpVector(-speed, 0, 0);
            break;
        case 'e':
            {
                if (currentNavigationMode == ABSOLUTE) {
                    doAdjustUpVector(0, speed, 0);
                } else if (currentNavigationMode == RELATIVE) {
                    relativeYaw = relativeYaw + speed;
                } else {
                    printf("NOT IMPLEMENTED\n");
                }
            }
            break;
        case 'E':
            doAdjustUpVector(0, -speed, 0);
            break;
        case 'f':
            doAdjustUpVector(0, 0, speed);
            break;
        case 'F':
            doAdjustUpVector(0, 0, -speed);
            break;
        case '-':
            {
                speed = speed - 0.1;
                if (speed < 0) {
                    speed = 0;
                }
            }
            break;
        case '=':
            {
                speed = speed + 0.1;
            }
            break;
        case 'm':
            doReset();
            break;
        case 'p':
            isPaused = !isPaused;
            break;
        case 'w':
            {
                if (currentNavigationMode == RELATIVE) {
                    relativeMotion = relativeMotion + speed;
                }
            }
            break;
        case 's':
            {
                if (currentNavigationMode == RELATIVE) {
                    relativeMotion = relativeMotion - speed;
                }
            }
            break;
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

    if (!isPaused) {
        hour = (hour + 1) % 240;
        day = (day + 1) % 365;
    }

    int hourRotation = hour/240.0 * 360;
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

    if (currentNavigationMode == ABSOLUTE) {
        positionCamera(current_window);
    } else if (currentNavigationMode == RELATIVE) {
        
        if (currentShipControl == MOTHER && current_window == 1) {
            glRotatef(relativeYaw, 0, 1, 0);
            glRotatef(relativePitch, 1, 0, 0);
            glRotatef(relativeRoll, 0, 0, 1);
            glTranslatef(0, 0, relativeMotion);
            relativeMotion = 0;
            relativeYaw = 0;
            relativePitch = 0;
            relativeRoll = 0;
            
        } else if (currentShipControl == SCOUT && current_window == 2) {
            glRotatef(relativeYaw, 0, 1, 0);
            glRotatef(relativePitch, 1, 0, 0);
            glRotatef(relativeRoll, 0, 0, 1);
            glTranslatef(0, 0, relativeMotion);
            relativeMotion = 0;
            relativeYaw = 0;
            relativePitch = 0;
            relativeRoll = 0;
        }

        if (current_window == 1) {
            glMultMatrixf(motherShipViewMatrix);
        } else if (current_window == 2) {
            glMultMatrixf(scoutShipViewMatrix);
        }
        

    } else if (currentNavigationMode == GEOSYNC) {

        int hourRotation = hour/240.0 * 360;
        int rotation = day/365.0 * 360;

        float *planet = planetInfo[geosyncPlanetSelection];
        glPushMatrix();
        glRotatef(rotation, 0, 1, 0);
        glTranslatef(planet[1], 0, 0);
        glRotatef(hourRotation, 0, 0.9, 0.3);
        float planetCoord[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, planetCoord);
        glPopMatrix();

        bool inverted = invert_pose(planetCoord);
        if (!inverted) {
            printf("ERROR INVERTING MATRIX!\n");
        }

        glTranslatef(0, 0, -2);
        glMultMatrixf(planetCoord);

    } else {
        printf("Unknown navigation mode\n");
    }

    if (current_window == 1) {
        // printf("Mothership window:\n");
        GLfloat m[16]; 
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        //dumpMatrix(m);
        for (int i = 0; i < 16; i++) {
            motherShipViewMatrix[i] = m[i];
        }
    } else {
        // printf("Scoutship window:\n");
        GLfloat m[16]; 
        glGetFloatv(GL_MODELVIEW_MATRIX, m);
        //dumpMatrix(m);
        for (int i = 0; i < 16; i++) {
            scoutShipViewMatrix[i] = m[i];
        }
    }

    if (current_window == 2) {
        // draw the mother
        glPushMatrix();
        glColor3f(0, 1, 1);
        glTranslatef(motherShip[0], motherShip[1], motherShip[2]);
        glutSolidCone(0.2, 0.4, 8, 2);
        glPopMatrix();
    } else if (current_window == 1) {
        // draw the scout
        glPushMatrix();
        glColor3f(0, 1, 0);
        glTranslatef(scoutShip[0], scoutShip[1], scoutShip[2]);
        glutSolidCone(0.3, 0.6, 8, 2);
        glPopMatrix();
    } else {
        printf("Unknown window\n");
    }

    glPushMatrix();                     // root of solar hierarchy

    glPushMatrix();                     // sun
    drawPlanet(planetInfo[0], 1, 1, 0);
    glPopMatrix();                      // end sun

    glPushMatrix();                     // mercury
    drawPlanet(planetInfo[1], 1, 0, 0);
    glPopMatrix();                      // end mercury

    glPushMatrix();                     // venus
    drawPlanet(planetInfo[2], 0.4, 0.3, 0.4);
    glPopMatrix();                      // end venus

    glPushMatrix();                     // earth
    drawPlanet(planetInfo[3], 0.9, 0.1, 0.9);
        glPushMatrix();                     // earth's moon
        drawOrbit(0.45);
        glTranslatef(0.45, 0, 0);
        glutSolidSphere(0.1, 10, 10);
        glPopMatrix();
    glPopMatrix();                      // end earth

    glPushMatrix();                     // mars
    drawPlanet(planetInfo[4], 0.3, 0.5, 0.9);
    glPopMatrix();                      // end mars

    glPushMatrix();                     // jupiter
    drawPlanet(planetInfo[5], 0.3, 0.9, 0.9);
    glPopMatrix();                      // end jupiter

    glPushMatrix();                     // saturn
    drawPlanet(planetInfo[6], 0.1, 0.1, 0.9);
    drawOrbit(0.75);
    drawOrbit(0.8);
    drawOrbit(0.85);
    drawOrbit(0.9);
    glPopMatrix();                      // end saturn

    glPushMatrix();                     // uranus
    drawPlanet(planetInfo[7], 0.6, 0.9, 0.2);
    glPopMatrix();                      // end uranus

    glPushMatrix();                     // neptune
    drawPlanet(planetInfo[8], 0.9, 0.3, 0.9);
    glPopMatrix();                      // end neptune

    glPushMatrix();                     // pluto
    drawPlanet(planetInfo[9], 0.9, 0.8, 0.5);
    glPopMatrix();                      // end pluto

    glPopMatrix();                      // end solar hierarchy

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

    float extra = 1; // TODO-rm

    if (cameraId == 1) {
        gluLookAt(  motherShip[0], motherShip[1], motherShip[2],
                    motherShip[3], motherShip[4], motherShip[5],
                    motherShip[6], motherShip[7], motherShip[8]);
    } else if (cameraId == 2) {
        gluLookAt(  scoutShip[0], scoutShip[1], scoutShip[2],
                    scoutShip[3], scoutShip[4], scoutShip[5],
                    scoutShip[6], scoutShip[7], scoutShip[8]);
    } else {
        printf("Invalid camera id: %d\n", cameraId);
    }
}

void doAdjustEyePoint(float x, float y, float z) {
    if (currentShipControl == SCOUT) {
        scoutShip[0] += x;
        scoutShip[1] += y;
        scoutShip[2] += z;
    } else if (currentShipControl == MOTHER) {
        motherShip[0] += x;
        motherShip[1] += y;
        motherShip[2] += z;
    } else {
        printf("Not implemented\n");
    }
}

void doAdjustLookatPoint(float x, float y, float z) {
    if (currentShipControl == SCOUT) {
        scoutShip[3] += x;
        scoutShip[4] += y;
        scoutShip[5] += z;
    } else if (currentShipControl == MOTHER) {
        motherShip[3] += x;
        motherShip[4] += y;
        motherShip[5] += z;
    } else {
        printf("Not implemented\n");
    }
}

void doAdjustUpVector(float x, float y, float z) {
    if (currentShipControl == SCOUT) {
        scoutShip[6] += x;
        scoutShip[7] += y;
        scoutShip[8] += z;
    } else if (currentShipControl == MOTHER) {
        motherShip[6] += x;
        motherShip[7] += y;
        motherShip[8] += z;
    } else {
        printf("Not implemented\n");
    }
}

void drawOrbit(float radius) {

    float pi = 3.14;
    float steps = 50;

    glLineWidth(2);
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < steps; i++) {
        float angle = (i/steps) * 2 * pi;
        float x = radius * sin(angle);
        float y = radius * cos(angle);
        glVertex3f(x, 0, y);
    }

    glEnd();
}

void drawPlanet(float* planet, float r, float g, float b) {

    int hourRotation = hour/240.0 * 360;
    int rotation = day/365.0 * 360;

    glColor3f(r, g, b);
    drawOrbit(planet[1]);
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(planet[1], 0, 0);
    glRotatef(hourRotation, 0, 0.9, 0.3);
    glutSolidSphere(planet[0], 6, 6);
}

void doReset() {
    if (currentNavigationMode == ABSOLUTE) {
        // Initial ship locations and viewing angles
        scoutShip[0] = -2;
        scoutShip[1] = 2;
        scoutShip[2] = -3;
        scoutShip[3] = 0;
        scoutShip[4] = 0;
        scoutShip[5] = 0;
        scoutShip[6] = 0;
        scoutShip[7] = 1;
        scoutShip[8] = 0;
        
        motherShip[0] = -4;
        motherShip[1] = 8;
        motherShip[2] = -6;
        motherShip[3] = 0;
        motherShip[4] = 0;
        motherShip[5] = 0;
        motherShip[6] = 0;
        motherShip[7] = 1;
        motherShip[8] = 0;
    } else if (currentNavigationMode == RELATIVE) {
        printf("NOT IMPLEMENTED\n");
    } else if (currentNavigationMode == GEOSYNC) {
        printf("NOT IMPLEMENTED\n");
    } else {
        printf("Unknown navigation mode\n");
    }
}

void dumpMatrix(float* m) {

    for (int i = 0; i < 4; i++) {
        int base = i;
        printf("%f \t %f \t %f \t %f \n", m[base], m[base + 4], m[base + 8], m[base + 12]);
    }
    printf("\n");
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
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    
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

    doReset();

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