//
//  Frame.h
//  cs314 p1
//
//  Created by Rob MacEachern on 2013-01-24.
//
//

#ifndef __cs314_p1__Frame__
#define __cs314_p1__Frame__

#include <iostream>
#include <vector>

enum AngleKey {
    BODY_NECK, NECK_HEAD, NUMBER_OF_ANGLE_KEYS
};

struct Point {
    
    float x, y, z;
    
    Point(float x1, float y1, float z1) {
        x = x1; y = y1; z = z1;
    }

};

class Frame {
    
    // http://stackoverflow.com/questions/2102582/how-can-i-count-the-items-in-an-enum
    float rotationAngles[NUMBER_OF_ANGLE_KEYS];
    Point point = Point(0, 0, 0);
    
public:
    
    float getRotationAngle(AngleKey key);
    void setRotationAngle(AngleKey key, float val);
    
    Point getPoint();
    void setPoint(Point pt);
    
    bool equalWithinRange(Frame other, float epsilon);

};

#endif /* defined(__cs314_p1__Frame__) */
