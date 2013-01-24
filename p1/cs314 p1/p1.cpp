#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>

int dumpPPM(int frameNum);
void drawAxis();
void drawFloor();

unsigned char camera = 'r';

int iCount = 0;       // used for numbering the PPM image files
int Width = 800;      // window width (pixels)
int Height = 800;     // window height (pixels)
bool Dump=false;      // flag set to true when dumping animation frames
int rotation = 0;

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
        case 'm':
            camera = 'm';
            break;
        case 'i':
            dumpPPM(iCount);
            iCount++;
            break;
        case 'd':               // dump animation PPM frames
            iCount = 0;         // image file count
            Dump = !Dump;
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
    
    // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    // Draw your rabbit here
    // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    
//    // testing...
//    glPushMatrix();
//    
//    glLoadIdentity();
//    
//    glTranslatef(1, 1, 0);
//    glRotatef(90, 1, 0, 0);
//    glPushMatrix();
//    glScalef(1, 2, 1);
//    glTranslatef(1, 1, 0);
//    
//    glPopMatrix();
//    glTranslatef(1, 1, 0);
//    
//    
//    GLfloat matrix[16];
//    glGetFloatv (GL_MODELVIEW_MATRIX, matrix);
//    
//    // http://stackoverflow.com/questions/4360918/correct-opengl-matrix-format
//    printf("%f %f %f %f \n", matrix[0], matrix[4], matrix[8], matrix[12]);
//    printf("%f %f %f %f \n", matrix[1], matrix[5], matrix[9], matrix[13]);
//    printf("%f %f %f %f \n", matrix[2], matrix[6], matrix[10], matrix[14]);
//    printf("%f %f %f %f \n", matrix[3], matrix[7], matrix[11], matrix[15]);
//    
//    glPopMatrix(); // end testing
    
        
    glPushMatrix(); // start rabbit

    glColor3f(1, 1, 1);
    
    glPushMatrix(); // start torso
    
    glScalef(3, 2, 1.5);
    glTranslatef(0, 0.8, 0);
    glutSolidCube(1);
    
    glPopMatrix();  // end torso
        
    glPushMatrix(); // start tail
    
    glScalef(1, 1, 1);
    glTranslatef(1.5, 2.2, 0);
    glColor3f(1, 1, 0.5);
    glutSolidCube(1);
    glTranslatef(0.5, 0, 0);
    glScalef(0.55, 0.7, 0.7);
    glColor3f(1, 0.2, 0.5);
    glutSolidCube(1);
    
    glPopMatrix();  // end tail
    
    glPushMatrix(); // start head
    
    glTranslatef(-2.0, 3, 0);
    glScalef(1, 1.2, 0.7);
    glColor3f(0.5, 0.2, 0.5);
    glutSolidCube(1);
    
    glColor3f(0.5, 0.5, 0.5);
    glScalef(0.1, 0.2, 0.4);
    glTranslatef(-5, 1.2, -0.7);
    glColor3f(1, 0.9, 0.3);
    glutSolidCube(1);
    
    glTranslatef(0, 0, 1.4);
    glutSolidCube(1);
    
    glPopMatrix();  // end head
    
    glPopMatrix(); // end rabbit
    
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

void drawHead() {
    
}

void drawBody() {
    
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
    glutInitDisplayMode( GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE );
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
    
    // pass control over to GLUT
    glutMainLoop();
    
    return 0;       // never reached
}

