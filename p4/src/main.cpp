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
#include <sstream>

using namespace cimg_library;
using namespace std;


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Global State Variables ///////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// time increment between calls to idle() in ms,
// currently set to 50 FPS
float dt = 1000.0f*1.0f/70.0f;

// flag to indicate that we should clean up and exit
bool quit = false;

// display width and height
int dispWidth=1440, dispHeight=1024;

// 1 is overhead, 2 is paddleview
int cameraMode = 1;

void drawFloor();
void gl_select(int x, int y);

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
GLfloat ambient[] = {0.19225, 0.19225, 0.19225};
GLfloat diffuse[] = {0.50754, 0.50754, 0.50754};
GLfloat specular[] = {0.508273, 0.508273, 0.508273};
GLfloat shininess[] = {0.4 * 128};

static GLuint blockTex;
static GLuint floorTex;
static GLuint paddleTex;

time_t startOfSample = time(NULL);
timeval start;
int numFrames = 1;
float fps = 30.0;

float currViewpointRotation = 30.0;
float currViewpointTranslateX = -5;
float currViewpointTranslateY = -5;
float currViewpointTranslateZ = -5;

void loadTexture(string filePath, GLuint* texName)
{
    cimg::imagemagick_path( "/usr/local/bin/convert" );

    CImg<uint8_t> tex(filePath.c_str());
    //tex.resize( 1024, 1024 );

    uint8_t *data = new uint8_t[ tex.width()*tex.height()*tex.spectrum() ];
    int pos = 0;
    for( int i=0; i<tex.height(); i++ ){
        for( int j=0; j<tex.width(); j++ ){
            for( int k=0; k<tex.spectrum(); k++ ){
                data[pos++] = tex(j,i,0,k);
            }
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, texName);
    glBindTexture(GL_TEXTURE_2D, *texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width(), tex.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, data);

}

void drawCube() {
    glutSolidCube(1);
}

void setOrthographicProjection() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
       gluOrtho2D(0, dispWidth, 0, dispHeight);
    glScalef(1, -1, 1);
    glTranslatef(0, -dispHeight, 0);
    glMatrixMode(GL_MODELVIEW);
} 

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Initialization/Setup and Teardown ////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void init(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, dispWidth, dispHeight);
 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.0001, 1000.0);
 
    glMatrixMode(GL_MODELVIEW);
}

void doSpaceBarPush()
{
    if (gameState == 0) {
        // put the ball in motion
        ball.deltaX = 0.04;
        ball.deltaY = 0.09;
        gameState = 1;
    }
}

void resetPerspectiveProjection() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Callback Stubs ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void writeText(int x, int y, string text)
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    setOrthographicProjection();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1, 1, 1);

    glRasterPos3i(x, y, 0);
    
    for( int i = 0; i < text.length(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);  
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    
    resetPerspectiveProjection();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void printSegments(LineSegment* segments)
{
    printf("Segment[0] = (%f, %f) (%f, %f)\n", segments[0].a.x, segments[0].a.y, segments[0].b.x, segments[0].b.y);
    printf("Segment[1] = (%f, %f) (%f, %f)\n", segments[1].a.x, segments[1].a.y, segments[1].b.x, segments[1].b.y);
    printf("Segment[2] = (%f, %f) (%f, %f)\n", segments[2].a.x, segments[2].a.y, segments[2].b.x, segments[2].b.y);
    printf("Segment[3] = (%f, %f) (%f, %f)\n", segments[3].a.x, segments[3].a.y, segments[3].b.x, segments[3].b.y);
}

void drawBlock(Block b, bool isPaddle)
{
    if (!b.isActive) return;

    float blockHeight = 0.4;
    glPushMatrix();

    Point2D pos = b.position;
    RectSize size = b.size;

    glTranslatef(pos.x + (size.w / 2.0), 0.2, -(pos.y + size.h/2.0));
    glScalef(size.w, blockHeight, size.h);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    if (!isPaddle) {
        switch(b.hpRemaining) {
            case 3:
                glColor3f(0.8,0.9,0.2);
                break;
            case 2:
                glColor3f(0.8,0.3,0.2);
                break;
            case 1:
                glColor3f(0.8,0.1,0.1);
                break;
            default:
                glColor3f(0.8,0.9,0.2);
        }
        glBindTexture(GL_TEXTURE_2D, blockTex);
    } else {
        glColor3f(0.5,0.4,0.3);
        glBindTexture(GL_TEXTURE_2D, paddleTex);
    }
    drawCube();
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void drawBall(Ball b)
{
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();

    glColor3f(0.6, 0.6, 0.6);

    Point2D pos = b.center;
    float radius = b.radius;

    glTranslatef(pos.x, 0.35, -(pos.y));

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, ambient);

    glPushName(22);
    glutSolidSphere(radius, 35, 35);
    glPopName();

    glPopMatrix();
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
        }
    }

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
        if (livesRemaining > 0) {
            livesRemaining--;
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
            doSpaceBarPush();
        default:
            break;
    }
}


// not exactly a callback, but sets a timer to call itself
// in an endless loop to update the program
void idle( int value ){
    
    // if the user wants to quit the program, then exit the
    // function without resetting the timer or triggering
    // a display update
    if( quit ){
        // perform hard exit of the program, since glutMainLoop()
        // will never return
        exit(0);
    }
    
    // set a timer to call this function again after the
    // required number of milliseconds
    glutPostRedisplay();
    glutTimerFunc( dt, idle, 0 );
}

void drawFloor() {
    // Draw a floor. Since it is transparent, we need to do it AFTER all of
    // the opaque objects.
    
    glPushMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glScalef(10, 1, 10);
    glTranslatef(0.5, -0.5, -0.5);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glBindTexture(GL_TEXTURE_2D, floorTex);
    drawCube();
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
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
        break;  
        case GLUT_KEY_DOWN:
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
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    if (cameraMode == 1)
    {
        currViewpointRotation = fmin(currViewpointRotation + 0.5, 60);
        glRotatef(currViewpointRotation, 1, 0, 0);

        float progress = (currViewpointRotation / 60);
        glTranslatef( -5.0, -5.6 * progress, 0.0 );
    } else if (cameraMode == 2) {

        currViewpointRotation = fmax(currViewpointRotation - 0.5, 20);

        float progress = 20 / currViewpointRotation;
        glRotatef(currViewpointRotation, 1, 0, 0);
        glTranslatef(-(paddle->position.x + (paddle->size.w/2.0)*progress), -2.5/progress, -2.0 * progress);
    }

    stringstream ss;
    ss << "FPS: " << fps;
    writeText(10, 40, ss.str());

    ss.str("");
    ss << "Score: " << currentScore;
    writeText(10, 80, ss.str());

    ss.str("");
    ss << "Lives: " << livesRemaining;
    writeText(10, 120, ss.str());

    if (gameState == 1) {
        updateBallPositionAndVelocity();    
    } else if (gameState == 0) {
        // draw ball in middle of paddle
        ball.center.x = paddle->position.x + (paddle->size.w)/2.0;
        ball.center.y = paddle->position.y + paddle->size.h + ball.radius;
    }
    
    drawBall(ball);

    //drawPaddle();
    drawBlock(*paddle, true);

    int i;
    for (i = 0; i < blockVector.size(); i++)
    {
        drawBlock(blockVector[i], false);
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
        fps = numFrames/ (elapsedTime/1000);
        //printf ("FPS: %f.\n", numFrames/ (elapsedTime/1000));
        start = now;
        numFrames = 0;
    }
    numFrames++;
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
    GLuint buff[64];
    GLint hits, view[4];
    int id;
 
    /*
        This choose the buffer where store the values for the selection data
    */
    glSelectBuffer(64, buff);
 
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
    //glPushName(99);
 
    /*
        Now modify the vieving volume, restricting selection area around the cursor
    */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
 
        /*
            restrict the draw to an area around the cursor
        */
        gluPickMatrix(x, y, 0.2, 0.2, view);
        //gluPerspective(60, 1.0, 0.0001, 1000.0);
        gluPerspective(70.0, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.0001, 1000.0);

        /*
            Draw the objects onto the screen
        */
        glMatrixMode(GL_MODELVIEW);
 
        /*
            draw only the names in the stack, and fill the array
        */
        //glutSwapBuffers();
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
    
    //gl_selall(hits, buff);
    
 
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
    loadTexture("electric.bmp", &blockTex);
    loadTexture("floor-tile.bmp", &floorTex);
    loadTexture("metal-tex2.bmp", &paddleTex);

    // lighting stuff
    GLfloat ambient[] = {0.3, 0.3, 0.3, 1.0};
    GLfloat diffuse[] = {0.9, 0.9, 0.9, 1.0};
    GLfloat specular[] = {0.8, 0.8, 0.8, 1.0};    
    GLfloat position1[] = {2.0, 5.0, 0, 0.0};
    glLightfv( GL_LIGHT1, GL_POSITION, position1 );
    glLightfv( GL_LIGHT1, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT1, GL_SPECULAR, specular );
    
    glEnable( GL_LIGHTING );
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