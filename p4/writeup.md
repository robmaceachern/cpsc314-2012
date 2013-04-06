CPSC 314
Project 4: Wall Buster

Rob MacEachern
m9c7
4241509a

What:
I've developed a desktop version of the classic Breakout/Brick-Breaker style game. The goal is to destroy as many blocks as possible by directing a ball with a paddle. You lose lives when the ball reaches the bottom edge of the play area.

The game implements each part of the required framework in addition to collision detection and animations when switching between view modes.

How:
Below are descriptions of how some of the core components of the game were implemented.

Collision detection:
Collision detection is accomplished by calculating if any block line segment intersects with the ball. The key function for this is Primitives.h Ball::intersectsWith(LineSegment, Vec3*). When a collision is detected, the Vec3* argument is set to the value of the overlap between the two objects so that the calling function can respond to the collision and reposition the objects appropriately to resolve the collision. The general approach I used was detailed in http://doswa.com/2009/07/13/circle-segment-intersectioncollision.html.

The updateBallPositionAndVelocity handles the grunt work of updating the ball position on each frame while handling any collisions detected between walls or blocks.

Texture mapping:
My texture mapping makes use of the CImg library to load each texture bitmap file. Once they have been loaded, I process each image with OpenGL's texture framework and are then bound to each object during drawing with the GL_TEXTURE_GEN_S and GL_TEXTURE_GEN_T coordinate mapping.

Control panel text writing:
The writeText function takes a pair of x,y coordinates and a string to write and then utilizes glutBitmapCharacter to write each character in the string. The key things that happen in this function are:
- disabling GL_TEXTURE_2D and GL_LIGHTING
- set the projection to orthographic
- set the raster position based on the x,y coordinates
- reenabling GL_TEXTURE_2D and GL_LIGHTING
- reset the perspective projection

Howto:

The controls for the game are:

Spacebar: Launch ball from paddle
Left & right arrows: move the paddle from side-to-size
#1 key: switch to overhead view (default)
#2 key: switch to paddle view - an alternate way to play the classic game!
Mouse click on ball: click the ball to change the ball to a random colour

Sources:

Prior assignments and framework code

http://www.lighthouse3d.com/opengl/picking/index.php?openglway3

http://doswa.com/2009/07/13/circle-segment-intersectioncollision.html

http://www.opengl.org/discussion_boards/showthread.php/136442-what-are-the-codes-for-arrow-keys-to-use-in-glut-keyboard-callback-function

http://stackoverflow.com/questions/2150291/how-do-i-measure-a-time-interval-in-c

http://www.opengl.org/discussion_boards/showthread.php/136442-what-are-the-codes-for-arrow-keys-to-use-in-glut-keyboard-callback-function



