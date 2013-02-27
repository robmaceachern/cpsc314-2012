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

	// made use of this tutorial
	// http://blogs.warwick.ac.uk/nickforrington/entry/raytracing_intersection_with/
	// return false;
	// Vec3 normalvec(	this->params[0],
	// 				this->params[1],
	// 				this->params[2]);

	// Vec3 dvec(	ray.direction[0],
	// 			ray.direction[1],
	// 			ray.direction[2]);

	// Vec3 evec(	ray.origin[0],
	// 			ray.origin[1],
	// 			ray.origin[2]);

	// // We need to find a point on the plane
	// Vec3 planePoint(0,0,0);

	// if (this->params[0] != 0) {

	// 	Vec3 point(this->params[3] / this->params[0],
	// 				0,
	// 				0);
	// 	planePoint = point;

	// } else if (this->params[1] != 0) {

	// 	Vec3 point(this->params[3] / this->params[1],
	// 				0,
	// 				0);
	// 	planePoint = point;

	// } else if (this->params[2] != 0) {

	// 	Vec3 point(this->params[3] / this->params[2],
	// 				0,
	// 				0);
	// 	planePoint = point;

	// } else {

	// 	printf("Something is fucked!!! Weird plane equation parameters! \n");

	// }

	// double t = normalvec.dot(planePoint.subtract(evec)) / (evec.dot(dvec));

	// I called this the evec in sphere intersection, but I wanted to keep 
	// things consistent with the tutorial I was using.
	Vec3 ovec(	ray.origin[0],
				ray.origin[1],
				ray.origin[2]);

	Vec3 nvec(	this->params[0],
				this->params[1],
				this->params[2]);

	Vec3 dvec(	ray.direction[0],
				ray.direction[1],
				ray.direction[2]);

	double d = this->params[3];
	double t = -1;
	double denom = dvec.dot(nvec);

	if (denom > 0 || denom < 0) {

		t = (-d - (ovec.dot(nvec))) / dvec.dot(nvec);
		
	}

	if (t < 0 || t > 1) {

		return false;

	} else {

		*depth = t;
		*posX = (dvec[0] * t) + ovec[0];
		*posY =	(dvec[1] * t) + ovec[1];
		*posZ = (dvec[2] * t) + ovec[2];
		*normalX = nvec[0];
		*normalY = nvec[1];
		*normalZ = nvec[2];

	}

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

	// discriminant: b^2 - 4ac
	double discriminant = (b * b) - (4 * a * c);

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
		
		// position: (e + td)
		Vec3 posvec = dvec.scale(t).add(evec);
		*posX = posvec[0];
		*posY = posvec[1];
		*posZ = posvec[2];

		// normal: 2(p - c)
		Vec3 normalvec = posvec.subtract(cvec).scale(2);
		*normalX = normalvec[0];
		*normalY = normalvec[1];
		*normalZ = normalvec[2];
	}

	//////////*********** END OF CODE TO CHANGE *******////////////
	//printf("Sphere intersection found (%f, %f, %f) \n", *posX, *posY, *posZ);
	return true;
}

///////////////////////////////////////////////////////////////////////////
