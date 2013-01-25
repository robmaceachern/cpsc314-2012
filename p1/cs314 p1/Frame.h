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
    BODY_NECK,
    BODY_RIGHT_REAR_LEG, RIGHT_REAR_LOWER_LEG,
    BODY_LEFT_REAR_LEG, LEFT_REAR_LOWER_LEG,
    BODY_RIGHT_FRONT_LEG, RIGHT_FRONT_LOWER_LEG,
    BODY_LEFT_FRONT_LEG, LEFT_FRONT_LOWER_LEG,
    HEAD_RIGHT_EAR, HEAD_LEFT_EAR,
    BODY_REAR, BODY_CHEST,
    NECK_HEAD, MAIN_BODY, NUMBER_OF_ANGLE_KEYS
};

struct Point {
    
    float x, y, z;
    
    Point(float x1, float y1, float z1) {
        x = x1; y = y1; z = z1;
    }
    
    Point() {
        x = 0;
        y = 0;
        z = 0;
    }

};

class Frame {
    
    // http://stackoverflow.com/questions/2102582/how-can-i-count-the-items-in-an-enum
    float rotationAngles[NUMBER_OF_ANGLE_KEYS];
    Point point;
    
public:
    
    float getRotationAngle(AngleKey key);
    void setRotationAngle(AngleKey key, float val);
    
    Point getPoint();
    void setPoint(Point pt);
    
    bool equalWithinRange(Frame other, float epsilon);
    
    Frame copy();

};

#endif /* defined(__cs314_p1__Frame__) */
