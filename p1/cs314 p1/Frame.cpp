//
//  Frame.cpp
//  cs314 p1
//
//  Created by Rob MacEachern on 2013-01-24.
//
//

#include <math.h>
#include "Frame.h"

float Frame::getRotationAngle(AngleKey key) {
    return rotationAngles[key];
}

void Frame::setRotationAngle(AngleKey key, float angle) {
    rotationAngles[key] = angle;
}

Point Frame::getPoint() {
    return Point(point.x, point.y, point.z);
}

void Frame::setPoint(Point pt) {
    point.x = pt.x;
    point.y = pt.y;
    point.z = pt.z;
}

bool Frame::equalWithinRange(Frame other, float epsilon) {
    // assume equality and search for differences
    
    Point p1 = point;
    Point pOther = other.point;
    
    if (fabs(p1.x - pOther.x) > epsilon || fabs(p1.y - pOther.y) > epsilon || fabs(p1.z - pOther.z) > epsilon) {
        return false;
    }
    
    for (AngleKey i = BODY_NECK; i != NUMBER_OF_ANGLE_KEYS; i = static_cast<AngleKey>(i + 1)) {
        float angle = rotationAngles[i];
        float otherAngle = other.rotationAngles[i];
        if (fabs(angle - otherAngle) > epsilon) {
            return false;
        }
    }
    
    return true;
}

Frame Frame::copy() {
    Frame copy;
    
    copy.setPoint(point);
    
    for (AngleKey i = BODY_NECK; i != NUMBER_OF_ANGLE_KEYS; i = static_cast<AngleKey>(i + 1)) {
        copy.setRotationAngle(i, rotationAngles[i]);
    }
    
    return copy;
}