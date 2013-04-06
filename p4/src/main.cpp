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

GLuint blockTex;
GLuint floorTex;
GLuint paddleTex;

time_t startOfSample = time(NULL);
timeval start;
int numFrames = 1;
float fps = 30.0;

float currViewpointRotation = 30.0;
float currViewpointTranslateX = -5;
float currViewpointTranslateY = -5;
float currViewpointTranslateZ = -5;

void drawFloor();
void doSelection(int x, int y);
void writeText(int x, int y, string text);
void display();
void updateBallPositionAndVelocity();
void drawBall(Ball b);
void drawBlock(Block b, bool isPaddle);
void doSpaceBarPush();


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Initialization/Setup and Teardown ////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// Initialize the viewport and perspective projection
void init(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, dispWidth, dispHeight);
 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.0001, 1000.0);
 
    glMatrixMode(GL_MODELVIEW);
}

// Load the texture and associate it with texName
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

// Set the projection to orthographic
void setOrthographicProjection() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, dispWidth, 0, dispHeight);
    glScalef(1, -1, 1);
    glTranslatef(0, -dispHeight, 0);
    glMatrixMode(GL_MODELVIEW);
} 

// Reset the projection to the original perspective projection
void resetPerspectiveProjection() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//
//
// callbacks
//
//

// The display callback, used to generate each frame
void display()
{

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

    if (gameState == 3) {
        ss.str("");
        ss << "Game Over! Push spacebar to play again. You are awesome!";
        writeText(10, 1000, ss.str());
    }

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

// Keyboard callback
void keyboardCallback( unsigned char key, int x, int y ){
    switch( key ){
        case 'q':
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

// Handles a space bar action
void doSpaceBarPush()
{
    if (gameState == 0) {
        // put the ball in motion
        ball.deltaX = 0.04;
        ball.deltaY = 0.09;
        gameState = 1;
    } else if (gameState == 3) {
        livesRemaining = 5;
        currentScore = 0;
        gameState = 0;
        // position ball to start
        ball.center.x = paddle->position.x + (paddle->size.w)/2.0;
        ball.center.y = paddle->position.y + paddle->size.h + ball.radius;
        for (int i = 0; i < blockVector.size(); i++)
        {
            blockVector[i].isActive = true;
            blockVector[i].hpRemaining = 3;
        }   
    }
}

// Sets a timer to call itself
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

// Handles the left and right arrow keys
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

//
//
// drawing
//
//

// Draws a ball at it's current position
void drawBall(Ball b)
{

    static GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
    static GLfloat ambient[] = {0.19225, 0.19225, 0.19225};
    static GLfloat diffuse[] = {0.50754, 0.50754, 0.50754};
    static GLfloat specular[] = {0.508273, 0.508273, 0.508273};
    static GLfloat shininess[] = {0.4 * 128};

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();

    glColor3f(b.red, b.green, b.blue);

    Point2D pos = b.center;
    float radius = b.radius;

    glTranslatef(pos.x, 0.35, -(pos.y));

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, ambient);

    glLoadName(2); glPushName(20);
    glutSolidSphere(radius, 35, 35);
    glPopName();

    glPopMatrix();
}

// Draws a unit cube
void drawCube() {
    glutSolidCube(1);
}

// Draws a block at it current position.
// The isPaddle flag should be set for drawing the paddle block
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
        glColor3f(0,0,0);
        glBindTexture(GL_TEXTURE_2D, paddleTex);
    }
    drawCube();
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

// Draws the floor
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

// Writes the text at the given (x,y) position on the screen
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


//
//
// Positioning/collision
//
//

// Checks for intersection between the ball and block and 
// updates ball position and block attributes if necessary
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

// Updates the ball position and velocity based on any collisions
// detected between the ball and each block in the scene
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


//
//
// Picking
//
// 

// Handles a mouse down event
void doMouseDown(int x, int y, int but)
{
    printf("Mouse button %d pressed at %d %d\n", but, x, y);
    for (int i = 0; i < 100; i++){
        doSelection(x,dispHeight-y); //Important: gl (0,0) ist bottom left but window coords (0,0) are top left so we have to change this!
    }
}

// Handles a mouse click
void mouseClick(int button, int state, int x, int y)
 {
    if  ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
        doMouseDown(x, y, button);
    }
}

// Handles the hits detected in selection mode
void handleHits( GLint hits, GLuint* nameBuffer )
{
    unsigned int i;
    unsigned int j;
    GLuint names;
    GLuint* pPtr;

    printf( "hits = %d\n", hits );
    pPtr = nameBuffer;

    if (hits == 3) {
        ball.red = (rand() % 100) / 100.0;
        ball.green = (rand() % 100) / 100.0;
        ball.blue = (rand() % 100) / 100.0;
    }

    for( i = 0; i < hits; ++i )
    {
        names = *pPtr;
        printf("number of names for hit = %u\n", names);
        ++pPtr;

        printf( "min depth = %f\t", *pPtr/(pow(2.0,32.0)-1.0) );
        ++pPtr;

        printf( "max depth = %f\n", *pPtr/(pow(2.0,32.0)-1.0) );
        ++pPtr;

        printf("Names:\t");
        for( j = 0; j < names; ++j, ++pPtr )
        {
            printf( "%u\t", *pPtr );
        }
        printf("\n\n");     
    }
    printf("\n\n");  
}

// Performs a selection at the given coordinates
void doSelection(int x, int y)
{
    GLuint buff[50];
    GLint hits, view[4];
    int id;
    GLfloat projMatrix[16];

    glSelectBuffer(50, buff);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(70);
    glGetFloatv( GL_PROJECTION_MATRIX, projMatrix );
 
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glGetIntegerv(GL_VIEWPORT, view);
    gluPickMatrix(x, y-150, 50, 50, view);
    glMultMatrixf( projMatrix ); /* post multiply the "current" projection matrix */
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    display()
    ;
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    hits = glRenderMode(GL_RENDER);

    if( -1 != hits ) {
        handleHits( hits,buff);
    } else {
        printf("Hit selection error\n" );
    }
}


//
//
// Entry point
//
//

int main(int argc, char **argv)
{
    // create window and rendering context
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize( dispWidth, dispHeight );
    glutCreateWindow( "Wall Buster" );
    
    // register display callback
    glutDisplayFunc( display )
    ;
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
    
    // layout the level
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