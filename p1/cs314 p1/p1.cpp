#ifdef __APPLE__
#  include <GLUT/glut.h>
#  include "Frame.h"
#else
#  include "Frame.cpp"
#  include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

enum AnimationMode {
    JUMPCUT, SMOOTH
};

//enum Animation {
//    JUMP, CURL, REAR, LEFT_FRONT_LEG, LEFT_REAR_LEG, RIGHT_FRONT_LEG, RIGHT_REAR_LEG,
//    RIGHT_EAR, LEFT_EAR, HEAD_NOD
//};

int dumpPPM(int frameNum);
void drawAxis();
void drawFloor();
void drawRabbit(Frame frame);
void drawHead();
void drawCube();
void drawScaledCube(float x, float y, float z);
void toggleHeadNod();
Frame interpolateFrames(Frame currentFrame, Frame goalFrame, int elapsedTime);
void idle();
void toggleLegRaise(AngleKey legId);
void toggleEarWiggle(AngleKey earId);
void toggleJump();
void toggleCurl();
void toggleRear();
void toggleSpin();
void prepAnimation();

unsigned char camera = 'r';

int iCount = 0;       // used for numbering the PPM image files
int Width = 800;      // window width (pixels)
int Height = 800;     // window height (pixels)
bool Dump=false;      // flag set to true when dumping animation frames
int rotation = 0;
Frame restFrame;
Frame goalFrame;
Frame currentFrame;
AnimationMode animationMode = JUMPCUT;
int lastAnimationTime = -1;

void keyboardCallback(unsigned char c, int x, int y) {
    switch (c) {
        case 'q':
            exit (0);
            break;
        case 'p':
            camera = 'p';
            break;
        case 'f':
            camera = 'f';
            break;
        case 'b':
            camera = 'b';
            break;
        case 'a':
            camera = 'a';
            break;
        case 'u':
            camera = 'u';
            break;
        case 'r':
            camera = 'r';
            break;
        case 'i':
            dumpPPM(iCount);
            iCount++;
            break;
        case 'd':               // dump animation PPM frames
            iCount = 0;         // image file count
            Dump = !Dump;
            break;
        case 'h':
            printf("Nodding head...\n");
            toggleHeadNod();
            break;
        case 'l':
            printf("Moving left front leg...\n");
            toggleLegRaise(BODY_LEFT_FRONT_LEG);
            break;
        case 'm':
            printf("Moving right front leg...\n");
            toggleLegRaise(BODY_RIGHT_FRONT_LEG);
            break;
        case 'n':
            printf("Moving left rear leg...\n");
            toggleLegRaise(BODY_LEFT_REAR_LEG);
            break;
        case 'o':
            printf("Moving right rear leg...\n");
            toggleLegRaise(BODY_RIGHT_REAR_LEG);
            break;
        case 'e':
            printf("Wiggling right ear...\n");
            toggleEarWiggle(HEAD_RIGHT_EAR);
            break;
        case 'w':
            printf("Wiggling left ear...\n");
            toggleEarWiggle(HEAD_LEFT_EAR);
            break;
        case 'j':
            printf("Jumping...\n");
            toggleJump();
            break;
        case 'c':
            printf("Curling (hurry hard!)...\n");
            toggleCurl();
            break;
        case 't':
            printf("Rearing...\n");
            toggleRear();
            break;
        case 's':
            printf("Spinning...\n");
            toggleSpin();
            break;
        case ' ':
            if (animationMode == JUMPCUT) {
                printf("Switching to SMOOTH animation mode\n");
                animationMode = SMOOTH;
                glutIdleFunc(idle);
            } else {
                printf("Switching to JUMPCUT animation mode\n");
                animationMode = JUMPCUT;
                glutIdleFunc(NULL);
            }
            break;
    }
    
    glutPostRedisplay();
}

void reshapeCallback(int w, int h)
{
    Width = w;
    Height = h;
    glViewport(0, 0, w, h);
}

void displayCallback()
{
    // clear the color buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // set up camera
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45, 1.0, 0.1, 200.0 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0, -3.0, -10.0 );
    switch (camera) {
        case 'p':
            glRotatef( 180, 0.0, 1.0, 0.0 );
            break;
        case 'f':
            glRotatef( -90, 0.0, 1.0, 0.0 );
            break;
        case 'b':
            glRotatef( 90, 0.0, 1.0, 0.0 );
            break;
        case 'a':
            glRotatef( 90, 1.0, 0.0, 0.0 );
            break;
        case 'u':
            glRotatef( -90, 1.0, 0.0, 0.0 );
            break;
        case 'm':
            rotation = (rotation + 10) % 360;
            glRotatef( rotation, 0.0, 1.0, 0.0 );
            break;
        case 'r':
            break;
    }
    
    drawAxis();
    
    if (animationMode == JUMPCUT) {
        currentFrame = goalFrame;
    } else {
        int elapsed = glutGet(GLUT_ELAPSED_TIME);
        
        Frame interpolated = interpolateFrames(currentFrame, goalFrame, elapsed - lastAnimationTime);
        lastAnimationTime = elapsed;
        
        // We'll check if the interpolated frame is basically the same as the goal frame.
        // If it is, we'll stop animating for now.
        if (interpolated.equalWithinRange(goalFrame, 0.1)) {
            currentFrame = goalFrame;
            glutIdleFunc( NULL );
        } else {
            currentFrame = interpolated;
            glutIdleFunc(idle);
        }
    }
    
    glPushMatrix();
    drawRabbit(currentFrame);
    glPopMatrix();
    
    // draw after the opaque objects, since it is translucent
    drawFloor();
    
    // draw the buffer to the screen
    glutSwapBuffers();
    
    if (Dump) {               // save images to file
        dumpPPM(iCount);
        iCount++;
    }
    
    GLenum error = glGetError();
    if(error != GL_NO_ERROR)
        printf("ERROR: %s\n", gluErrorString(error));
}

// Draws the rabbit based on the angles and positions in the frame
void drawRabbit(Frame frame) {
    
    glColor3f(0.32, 0.31, 0.33);
    
    glRotated(frame.getRotationAngle(MAIN_BODY_SPIN), 0, 1, 0);
    
    // center body piece - root of object hierarchy
    Point bodyPoint = frame.getPoint();
    glTranslatef(0, 2, 0);
    glRotated(frame.getRotationAngle(MAIN_BODY), 0, 0, 1);
    glTranslatef(0, -2, 0);
    glTranslatef(bodyPoint.x, bodyPoint.y, bodyPoint.z);
    drawScaledCube(1.7, 1.2, 1.3);
    
    // the front body piece
    glPushMatrix();
    glTranslatef(0.8, -0.1, 0);
    glRotatef(frame.getRotationAngle(BODY_CHEST), 0, 0, 1);
    glTranslatef(0.4, 0.1, 0);
    drawScaledCube(1.3, 1.2, 1.3);

    glPushMatrix();             // neck
    glTranslatef(0.3, 0.5, 0);
    glRotatef(frame.getRotationAngle(BODY_NECK), 0, 0, 1);
    drawScaledCube(0.3, 0.7, 0.7);
    
    // the head
    glTranslatef(0.7, 0.5, 0);
    glRotatef(frame.getRotationAngle(NECK_HEAD), 0, 0, 1);
    drawScaledCube(1.3, 1.0, 1.0);
    
    glPushMatrix();             // right ear - e
    glTranslatef(-0.1, 0.4, 0.4);
    glRotatef(frame.getRotationAngle(HEAD_RIGHT_EAR), 1, 0, 0);
    glTranslatef(0, 0.6, 0.0);
    drawScaledCube(0.4, 1.3, 0.2);
    glPopMatrix();
    
    glPushMatrix();             // left ear - w
    glTranslatef(-0.1, 0.4, -0.4);
    glRotatef(frame.getRotationAngle(HEAD_LEFT_EAR), 1, 0, 0);
    glTranslatef(0, 0.6, 0.0);
    drawScaledCube(0.4, 1.3, 0.2);
    glPopMatrix();
    
    glColor3f(0.9, 0.9, 0.9);
    
    glPushMatrix();             // right eye
    glTranslatef(0.7, 0.3, 0.3);
    drawScaledCube(0.2, 0.3, 0.3);
    glPopMatrix();              // end right eye
    
    glPushMatrix();             // left eye
    glTranslatef(0.7, 0.3, -0.3);
    drawScaledCube(0.2, 0.3, 0.3);
    glPopMatrix();              // end left eye
    
    glColor3f(0.32, 0.31, 0.33); // back to gray
    
    glPopMatrix();              // end neck
    
    glPushMatrix();             // right front leg
    glTranslatef(0.4, -0.6, 0.5);
    glRotatef(frame.getRotationAngle(BODY_RIGHT_FRONT_LEG), 0, 0, 1);
    drawScaledCube(0.2, 0.6, 0.1);
    // right front paw
    glTranslatef(0.1, -0.35, 0);
    glRotatef(frame.getRotationAngle(RIGHT_FRONT_LOWER_LEG), 0, 0, 1);
    drawScaledCube(0.3, 0.1, 0.1);
    glPopMatrix();              // end right front leg
    
    glPushMatrix();             // left front leg
    glTranslatef(0.4, -0.6, -0.5);
    glRotatef(frame.getRotationAngle(BODY_LEFT_FRONT_LEG), 0, 0, 1);
    drawScaledCube(0.2, 0.6, 0.1);
    // left front paw
    glTranslatef(0.1, -0.35, 0);
    glRotatef(frame.getRotationAngle(LEFT_FRONT_LOWER_LEG), 0, 0, 1);
    drawScaledCube(0.3, 0.1, 0.1);
    glPopMatrix();              // end left front leg
    
    glPopMatrix();              // end front body
    
    glPushMatrix();             // rear body piece
    glTranslated(-0.3, 0.3, 0);
    glRotatef(frame.getRotationAngle(BODY_REAR), 0, 0, 1);
    glTranslatef(-1.1, -0.3, 0);
    drawScaledCube(1.4, 1.4, 1.2);
    
    glPushMatrix();             // right rear leg
    // upper right leg
    glTranslatef(0, -0.5, 0.55);
    glRotatef(frame.getRotationAngle(BODY_RIGHT_REAR_LEG), 0, 0, 1);
    drawScaledCube(0.7, 0.7, 0.3);
    // lower right leg
    glTranslatef(0.2, -0.4, 0);
    glRotatef(frame.getRotationAngle(RIGHT_REAR_LOWER_LEG), 0, 0, 1);
    drawScaledCube(1, 0.1, 0.3);
    glPopMatrix();              // end right rear leg
    
    glPushMatrix();             // left rear leg
    // upper left leg
    glTranslatef(0, -0.5, -0.55);
    glRotatef(frame.getRotationAngle(BODY_LEFT_REAR_LEG), 0, 0, 1);
    drawScaledCube(0.7, 0.7, 0.3);
    //lower left leg
    glTranslatef(0.2, -0.4, 0);
    glRotatef(frame.getRotationAngle(LEFT_REAR_LOWER_LEG), 0, 0, 1);
    drawScaledCube(1, 0.1, 0.3);
    glPopMatrix();              // end left rear leg
    
    glPushMatrix();             // tail
    glColor3f(0.9, 0.9, 0.9);
    glTranslatef(-0.7, 0.6, 0);
    drawScaledCube(0.5, 0.5, 0.5);
    glPopMatrix();              // end tail
    
    glPopMatrix();              // end rear body
}

void drawCube() {
    glutSolidCube(1);
}

void idle() {
    glutPostRedisplay();
}

void drawScaledCube(float x, float y, float z) {
    glPushMatrix();
    glScalef(x, y, z);
    drawCube();
    glPopMatrix();
}

// Updates the goal frame neck angles
void toggleHeadNod() {
    
    prepAnimation();
    // determine if head is already being nodded
    float currentNeckHead = currentFrame.getRotationAngle(NECK_HEAD);
    
    if (currentNeckHead < 0) {
        // We will bring the head back up
        goalFrame.setRotationAngle(NECK_HEAD, 6);
        goalFrame.setRotationAngle(BODY_NECK, 6);
    } else {
        // Drop the head down
        goalFrame.setRotationAngle(NECK_HEAD, -20);
        goalFrame.setRotationAngle(BODY_NECK, -20);
    }
}

// Returns an interpolated frame.
Frame interpolateFrames(Frame currentFrame, Frame goalFrame, int elapsedTime) {
    
    Frame interpolated;
    float timeFactor = 240.0;
    float multiplier =  fminf(0.1, elapsedTime/timeFactor);
    
    // Iterating through our set of angle keys, interpolating if necessary
    for (AngleKey i = BODY_NECK; i != NUMBER_OF_ANGLE_KEYS; i = static_cast<AngleKey>(i + 1)) {
        float curr = currentFrame.getRotationAngle(i);
        float goal = goalFrame.getRotationAngle(i);
        
        float interpolatedAngle =  curr + (goal - curr) * multiplier;
        
        interpolated.setRotationAngle(i, interpolatedAngle);
    }
    Point currPoint = currentFrame.getPoint();
    Point goalPoint = goalFrame.getPoint();
    
    float interpolatedX = currPoint.x + (goalPoint.x - currPoint.x) * multiplier;
    float interpolatedY = currPoint.y + (goalPoint.y - currPoint.y) * multiplier;
    float interpolatedZ = currPoint.z + (goalPoint.z - currPoint.z) * multiplier;
    
    interpolated.setPoint(Point(interpolatedX, interpolatedY, interpolatedZ));
    
    return interpolated;
}

// Alter's the goalFrame's leg angles depending on current position and the leg specified
void toggleLegRaise(AngleKey legId) {
    
    prepAnimation();
    
    if (legId == BODY_LEFT_FRONT_LEG || legId == BODY_RIGHT_FRONT_LEG) {
        // front leg
        float raiseAngle = 90;
        AngleKey lowerLeg;
        
        if (legId == BODY_RIGHT_FRONT_LEG) {
            lowerLeg = RIGHT_FRONT_LOWER_LEG;
        } else {
            lowerLeg = LEFT_FRONT_LOWER_LEG;
        }
        
        bool shouldRaise = (currentFrame.getRotationAngle(legId) < raiseAngle -30);
        
        if (shouldRaise) {
            goalFrame.setRotationAngle(legId, raiseAngle);
            
            // lower leg
            float lowerLegAngle = -80;
            goalFrame.setRotationAngle(lowerLeg, lowerLegAngle);
            
        } else {
            goalFrame.setRotationAngle(legId, restFrame.getRotationAngle(legId));
            goalFrame.setRotationAngle(lowerLeg, restFrame.getRotationAngle(lowerLeg));
        }
        
        
    } else if (legId == BODY_LEFT_REAR_LEG || legId == BODY_RIGHT_REAR_LEG) {
        // rear leg
        float raiseAngle = 45;
        
        bool shouldRaise = (currentFrame.getRotationAngle(legId) < raiseAngle - 20);
        
        if (shouldRaise) {
            goalFrame.setRotationAngle(legId, raiseAngle);
        } else {
            goalFrame.setRotationAngle(legId, restFrame.getRotationAngle(legId));
        }
    }
    
    printf("TODO leg raise \n");
}

// Updates the ear angles on the goalFrame
void toggleEarWiggle(AngleKey earId) {
    
    prepAnimation();

    float wiggleAngle;
    bool shouldWiggle;
    
    if (earId == HEAD_RIGHT_EAR) {
        wiggleAngle = 170;
        shouldWiggle = (currentFrame.getRotationAngle(earId) < wiggleAngle - 45);
    } else if (earId == HEAD_LEFT_EAR) {
        wiggleAngle = -170;
        printf("left ear angle current: %f", currentFrame.getRotationAngle(earId));
        shouldWiggle = (currentFrame.getRotationAngle(earId) > wiggleAngle + 45);
    }
    
    if (shouldWiggle) {
        goalFrame.setRotationAngle(earId, wiggleAngle);
    } else {
        goalFrame.setRotationAngle(earId, restFrame.getRotationAngle(earId));
    }
    
    printf("TODO ear wiggle \n");
}

// Updates multiple goalFrame angles when jumping
void toggleJump() {
    
    prepAnimation();
    
    float height = 3.5;
    
    // we will jump up if we're currently lower than jump height
    bool shouldJump = currentFrame.getPoint().y < height - 1;
    
    if (shouldJump) {
        goalFrame.setPoint(Point(0, height, 0));
        goalFrame.setRotationAngle(BODY_REAR, -10);
        goalFrame.setRotationAngle(BODY_CHEST, 20);
        goalFrame.setRotationAngle(BODY_LEFT_REAR_LEG, -40);
        goalFrame.setRotationAngle(BODY_RIGHT_REAR_LEG, -40);
        goalFrame.setRotationAngle(BODY_LEFT_FRONT_LEG, 40);
        goalFrame.setRotationAngle(BODY_RIGHT_FRONT_LEG, 40);
        goalFrame.setRotationAngle(BODY_NECK, 20);
        goalFrame.setRotationAngle(NECK_HEAD, 10);
    } else {
        // back to rest
        goalFrame = restFrame.copy();
    }
}

void toggleCurl() {
    
    prepAnimation();
    
    float curlRate = 35;
    
    bool isCurled = (currentFrame.getRotationAngle(BODY_CHEST) == -curlRate);
    
    if (isCurled) {
        // return to rest position
        goalFrame = restFrame.copy();
    } else {
        goalFrame.setRotationAngle(BODY_REAR, curlRate);
        goalFrame.setRotationAngle(BODY_CHEST, -curlRate);
        goalFrame.setRotationAngle(BODY_NECK, -curlRate * 2);
        goalFrame.setRotationAngle(NECK_HEAD, -curlRate);
    }
}

void toggleRear() {
    prepAnimation();
    
    float rearAngle = 75;
    
    bool shouldRear = currentFrame.getRotationAngle(MAIN_BODY) < rearAngle - 30;
    
    if (shouldRear) {
        goalFrame.setRotationAngle(MAIN_BODY, rearAngle);
        goalFrame.setRotationAngle(BODY_RIGHT_REAR_LEG, -rearAngle);
        goalFrame.setRotationAngle(BODY_LEFT_REAR_LEG, -rearAngle);
        goalFrame.setRotationAngle(BODY_NECK, -rearAngle);
        goalFrame.setRotationAngle(LEFT_FRONT_LOWER_LEG, -rearAngle);
        goalFrame.setRotationAngle(RIGHT_FRONT_LOWER_LEG, -rearAngle);
    } else {
        goalFrame = restFrame.copy();
    }
    
    printf("TODO rear \n");
}

void toggleSpin() {
    float spinAmount = rand() % 361;
    goalFrame.setRotationAngle(MAIN_BODY_SPIN, spinAmount);
}

void prepAnimation() {
    lastAnimationTime = glutGet(GLUT_ELAPSED_TIME);
}

//---------------------------------------------------------------

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
    for (int x = -5; x < 5; x++) {
        for (int y = -5; y < 5; y++) {
            glColor4f(1, 1, 1, (x + y) % 2 ? 0.75 : 0.5);
            glNormal3f(0, 1, 0);
            glBegin(GL_POLYGON);
            glVertex3f(x    , 0, y    );
            glVertex3f(x + 1, 0, y    );
            glVertex3f(x + 1, 0, y + 1);
            glVertex3f(x    , 0, y + 1);
            glEnd();
        }
    }
    // visible from initial side angle
    glBegin(GL_POLYGON);
    glVertex3f(-5,    0, 0);
    glVertex3f(-5, -.05, 0);
    glVertex3f( 5, -.05, 0);
    glVertex3f( 5,    0, 0);
    glEnd();
    
    // visible from front angle
    glBegin(GL_POLYGON);
    glVertex3f(0,    0, -5);
    glVertex3f(0, -.05, -5);
    glVertex3f(0, -.05, 5);
    glVertex3f(0,    0, 5);
    glEnd();
}

int dumpPPM(int frameNum)
{
    FILE *fp;
    const int maxVal=255;
    register int y;
    unsigned char *pixels;
    
    glReadBuffer( GL_FRONT );
    char fname[100];
    sprintf(fname,"./ppm/img%03d.ppm",frameNum);
    fp = fopen(fname,"wb");
    if (!fp) {
        printf("Unable to open file '%s'\n",fname);
        return 1;
    }
    printf("Saving image `%s`\n",fname);
    fprintf(fp, "P6 ");
    fprintf(fp, "%d %d ", Width, Height);
    fprintf(fp, "%d", maxVal);
    putc(13,fp);
    pixels = new unsigned char [3*Width];
    
    y = 0;
    glReadPixels(0,Height-1,Width,1,GL_RGB,GL_UNSIGNED_BYTE, (GLvoid *) pixels);
    
    for ( y = Height-1; y >=0; y-- ) {
        glReadPixels(0,y,Width,1,GL_RGB,GL_UNSIGNED_BYTE, (GLvoid *) pixels);
        for (int n=0; n<3*Width; n++) {
            putc(pixels[n],fp);
        }
    }
    fclose(fp);
    delete [] pixels;
    return 0;
}

//---------------------------------------------------------------

int main(int argc, char **argv)
{
    // create window and rendering context
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize( Width, Height );
    glutCreateWindow( "Rabbit" );
    
    // register display callback
    glutDisplayFunc( displayCallback );
    glutKeyboardFunc( keyboardCallback );
    
    glViewport( 0, 0, Width, Height );
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
    
    // set our initial frame for the rabbit
    currentFrame = Frame();
    currentFrame.setPoint(Point(0, 1.5, 0));
    currentFrame.setRotationAngle(BODY_NECK, 6);
    currentFrame.setRotationAngle(NECK_HEAD, 6);
    currentFrame.setRotationAngle(BODY_RIGHT_REAR_LEG, 4);
    currentFrame.setRotationAngle(RIGHT_REAR_LOWER_LEG, 0);
    currentFrame.setRotationAngle(BODY_LEFT_REAR_LEG, 4);
    currentFrame.setRotationAngle(LEFT_REAR_LOWER_LEG, 0);
    
    currentFrame.setRotationAngle(BODY_LEFT_FRONT_LEG, 10);
    currentFrame.setRotationAngle(LEFT_FRONT_LOWER_LEG, 0);
    currentFrame.setRotationAngle(BODY_RIGHT_FRONT_LEG, 10);
    currentFrame.setRotationAngle(RIGHT_FRONT_LOWER_LEG, 0);
    
    currentFrame.setRotationAngle(HEAD_LEFT_EAR, -10);
    currentFrame.setRotationAngle(HEAD_RIGHT_EAR, 10);
    
    currentFrame.setRotationAngle(BODY_CHEST, -5);
    currentFrame.setRotationAngle(BODY_REAR, 5);

    restFrame = currentFrame.copy();
    goalFrame = currentFrame.copy();
    
    // pass control over to GLUT
    glutMainLoop();
    
    return 0;       // never reached
}

