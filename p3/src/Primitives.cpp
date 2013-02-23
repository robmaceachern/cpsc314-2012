///////////////////////////////////////////////////////////////////////////
//
//	simple raytracer
//
//	UBC CS 314 - Feburary 2013
//	instructor: Tamara Munzner [tmm@cs.ubc.ca] 
//	assignment 3
//
//	originally written by Gordon Wetzstein
//	updated by and questions should go to: Yufeng Zhu [mike323@cs.ubc.ca]
//	
///////////////////////////////////////////////////////////////////////////

#include <Primitives.h>

#include <math.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//	calculate the intersection of a plane the given ray
//	the ray has an origin and a direction, ray = origin + t*direction
//	find the t parameter, return true if it is between 0.0 and 1.0, false 
//	otherwise, write the results in following variables:
//	depth	- t \in [0.0 1.0]
//	posX	- x position of intersection point, nothing if no intersection
//	posY	- y position of intersection point, nothing if no intersection
//	posZ	- z position of intersection point, nothing if no intersection
//	normalX	- x component of normal at intersection point, nothing if no intersection
//	normalX	- y component of normal at intersection point, nothing if no intersection
//	normalX	- z component of normal at intersection point, nothing if no intersection
//
/////////////////////////////////////////////////////////////////////////////////
bool 
Plane::intersect(Ray ray, double *depth,
				 double *posX, double *posY, double *posZ,
				 double *normalX, double *normalY, double *normalZ)

{
	//////////*********** START OF CODE TO CHANGE *******////////////

	//////////*********** END OF CODE TO CHANGE *******////////////

	return true;
}

///////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//	calculate the intersection of a sphere the given ray
//	the ray has an origin and a direction, ray = origin + t*direction
//	find the t parameter, return true if it is between 0.0 and 1.0, false 
//	otherwise, write the results in following variables:
//	depth	- t \in [0.0 1.0]
//	posX	- x position of intersection point, nothing if no intersection
//	posY	- y position of intersection point, nothing if no intersection
//	posZ	- z position of intersection point, nothing if no intersection
//	normalX	- x component of normal at intersection point, nothing if no intersection
//	normalX	- y component of normal at intersection point, nothing if no intersection
//	normalX	- z component of normal at intersection point, nothing if no intersection
//
//	attention: a sphere has usually two intersection points make sure to return 
//	the one that is closest to the ray's origin and still in the viewing frustum
//
/////////////////////////////////////////////////////////////////////////////////
bool 
Sphere::intersect(Ray ray, double *depth,	
				  double *posX, double *posY, double *posZ,
				  double *normalX, double *normalY, double *normalZ)

{
	//////////*********** START OF CODE TO CHANGE *******////////////

	printf("ray.x: %f, ray.y: %f, ray.z: %f \n", ray.direction[0], ray.direction[1], ray.direction[2]);
	
	// from slides:
	// (cx + t * vx)^2 + (cy + t * vy)^2 + (cz + t * vy)^2 = r^2

	// text:
	// (e+td−c)·(e+td−c)−R2 = 0
	// (d·d)t^2 +2d·(e−c)t+(e−c)·(e−c)−R^2 = 0

	// d: the direction vector of the ray
	// e: point at which the ray starts
	// c: center point of the sphere

	Vec3 dvec(	ray.direction[0],
				ray.direction[1],
				ray.direction[2]);

	Vec3 evec(	ray.origin[0],
				ray.origin[1],
				ray.origin[2]);

	Vec3 cvec(	this->center[0],
				this->center[1],
				this->center[2]);

	// use the quadratic equation, since we have the form At^2 + Bt + C = 0.

	double a = dvec.dot(dvec);
	double b = dvec.scale(2).dot(evec.subtract(cvec));

	Vec3 eMinusCvec = evec.subtract(cvec);
	double c = eMinusCvec.dot(eMinusCvec) - (this->radius * this->radius);

	printf("a: %f, b: %f, c: %f \n", a, b, c);

	// discriminant: b^2 + 4ac
	double discriminant = (b * b) + (4 * a * c);

	// From text: If the discriminant is negative, its square root 
	// is imaginary and the line and sphere do not intersect.
	if (discriminant < 0) {
		
		//printf("No intersection with sphere - 1\n");
		return false;

	} else {
		// there is at least one intersection point
		double t1 = (-b + sqrt(discriminant)) / (2 * a);
		double t2 = (-b - sqrt(discriminant)) / (2 * a);

		double tmin = fminf(t1, t2);
		double tmax = fmaxf(t1, t2);

		double t = 0; // t is set to either tmin or tmax (or the function returns false)

		if (tmin >= 0 && tmin <= 1) {

			t = tmin;

		} else if (tmax >= 0 && tmax <= 1) {

			t = tmax;

		} else {

			// return false if neither interestion point is within [0, 1]
			//printf("No intersection with sphere. t values (%f, %f)\n", t1, t2);
			return false;

		}

		*depth = t;
		
		Vec3 posvec = dvec.scale(t).add(evec);
		*posX = posvec[0];
		*posY = posvec[1];
		*posZ = posvec[2];

		Vec3 normalvec = posvec.subtract(cvec).scale(1 / this->radius);
		*normalX = normalvec[0];
		*normalY = normalvec[1];
		*normalZ = normalvec[2];
	}

	//////////*********** END OF CODE TO CHANGE *******////////////

	printf("Intersection with sphere\n");
	return true;
}

///////////////////////////////////////////////////////////////////////////