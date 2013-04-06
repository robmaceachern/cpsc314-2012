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
#include<stdint.h>
#include"CImg.h"

#include <Primitives.h>
#include <FileParser.h>
#include <sys/time.h>

using namespace cimg_library;
using namespace std;


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

// display width and height
int dispWidth=1440, dispHeight=1024;

int cameraMode = 1; // 1 is overhead, 2 is paddleview

void drawAxis();
void drawFloor();
void positionCamera();
void gl_select(int x, int y);
void gl_draw();
void draw_block(float x, float y, float z);

vector<Block> blockVector;

Ball ball = Ball(0.2, Point2D(1,1));

Block* paddle;

int currentScore = 0;
int livesRemaining = 5;
int currentLevel = 0;
int totalPaddleHits = 0;

// 0 - ball on paddle
// 1 - in game
// 2 - level beat
// 3 - game over
int gameState = 0;

GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
GLfloat mat_diffuse[] = { 0.1, 0.5, 0.8, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat no_shininess[] = { 0.0 };
GLfloat low_shininess[] = { 5.0 };
GLfloat high_shininess[] = { 100.0 };
GLfloat mat_emission[] = {0.3, 0.2, 0.2, 0.0};

GLfloat ambient[] = {0.19225, 0.19225, 0.19225};
GLfloat diffuse[] = {0.50754, 0.50754, 0.50754};
GLfloat specular[] = {0.508273, 0.508273, 0.508273};
GLfloat shininess[] = {0.4 * 128};

/*  Create checkerboard texture  */
#define checkImageWidth 64
#define checkImageHeight 64
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];
static GLuint texName;
uint8_t *data;

time_t startOfSample = time(NULL);
timeval start;
int numFrames = 1;

void makeCheckImage(void)
{
    // XCode users, uncomment the following line and replace *path_to_convert* with 
    // the output of the command "which convert" run from the command line.
    cimg::imagemagick_path( "/usr/local/bin/convert" );

    CImg<uint8_t> tex("metal-tex2.bmp");
    //tex.resize( 1024, 1024 );

    // resize to power-of-two if image is not already correctly sized
    // tex.resize( 1024, 1024 );

    // EDIT: added uint8_t* to declaration of data variable to fix compile error
    data = new uint8_t[ tex.width()*tex.height()*tex.spectrum() ];
    int pos = 0;
    for( int i=0; i<tex.height(); i++ ){
        for( int j=0; j<tex.width(); j++ ){
            for( int k=0; k<tex.spectrum(); k++ ){
                data[pos++] = tex(j,i,0,k);
            }
        }
    }
}



// AUX_RGBImageRec* LoadBMP(char* Filename)            // Loads A Bitmap Image
// {
//     //FILE *File=NULL;                    // File Handle
 
//     if (!Filename)                      // Make Sure A Filename Was Given
//     {
//         return NULL;                    // If Not Return NULL
//     }
 
//     FILE File=fopen(Filename,"r");               // Check To See If The File Exists
 
//     if (File)                       // Does The File Exist?
//     {
//         fclose(File);                   // Close The Handle
//         return auxDIBImageLoad(Filename);       // Load The Bitmap And Return A Pointer
//     }
//     return NULL;                        // If Load Failed Return NULL
// }

void drawCube() {
    glutSolidCube(1);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Initialization/Setup and Teardown ////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void init(){
    printf("what's up init?\n");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, dispWidth, dispHeight);
 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //positionCamera();
    gluPerspective(70.0, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.0001, 1000.0);
 
    glMatrixMode(GL_MODELVIEW);
}

// free any allocated objects and return
void cleanup(){

}


void doUpArrowPushed()
{
    if (gameState == 0) {
        // put the ball in motion
        ball.deltaX = 0.09;
        ball.deltaY = 0.16;
        gameState = 1;
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Callback Stubs ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// window resize callback
void resize_callback( int width, int height ){    

}

void writeText(Point2D point, char* text)
{

    glPushMatrix();
    //glLoadIdentity();    
    glColor3f(0.5, 0.4, 1);


    //glRasterPos2i(0, 1);
    glDisable(GL_TEXTURE_2D);
    for( int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);  
    }
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

void printSegments(LineSegment* segments)
{
    printf("Segment[0] = (%f, %f) (%f, %f)\n", segments[0].a.x, segments[0].a.y, segments[0].b.x, segments[0].b.y);
    printf("Segment[1] = (%f, %f) (%f, %f)\n", segments[1].a.x, segments[1].a.y, segments[1].b.x, segments[1].b.y);
    printf("Segment[2] = (%f, %f) (%f, %f)\n", segments[2].a.x, segments[2].a.y, segments[2].b.x, segments[2].b.y);
    printf("Segment[3] = (%f, %f) (%f, %f)\n", segments[3].a.x, segments[3].a.y, segments[3].b.x, segments[3].b.y);
}

void drawBlock(Block b)
{
    if (!b.isActive) return;

    float blockHeight = 0.4;
    glPushMatrix();

    Point2D pos = b.position;
    RectSize size = b.size;

    glTranslatef(pos.x + (size.w / 2.0), 0.2, -(pos.y + size.h/2.0));
    glScalef(size.w, blockHeight, size.h);

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glBindTexture(GL_TEXTURE_2D, texName);
    drawCube();
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void drawBall(Ball b)
{
    glPushMatrix();

    glColor3f(0.5, 0.4, 1);
    //glColor3f(1, 1, 1);


    Point2D pos = b.center;
    float radius = b.radius;

    glTranslatef(pos.x, 0.35, -(pos.y));

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
    glutSolidSphere(radius, 35, 35);

    glPopMatrix();
}

void drawPaddle()
{
    glPushMatrix();

    glTranslatef(paddle->position.x, paddle->position.y, -0.2);
    glScalef(paddle->size.w, 0.5, paddle->size.h);
    drawCube();

    glPopMatrix();
}

void positionCamera()
{
    gluPerspective(70.0, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.0001, 1000.0);
    //gluPerspective( 70.0f, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.1f, 2000.0f );

    // if (cameraMode == 1)
    // {
    //     glRotatef(60, 1, 0, 0);
    //     glTranslatef( -5.0, -5.0, 0 );
    //     drawAxis();
    // } else if (cameraMode == 2) {
    //     glTranslatef(-(paddle->position.x + (paddle->size.w/2.0)), -1.5, -1.5);
    //     drawAxis();
    // }
}

bool checkAndHandleIntersection(Ball* ball, Block* b)
{
    if (!b->isActive) return false;

    bool intersectionOccured = false;

    LineSegment segments[4] = b->segments;
    
    int j;
    for (j = 0; j < 4; j++) {
        Vec3 bounceVec;
        bool intersect = ball->intersectsWith(segments[j], &bounceVec);
        if (intersect) {

            intersectionOccured = true;
            LineSegment curr = segments[j];
            if (curr.a.x == curr.b.x) {
                // vertical segment
                ball->deltaX = -ball->deltaX;
            } else if (curr.a.y == curr.b.y) {
                // horizontal segment
                ball->deltaY = -ball->deltaY;
            }

            ball->center.x += bounceVec[0];
            ball->center.y += bounceVec[1];
        }
    }

    if (intersectionOccured) {
        b->hpRemaining = b->hpRemaining - 1;
    }

    if (b->hpRemaining <= 0) {
        b->isActive = false;
        currentScore += 100;
    }

    return intersectionOccured;
}

void updateBallPositionAndVelocity()
{
    // ball will be drawn immediately following this function


    // update position
    ball.center.x += ball.deltaX;
    ball.center.y += ball.deltaY;

    // check for collisions with blocks
    int i;
    for (i = 0; i < blockVector.size(); i++) {
        Block* b = &blockVector[i];
        bool blockHit = checkAndHandleIntersection(&ball, b);
    }

    // check for collision with paddle
    bool paddleHit = checkAndHandleIntersection(&ball, paddle);

    if (paddleHit) {
        totalPaddleHits++;
        if (totalPaddleHits > 15) {
            ball.deltaY *= 1.2;
            ball.deltaX *= 1.2;
            totalPaddleHits = 0;
            printf("ball speed increased! (%f, %f)\n", ball.deltaX, ball.deltaY);
        }
    }
    // TODO add some paddle spin?

    float xRightWall = 10;
    float xLeftWall = 0;
    float zTopWall = 10;
    float zBottomWall = 0;

    if (ball.center.x + ball.radius > xRightWall || ball.center.x - ball.radius < xLeftWall) {
        ball.deltaX = -ball.deltaX;
    }

    if (ball.center.y + ball.radius > zTopWall) {
        ball.deltaY = -ball.deltaY;
    }

    if (ball.center.y - ball.radius < zBottomWall) {
        printf("You died!\n");
        if (livesRemaining > 0) {
            gameState = 0;
        } else {
            gameState = 3;
        }
    }
}

// keyboard callback
void keyboardCallback( unsigned char key, int x, int y ){
    switch( key ){
        case 27:
            quit = true;
            break;
        case '1':
            cameraMode = 1;
            break;
        case '2':
            cameraMode = 2;
            break;
        case ' ': //spacebar
            doUpArrowPushed();
        default:
            break;
    }
}


// display callback
void displayCallback( void ){
    //int current_window;
    
    // retrieve the currently active window
    //current_window = glutGetWindow();
    
    // clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    /////////////////////////////////////////////////////////////
    /// TODO: Put your rendering code here! /////////////////////
    /////////////////////////////////////////////////////////////
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // gluPerspective( 70.0f, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.1f, 2000.0f );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    positionCamera();    

    //writeText(Point2D(1,1), "Hello!");

    if (gameState == 1) {
        updateBallPositionAndVelocity();    
    } else if (gameState == 0) {
        // draw ball in middle of paddle
        ball.center.x = paddle->position.x + (paddle->size.w)/2.0;
        ball.center.y = paddle->position.y + paddle->size.h + ball.radius;
    }
    

    drawBall(ball);

    //drawPaddle();
    drawBlock(*paddle);

    int i;
    for (i = 0; i < blockVector.size(); i++)
    {
        drawBlock(blockVector[i]);   
    }

    drawFloor();
    // swap the front and back buffers to display the scene
    //glutSetWindow( current_window );
    glutSwapBuffers();
    if (numFrames >= 15) {

        //http://stackoverflow.com/questions/2150291/how-do-i-measure-a-time-interval-in-c
        timeval now;
        gettimeofday(&now, NULL);
        double elapsedTime = elapsedTime = (now.tv_sec - start.tv_sec) * 1000.0;
        elapsedTime += (now.tv_usec - start.tv_usec) / 1000.0;
        //printf ("FPS: %f.\n", numFrames/ (elapsedTime/1000));
        start = now;
        numFrames = 0;
    }
    numFrames++;

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
    
    // set a timer to call this function again after the
    // required number of milliseconds
    glutPostRedisplay();
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

void drawFloor() {
    // Draw a floor. Since it is transparent, we need to do it AFTER all of
    // the opaque objects.
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            glColor4f(1, 1, 1, (x + y) % 2 ? 0.75 : 0.5);
            glNormal3f(0, 1, 0);
            glBegin(GL_POLYGON);
            glVertex3f(x    , 0, (-y)    );
            glVertex3f(x + 1, 0, (-y)    );
            glVertex3f(x + 1, 0, -(y + 1));
            glVertex3f(x    , 0, -(y + 1));
            glEnd();
        }
    }
    // visible from initial side angle
    glBegin(GL_POLYGON);
    glVertex3f(0,    0, 0);
    glVertex3f(0, -.05, 0);
    glVertex3f( 10, -.05, 0);
    glVertex3f( 10,    0, 0);
    glEnd();
    
    // visible from front angle
    glBegin(GL_POLYGON);
    glVertex3f(0,    0, 0);
    glVertex3f(0, -.05, 0);
    glVertex3f(0, -.05, 10);
    glVertex3f(0,    0, 10);
    glEnd();
}

void dumpMatrix(float* m) {

    for (int i = 0; i < 4; i++) {
        int base = i;
        printf("%f \t %f \t %f \t %f \n", m[base], m[base + 4], m[base + 8], m[base + 12]);
    }
    printf("\n");
}



// http://www.opengl.org/discussion_boards/showthread.php/136442-what-are-the-codes-for-arrow-keys-to-use-in-glut-keyboard-callback-function
void SpecialInput(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
        doUpArrowPushed();
        break;  
        case GLUT_KEY_DOWN:
        //do something here
        break;
        case GLUT_KEY_LEFT:
        paddle->position.x = paddle->position.x - 0.2;
        paddle->updateSegments();
        break;
        case GLUT_KEY_RIGHT:
        paddle->position.x = paddle->position.x + 0.2;
        paddle->updateSegments();
        break;
    }
}
// source: http://www.lighthouse3d.com/opengl/picking/index.php?openglway3
void processHits2 (GLint hits, GLuint buffer[])
{
   unsigned int i, j;
   GLuint names, *ptr, minZ,*ptrNames, numberOfNames;

   printf ("hits = %d\n", hits);
   ptr = (GLuint *) buffer;
   minZ = 0xffffffff;
   for (i = 0; i < hits; i++) { 
      names = *ptr;
      ptr++;
      if (*ptr < minZ) {
          numberOfNames = names;
          minZ = *ptr;
          ptrNames = ptr+2;
      }
      
      ptr += names+2;
    }
  printf ("The closest hit names are ");
  ptr = ptrNames;
  for (j = 0; j < numberOfNames; j++,ptr++) {
     printf ("%d ", *ptr);
  }
  printf ("\n");
   
}

void display2(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    /////////////////////////////////////////////////////////////
    /// TODO: Put your rendering code here! /////////////////////
    /////////////////////////////////////////////////////////////
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // gluPerspective( 70.0f, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.1f, 2000.0f );
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //positionCamera();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    //glTranslatef(2.0, -2.0, -5.0);
    if (cameraMode == 1)
    {
        glRotatef(60, 1, 0, 0);
        glTranslatef( -5.0, -5.0, 0.0 );
        drawAxis();
    } else if (cameraMode == 2) {
        glTranslatef(-(paddle->position.x + (paddle->size.w/2.0)), -1.5, -1.5);
        drawAxis();
    }

    //writeText(Point2D(1,1), "Hello!");

    if (gameState == 1) {
        updateBallPositionAndVelocity();    
    } else if (gameState == 0) {
        // draw ball in middle of paddle
        ball.center.x = paddle->position.x + (paddle->size.w)/2.0;
        ball.center.y = paddle->position.y + paddle->size.h + ball.radius;
    }
    

    drawBall(ball);

    //drawPaddle();
    drawBlock(*paddle);

    int i;
    for (i = 0; i < blockVector.size(); i++)
    {
        glPushName(i);
        drawBlock(blockVector[i]);
        glPopName();
    }

    drawFloor();
    // swap the front and back buffers to display the scene
    //glutSetWindow( current_window );
    glutSwapBuffers();
    if (numFrames >= 15) {

        //http://stackoverflow.com/questions/2150291/how-do-i-measure-a-time-interval-in-c
        timeval now;
        gettimeofday(&now, NULL);
        double elapsedTime = elapsedTime = (now.tv_sec - start.tv_sec) * 1000.0;
        elapsedTime += (now.tv_usec - start.tv_usec) / 1000.0;
        //printf ("FPS: %f.\n", numFrames/ (elapsedTime/1000));
        start = now;
        numFrames = 0;
    }
    numFrames++;
}

// source: http://content.gpwiki.org/index.php/OpenGL:Tutorials:Picking
// void doMouseClick(int button, int state, int cursorX, int cursorY) {

//     if (state == GLUT_UP) return;
//     printf("doMouseClick x: %d, y: %d\n", cursorX, cursorY);

//     int BUFSIZE = 512;
//     int hits = 0;

//     GLint viewport[4];
//     GLuint selectBuf[BUFSIZE];

//     glSelectBuffer(BUFSIZE,selectBuf);
//     glRenderMode(GL_SELECT);
//     glInitNames();
//     gluPickMatrix(cursorX,viewport[3]-cursorY,
//             5,5,viewport);

//     display2();

//     // glMatrixMode(GL_PROJECTION);
//     // glPushMatrix();
//     // glLoadIdentity();
//     // glGetIntegerv(GL_VIEWPORT,viewport);
    
//     // positionCamera();
//     // glMatrixMode(GL_MODELVIEW);
//     // glLoadIdentity();
//     // glInitNames();

//     // int i;
//     // for (i = 0; i < blockVector.size(); i++)
//     // {
//     //     glPushName(i);
//     //     drawBlock(blockVector[i]);   
//     //     glPopName();
//     // }

//     // returning to normal rendering mode
//     hits = glRenderMode(GL_RENDER);
//     if (hits != 0)
//         processHits2(hits,selectBuf);
// }
void gl_draw()
 {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    glTranslatef(0.0, 0.0, -5.0);
 
    glColor3f(1.0, 0.0, 0.0);
    glLoadName(7); /* Overwrite the first name in the buffer */
    draw_block(-0.3, 0, -2);
 
    glColor3f(0.0, 1.0, 0.0);
    glLoadName(14); /* Overwrite the first name in the buffer */
    draw_block(0, 0, -4);
 
    glColor3f(0.0, 0.0, 1.0);
    glLoadName(21); /* Overwrite the first name in the buffer */
    draw_block(0.3, 0, -6);
 
    glutSwapBuffers();
 }

void list_hits(GLint hits, GLuint *names)
 {
    int i;
 
    /*
        For each hit in the buffer are allocated 4 bytes:
        1. Number of hits selected (always one,
                                    beacuse when we draw each object
                                    we use glLoadName, so we replace the
                                    prevous name in the stack)
        2. Min Z
        3. Max Z
        4. Name of the hit (glLoadName)
    */
 
    printf("%d hits:\n", hits);
 
    for (i = 0; i < hits; i++)
        printf( "Number: %d\n"
                "Min Z: %d\n"
                "Max Z: %d\n"
                "Name on stack: %d\n",
                (GLubyte)names[i * 4],
                (GLubyte)names[i * 4 + 1],
                (GLubyte)names[i * 4 + 2],
                (GLubyte)names[i * 4 + 3]
                );
 
    printf("\n");
}

 void mousedw(int x, int y, int but)
 {
    printf("Mouse button %d pressed at %d %d\n", but, x, y);
    gl_select(x,dispHeight-y); //Important: gl (0,0) ist bottom left but window coords (0,0) are top left so we have to change this!
 }

void mouseClick(int button, int state, int x, int y)
 {
    if  ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
    {
        mousedw(x, y, button);
    }
 }

void draw_block(float x, float y, float z)
 {
    glPushMatrix();
        glTranslatef(x, y, z);
        glutSolidCube(1.0);
    glPopMatrix();
 }

void gl_selall(GLint hits, GLuint *buff)
 {
    GLuint *p;
    int i;
 
    display2();
 
    p = buff;
    for (i = 0; i < 6 * 4; i++)
    {
        printf("Slot %d: - Value: %d\n", i, p[i]);
    }
 
    printf("Buff size: %x\n", (GLbyte)buff[0]);
 }

void gl_select(int x, int y)
{
    GLuint buff[128] = {0};
    GLint hits, view[4];
    int id;
 
    /*
        This choose the buffer where store the values for the selection data
    */
    glSelectBuffer(128, buff);
 
    /*
        This retrieve info about the viewport
    */
    glGetIntegerv(GL_VIEWPORT, view);
 
    /*
        Switching in selecton mode
    */
    glRenderMode(GL_SELECT);
 
    /*
        Clearing the name's stack
        This stack contains all the info about the objects
    */
    glInitNames();
 
    /*
        Now fill the stack with one element (or glLoadName will generate an error)
    */
    glPushName(0);
 
    /*
        Now modify the vieving volume, restricting selection area around the cursor
    */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
 
        /*
            restrict the draw to an area around the cursor
        */
        gluPickMatrix(x, y, 5.0, 5.0, view);
        //gluPerspective(60, 1.0, 0.0001, 1000.0);
        gluPerspective(70.0, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.0001, 1000.0);

        /*
            Draw the objects onto the screen
        */
        glMatrixMode(GL_MODELVIEW);
 
        /*
            draw only the names in the stack, and fill the array
        */
        glutSwapBuffers();
        display2();
 
        /*
            Do you remeber? We do pushMatrix in PROJECTION mode
        */
        glMatrixMode(GL_PROJECTION);
    glPopMatrix();
 
    /*
        get number of objects drawed in that area
        and return to render mode
    */
    hits = glRenderMode(GL_RENDER);
 
    /*
        Print a list of the objects
    */
    list_hits(hits, buff);
 
    /*
        uncomment this to show the whole buffer
    */
    
    gl_selall(hits, buff);
    
 
    glMatrixMode(GL_MODELVIEW);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Program Entry Point //////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    // create window and rendering context
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize( dispWidth, dispHeight );
    glutCreateWindow( "Wall Buster" );
    
    // register display callback
    glutDisplayFunc( display2 );
    glutKeyboardFunc( keyboardCallback );
    glutSpecialFunc(SpecialInput);
    glutMouseFunc(mouseClick);
    
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_NORMALIZE );

    // texture
    makeCheckImage();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                   GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                   GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth,
                checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                data);

    // Turn on blending (for floor).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // lighting stuff
    GLfloat ambient[] = {0.3, 0.3, 0.3, 1.0};
    GLfloat diffuse[] = {0.9, 0.9, 0.9, 1.0};
    GLfloat specular[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat position0[] = {5.0, 1.0, 1.0, 0.0};
    glLightfv( GL_LIGHT0, GL_POSITION, position0 );
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    
    GLfloat position1[] = {2.0, 5.0, 0, 0.0};
    glLightfv( GL_LIGHT1, GL_POSITION, position1 );
    glLightfv( GL_LIGHT1, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT1, GL_SPECULAR, specular );
    
    glEnable( GL_LIGHTING );
    //glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );
    glEnable( GL_COLOR_MATERIAL );
    
    // (y + h) <= 10

    blockVector.push_back(Block(Point2D(0.5, 9.0), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 8.3), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 7.6), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 6.9), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 6.2), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 5.5), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 4.8), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 4.1), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(0.5, 3.4), RectSize(0.45, 0.45)));

    blockVector.push_back(Block(Point2D(1.0, 9.0), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 8.3), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 7.6), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 6.9), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 6.2), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 5.5), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 4.8), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 4.1), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(1.0, 3.4), RectSize(0.45, 0.45)));

    blockVector.push_back(Block(Point2D(1.95, 8.0), RectSize(0.9, 0.9)));
    blockVector.push_back(Block(Point2D(1.95, 6.6), RectSize(0.9, 0.9)));
    blockVector.push_back(Block(Point2D(1.95, 5.2), RectSize(0.9, 0.9)));

    blockVector.push_back(Block(Point2D(3.5, 9.0), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 8.3), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 7.6), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 6.9), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 6.2), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 5.5), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 4.8), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 4.1), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(3.5, 3.4), RectSize(0.45, 0.45)));

    blockVector.push_back(Block(Point2D(4.55, 8.0), RectSize(0.9, 0.9)));
    blockVector.push_back(Block(Point2D(4.55, 6.6), RectSize(0.9, 0.9)));
    blockVector.push_back(Block(Point2D(4.55, 5.2), RectSize(0.9, 0.9)));
    blockVector.push_back(Block(Point2D(4.55, 3.8), RectSize(0.9, 0.9)));

    blockVector.push_back(Block(Point2D(6.05, 9.0), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 8.3), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 7.6), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 6.9), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 6.2), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 5.5), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 4.8), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 4.1), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(6.05, 3.4), RectSize(0.45, 0.45)));

    blockVector.push_back(Block(Point2D(6.95, 8.0), RectSize(0.9, 0.9)));
    blockVector.push_back(Block(Point2D(6.95, 6.6), RectSize(0.9, 0.9)));
    blockVector.push_back(Block(Point2D(6.95, 5.2), RectSize(0.9, 0.9)));

    blockVector.push_back(Block(Point2D(8.5, 9.0), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 8.3), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 7.6), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 6.9), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 6.2), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 5.5), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 4.8), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 4.1), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(8.5, 3.4), RectSize(0.45, 0.45)));

    blockVector.push_back(Block(Point2D(9.0, 9.0), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 8.3), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 7.6), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 6.9), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 6.2), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 5.5), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 4.8), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 4.1), RectSize(0.45, 0.45)));
    blockVector.push_back(Block(Point2D(9.0, 3.4), RectSize(0.45, 0.45)));
    

    // blockVector.push_back(Block(Point2D(4, 8), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(6, 8), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(0, 2), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(2, 6), RectSize(1, 0.8)));
    // blockVector.push_back(Block(Point2D(8, 5), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(6, 7), RectSize(1, 0.8)));
    // blockVector.push_back(Block(Point2D(5, 6), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(3, 5), RectSize(1, 0.8)));
    // blockVector.push_back(Block(Point2D(2, 4), RectSize(4, 0.8)));
    // blockVector.push_back(Block(Point2D(4, 8), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(6, 8), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(0, 2), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(2, 6), RectSize(1, 0.8)));
    // blockVector.push_back(Block(Point2D(8, 5), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(6, 7), RectSize(1, 0.8)));
    // blockVector.push_back(Block(Point2D(5, 6), RectSize(2, 0.8)));
    // blockVector.push_back(Block(Point2D(3, 5), RectSize(1, 0.8)));

    // the paddle block
    Block paddleBlock = Block(Point2D(3, 0), RectSize(3, 0.4));
    paddleBlock.hpRemaining = std::numeric_limits<int>::max();
    paddle = &paddleBlock;

    idle( 0 );

    init();

    gettimeofday(&start, NULL);
    // pass control over to GLUT
    glutMainLoop();
    
    return 0;       // never reached
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

