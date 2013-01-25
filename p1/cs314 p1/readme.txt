CPSC 314
Project 1

Robert MacEachern
m9c7
42415091

By submitting this file, I hereby declare that I worked individually on this assignment and that I am the only author of this code. I have listed all external resoures (web pages, books) used below. I have listed all people with whom I have had significant discussions about the project below.

Some places I took inspiration from:

Cube artwork
http://blenderartists.org/forum/showthread.php?124116-Cube-modeling-challenge-(VOTING-OPEN!-SEE-PAGE-30)/page3

A few times I used stack overflow for C++ related questions I had.


Functionality recap

Jumpcut and smooth transitions for:
Head nod
Jump
Rear up on hind legs
Leg raises
Ear wiggles
Body curl

My smoothness function (interpolateFrames) interpolates between the current frame and the 'goal' frame,
which has the desirable property that it eases the transitions between frames. It is based on the wall clock 
time and it does not waste CPU while there is nothing to animate.

Additional cool things:
- turned on multisampling to improve display performance (smooths jagged edges and makes things pretty)
    glutInitDisplayMode( GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    
- random spin
    push 's' to spin the rabbit and get some interesting views. Looks cool while he's jumping!
    


