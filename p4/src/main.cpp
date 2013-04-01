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

#include <Primitives.h>
#include <FileParser.h>

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

vector<Block> blockVector;

Ball ball = Ball(0.3, Point2D(1,1));

Block* paddle;

int currentScore = 0;
int livesRemaining = 5;
int currentLevel = 0;

// 0 - ball on paddle
// 1 - in game
// 2 - level beat
// 3 - game over
int gameState = 0;

void drawCube() {
    glutSolidCube(1);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Initialization/Setup and Teardown ////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void init(){
    
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
    drawCube();

    glPopMatrix();
}

void drawBall(Ball b)
{
    glPushMatrix();

    Point2D pos = b.center;
    float radius = b.radius;

    glTranslatef(pos.x, 0.35, -(pos.y));
    glutSolidSphere(radius, 15, 15);

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
    if (cameraMode == 1)
    {
        glRotatef(60, 1, 0, 0);
        glTranslatef( -5.0, -5.0, 0 );
        drawAxis();
    } else if (cameraMode == 2) {
        glTranslatef(-(paddle->position.x + (paddle->size.w/2.0)), -1.5, -1.5);
        drawAxis();
    }
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 70.0f, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.1f, 2000.0f );

    glMatrixMode(GL_MODELVIEW);
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
    glutDisplayFunc( displayCallback );
    glutKeyboardFunc( keyboardCallback );
    glutSpecialFunc(SpecialInput);
    
    glViewport( 0, 0, dispWidth, dispHeight );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_NORMALIZE );
    
    // Turn on blending (for floor).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
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
    
    blockVector.push_back(Block(Point2D(2, 4), RectSize(4, 0.8)));
    blockVector.push_back(Block(Point2D(4, 8), RectSize(2, 0.8)));
    blockVector.push_back(Block(Point2D(6, 8), RectSize(2, 0.8)));
    blockVector.push_back(Block(Point2D(0, 2), RectSize(2, 0.8)));
    blockVector.push_back(Block(Point2D(2, 6), RectSize(1, 0.8)));
    blockVector.push_back(Block(Point2D(8, 5), RectSize(2, 0.8)));
    blockVector.push_back(Block(Point2D(6, 7), RectSize(1, 0.8)));
    blockVector.push_back(Block(Point2D(5, 6), RectSize(2, 0.8)));
    blockVector.push_back(Block(Point2D(3, 5), RectSize(1, 0.8)));

    // the paddle block
    Block paddleBlock = Block(Point2D(3, 0), RectSize(4, 0.8));
    paddleBlock.hpRemaining = std::numeric_limits<int>::max();
    paddle = &paddleBlock;

    idle( 0 );

    // pass control over to GLUT
    glutMainLoop();
    
    return 0;       // never reached
}

