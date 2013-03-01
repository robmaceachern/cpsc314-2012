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

#include <Raytracer.h>
#include <Operations.h>

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include <iostream>
using namespace std;

///////////////////////////////////////////////////////////////////////////

void	
Raytracer::raytraceScene(	const char *filename,
							const char *depth_filename,
							std::vector<PointLight> *lights, 
							std::vector<Plane> *planes,	
							std::vector<Sphere> *spheres,
							Camera *camera, int *resolution	)
{
	
	// pixel color buffer
	unsigned char *colorbuffer = new unsigned char[resolution[0]*resolution[1]*3];
	for(int i=0; i<3*resolution[0]*resolution[1]; i++) {
		colorbuffer[i] = (unsigned char)0;
	}
	// depth buffer
	double *depthbuffer = new double[resolution[0]*resolution[1]];
	for(int i=0; i<resolution[0]*resolution[1]; i++) {
		depthbuffer[i] = 1.0;
	}

	// the magical ray setup
	Vec3 f(	camera->center[0]-camera->position[0], 
			camera->center[1]-camera->position[1], 
			camera->center[2]-camera->position[2]);
	Vec3 up(camera->up[0],camera->up[1],camera->up[2]);
	// normalized up vector
	up.normalize();
	Vec3 right = f.cross(up);
	// normalized vector pointing to the side
	right.normalize();

	// step for each pixel
	double stepx = 2*tan(deg2rad(camera->fovy)*camera->aspect/2.0)*f.length() / resolution[0];
	double stepy = 2*tan(deg2rad(camera->fovy)/2.0)*f.length() / resolution[1];

	for(int y=0; y<resolution[1]; y++ ) {
		for(int x=0; x<resolution[0]; x++) {

			//////////////////////////////////////////////////////////////////////////////////
			//	the following lines implement a simple orthographic camera
			//	you have to modify the code for generating pespective rays
			//	i.e. all rays (theoretical) origin from the camera position and 
			//	propagate into the pixelLookAt
			//
			//	make sure that the ray's origin is not the camera center but 
			//	on the near plane. the direction of the ray should be a vector,
			//	that is computed as pointOnFarPlane-pointOnNearPlane, i.e. 
			//	adding it to the t parameter in the ray equation 
			//	point = pointOnNearPlane + t*rayDirection
			//	is always between 0 and 1. This will be the value in the depth
			//	buffer.
			//////////////////////////////////////////////////////////////////////////////////
			// generate the appropriate ray for this pixel
			
			//////////*********** START OF CODE TO CHANGE *******////////////

			// the direction of the ray should be a vector 
			// that is computed as pointOnFarPlane-pointOnNearPlan

			// point = pointOnNearPlane + t*rayDirection


			int alpha = x-resolution[0]/2;
			int beta  = y-resolution[1]/2;
			Vec3 pixelLookAt(	camera->center[0] + alpha*stepx*right[0] + beta*stepy*up[0],
								camera->center[1] + alpha*stepx*right[1] + beta*stepy*up[1],
								camera->center[2] + alpha*stepx*right[2] + beta*stepy*up[2]);			


			Vec3 pixelStart(	camera->position[0] + alpha*stepx*right[0] + beta*stepy*up[0],
								camera->position[1] + alpha*stepx*right[1] + beta*stepy*up[1],
								camera->position[2] + alpha*stepx*right[2] + beta*stepy*up[2]);

			Vec3 rayDirection(	pixelLookAt[0]-camera->position[0], 
								pixelLookAt[1]-camera->position[1], 
								pixelLookAt[2]-camera->position[2]);
			rayDirection.normalize();

			Ray pixelRay(	camera->position[0], 
							camera->position[1],
							camera->position[2],
							(camera->zFar-camera->zNear)*rayDirection[0],
							(camera->zFar-camera->zNear)*rayDirection[1],
							(camera->zFar-camera->zNear)*rayDirection[2]	);

			//////////*********** END OF CODE TO CHANGE *******////////////

			//////////////////////////////////////////////////////////////////////////////////
			
			int currentRayRecursion=0;
			double red=0.0, green=0.0, blue=0.0, depth=1.0;

			// calculate the pixel value by shooting the ray into the scene
			traceRay(	pixelRay, lights, planes, spheres, camera, &currentRayRecursion, 
						&red ,&green, &blue, &depth);

			// depth test
			if( (depth<depthbuffer[x+y*resolution[0]]) && (depth>=0.0)) {

				// red
				colorbuffer[3*(x+y*resolution[0])]=(unsigned char)(255.0*red);
				// green
				colorbuffer[3*(x+y*resolution[0])+1]=(unsigned char)(255.0*green);
				// blue
				colorbuffer[3*(x+y*resolution[0])+2]=(unsigned char)(255.0*blue);
	
				// depth 
				depthbuffer[x+y*resolution[0]] = depth;
			}
		}
	}
	writeImage(filename, colorbuffer, resolution);

	unsigned char *depthbuffer_uc = new unsigned char[resolution[0]*resolution[1]];
	for(int i=0; i<resolution[0]*resolution[1]; i++) {
		depthbuffer_uc[i] = (unsigned char)(255.0*depthbuffer[i]);
	}
	writeImage(depth_filename, depthbuffer_uc, resolution, true);

	delete [] depthbuffer_uc;
	delete [] depthbuffer;
	delete [] colorbuffer;
}

/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//	this function shoots a ray into the scene and calculates
//	the closest point of intersection
//	
//	if the ray hits a surface that is reflecting the rays have to be recursively
//	shot into the scene starting from that point into the proper direction
//	(using Snell's law of reflection). you need to implement the ray reflection 
//	and color blending between the reflected color and the local color (i.e.
//	direct lighting and indirect lighting). weight the reflection term by
//	1-material.reflect
//
/////////////////////////////////////////////////////////////////////////////////
void	
Raytracer::traceRay(	Ray pixelRay, 
						std::vector<PointLight> *lights, 
						std::vector<Plane> *planes,	
						std::vector<Sphere> *spheres, 
						Camera *camera,
						int *currRayRecursion,
						double *red , double *green, double *blue, double *depth )
{

	// did it intersect at all?
	bool bIntersect=false;
	// depth of the closest intersection
	double intersectDepth=*depth;
	// 3D position of the closest intersection
	double intersectPos[3];
	// normal at the point of closest intersection
	double intersectNormal[3];
	// the material of the object at the intersection point
	Material intersectMaterial;

	// calculate intersection of ray with all planes
	foreach(p, (*planes), vector<Plane>) {
		
		double iDepth, iPos[3], iNormal[3];

		if( p->intersect(	pixelRay, &iDepth, 
							&iPos[0], &iPos[1], &iPos[2],
							&iNormal[0], &iNormal[1], &iNormal[2]))
		{

			// depth test
			if(iDepth<intersectDepth) {
				bIntersect = true;
				intersectDepth = iDepth;
				intersectPos[0]=iPos[0]; intersectPos[1]=iPos[1]; intersectPos[2]=iPos[2];
				intersectNormal[0]=iNormal[0]; intersectNormal[1]=iNormal[1]; intersectNormal[2]=iNormal[2];
				intersectMaterial = p->material;

				if (*currRayRecursion > 1) {
					printf("recursive ray intersection with plane\n");
					printf("intersectNormal %f, %f, %f\n", intersectNormal[0], intersectNormal[1], intersectNormal[2]);
					printf("currRayRecursion %d \n", *currRayRecursion);
				}
			}
		}		
	}

	// calculate intersection of ray with all spheres
	foreach(s, (*spheres), vector<Sphere>) {
		
		double iDepth, iPos[3], iNormal[3];

		if( s->intersect(	pixelRay, &iDepth, 
							&iPos[0], &iPos[1], &iPos[2],
							&iNormal[0], &iNormal[1], &iNormal[2]))
		{

			// depth test
			if(iDepth<intersectDepth) {
				bIntersect = true;
				intersectDepth = iDepth;
				intersectPos[0]=iPos[0]; intersectPos[1]=iPos[1]; intersectPos[2]=iPos[2];
				intersectNormal[0]=iNormal[0]; intersectNormal[1]=iNormal[1]; intersectNormal[2]=iNormal[2];
				intersectMaterial = s->material;
			}
		}
	}

	if(bIntersect) {

		*depth = intersectDepth;

		shade(	intersectPos[0], intersectPos[1], intersectPos[2],
				intersectNormal[0], intersectNormal[1], intersectNormal[2],
				intersectMaterial, camera, 
				lights, planes,	spheres,
				red, green, blue);

		/////////////////////////////////////////////////////////////////////////////////////
		// recurse for reflections
		if( (*currRayRecursion<MAX_RAY_RECURSION) && (intersectMaterial.reflect>0.0) ) {				
			(*currRayRecursion) += 1;

			//////////*********** START OF CODE TO CHANGE *******////////////
			
			Vec3 iPoint(intersectPos[0],
						intersectPos[1],
						intersectPos[2]);

			Vec3 nvec(	intersectNormal[0],
						intersectNormal[1],
						intersectNormal[2]);
			//nvec.normalize();

			Vec3 origRay(	pixelRay.direction[0],
							pixelRay.direction[1],
							pixelRay.direction[2]);
			origRay.normalize();

			// http://en.wikipedia.org/wiki/Snell%27s_law#Vector_form
			double nDotRay = nvec.dot(origRay.scale(-1.0));

			if (nDotRay > 0) {

				double correctionFactorX = (nvec[0] > 0) ? 0.000001 : -0.000001;
				double correctionFactorY = (nvec[1] > 0) ? 0.000001 : -0.000001;
				double correctionFactorZ = (nvec[2] > 0) ? 0.000001 : -0.000001;

				Vec3 reflectVec = origRay.add(nvec.scale(2 * nDotRay));
				Ray secondaryRay(	iPoint[0] + correctionFactorX, 
									iPoint[1] + correctionFactorY,
									iPoint[2] + correctionFactorZ,
									reflectVec[0], reflectVec[1], reflectVec[2]);

				double redReflect = 0;
				double greenReflect = 0;
				double blueReflect = 0;
				double d = DBL_MAX;

				this->traceRay(secondaryRay, lights, planes, spheres, camera, currRayRecursion,
						 &redReflect, &greenReflect, &blueReflect, &d);

				*red = (*red * (1 - intersectMaterial.reflect)) + (redReflect * intersectMaterial.reflect);
				*green = (*green * (1 - intersectMaterial.reflect)) + (greenReflect * intersectMaterial.reflect);
				*blue = (*blue * (1 - intersectMaterial.reflect)) +  (blueReflect * intersectMaterial.reflect);
			}

			//////////*********** END OF CODE TO CHANGE *******////////////

		}
		/////////////////////////////////////////////////////////////////////////////////////

		// clip colors between 0.0 and 1.0
		*red = (*red > 1.0) ? 1.0 : *red;
		*red = (*red < 0.0) ? 0.0 : *red;
		*green = (*green > 1.0) ? 1.0 : *green;
		*green = (*green < 0.0) ? 0.0 : *green;
		*blue = (*blue > 1.0) ? 1.0 : *blue;
		*blue = (*blue < 0.0) ? 0.0 : *blue;

	}
}

///////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//	here you need to calculate the proper shading of a given point at posX, posY,
//	posZ and corresponding normal normalX, normalY and normalZ
//	
//	the material of the point is given as material, the camera as camera
//	and all objects in the scenes as planes and spheres
//	
//	lights is a list of all the light source, make sure to calculate the 
//	lighting for each one of them and add the color values, the material
//	emission part is only added once!
// 
//	write the color values in red, green and blue
// 
//	you also have to add the code that calculates if the point is in the 
//	shadow for each light source and weight the computed color value
//	bu 1-material.shadow
/////////////////////////////////////////////////////////////////////////////////
void	
Raytracer::shade(	double posX, double posY, double posZ,
					double normalX, double normalY, double normalZ,
					Material material, Camera *camera, 
					std::vector<PointLight> *lights, 
					std::vector<Plane>		*planes,	
					std::vector<Sphere>		*spheres,
					double *red, double *green, double *blue)
{

	// calculate emissive part here
	*red   = material.emission[0];
	*green = material.emission[1];
	*blue  = material.emission[2];

	Vec3 nvec(	normalX,
				normalY,
				normalZ);
	nvec.normalize();

	Vec3 vvec(	camera->position[0] - posX, 
				camera->position[1] - posY,
				camera->position[2] - posZ);
	vvec.normalize();

	foreach(light, (*lights), vector<PointLight>) {

		//////////*********** START OF CODE TO CHANGE *******////////////
		// add color value of this light source to color
		
		// include ambient, diffuse and specular
		// make sure to add the light attenuation
		// effect for the diffuse and specular term

		Vec3 lvec(	light->position[0] - posX,
					light->position[1] - posY,
					light->position[2] - posZ);

		double attenuation = 1 / (light->attenuation[2] * (lvec.length() * lvec.length()) + light->attenuation[1] * lvec.length() + light->attenuation[0]);
		lvec.normalize();

		Vec3 rvec = nvec.scale(nvec.dot(lvec)).scale(2).subtract(lvec);
		rvec.normalize();

		double lightComponents[3];
		int i;
		for (i = 0; i < 3; i++) {

			// https://piazza.com/class#winterterm22012/cpsc314/93
			double intensityDiffuse = (light->diffuse[i] * attenuation);
			double intensitySpecular = (light->specular[i] * attenuation);
			//printf("intensity: %f \n", intensity);

			double ambient = material.ambient[i] * light->ambient[i];

			double nDotL = nvec.dot(lvec);

			double diffuse = (nDotL > 0) ? material.diffuse[i] * intensityDiffuse * fmaxf(0, nvec.dot(lvec)) : 0;
			double specular = (nDotL > 0) ? material.specular[i] * intensitySpecular * pow(fmaxf(0, vvec.dot(rvec)), material.shininess) : 0;

			lightComponents[i] = fminf(ambient + diffuse + specular, 1);
		}
		
		/////////////////////////////////////////////////////////////////////////////////////
		// shoot a ray to every light source to see if the point is in shadow

		Vec3 toLightVec(light->position[0] - posX,
						light->position[1] - posY,
						light->position[2] - posZ);

		double correctionFactorX = (toLightVec[0] > 0) ? 0.000001 : -0.000001;
		double correctionFactorY = (toLightVec[1] > 0) ? 0.000001 : -0.000001;
		double correctionFactorZ = (toLightVec[2] > 0) ? 0.000001 : -0.000001;

		Ray toLight(posX + correctionFactorX,
					posY + correctionFactorY,
					posZ + correctionFactorZ,
					toLightVec[0], 
					toLightVec[1],
					toLightVec[2]);

		bool inShadow = false;

		foreach(s, (*spheres), vector<Sphere>) {
			
			double iDepth, iPos[3], iNormal[3];

			if( s->intersect(	toLight, &iDepth, 
								&iPos[0], &iPos[1], &iPos[2],
								&iNormal[0], &iNormal[1], &iNormal[2]))
			{

				// depth test
				// We only count it if the intersection occurs between the point
				// and the light (and not past the light).
				if(iDepth <= toLightVec.length()) {
					inShadow = true;
					break;
				}
			}
		}

		foreach(p, (*planes), vector<Plane>) {
			
			double iDepth, iPos[3], iNormal[3];

			if( p->intersect(	toLight, &iDepth, 
								&iPos[0], &iPos[1], &iPos[2],
								&iNormal[0], &iNormal[1], &iNormal[2]))
			{

				// depth test
				// We only count it if the intersection occurs between the point
				// and the light (and not past the light).
				if(iDepth <= toLightVec.length()) {
					inShadow = true;
					break;
				}
			}
		}

		//////////*********** END OF CODE TO CHANGE *******////////////

		if(material.shadow != 0.0) {

			//////////*********** START OF CODE TO CHANGE *******////////////
			
			if (inShadow == true) {

				//printf("Position of light that is shadowed: %f, %f, %f \n", light->position[0], light->position[1], light->position[2]);
				
				lightComponents[0] *= (1 - material.shadow);
				lightComponents[1] *= (1 - material.shadow);
				lightComponents[2] *= (1 - material.shadow);
				
				// if (light->position[1] > 6) {
				// 	*red = 1;
				// 	*green = 0;
				// 	*blue = 0;
				// } else {
				// 	*red = 0;
				// 	*green = 1;
				// 	*blue = 0;
				// }
				
				// doesn't work
				// *red += material.ambient[0] * light->ambient[0];
				// *green += material.ambient[1] * light->ambient[1];
				// *blue += material.ambient[2] * light->ambient[2];

			}
			
			//////////*********** END OF CODE TO CHANGE *******////////////
		}

		// end of shadows
		/////////////////////////////////////////////////////////////////////////////////////

		//////////*********** START OF CODE TO CHANGE *******////////////

		*red += lightComponents[0];
		*green += lightComponents[1];
		*blue += lightComponents[2];
		
		//////////*********** END OF CODE TO CHANGE *******////////////
	}	
}

///////////////////////////////////////////////////////////////////////////

bool	
Raytracer::writeImage(	const char *filename, 
						unsigned char *imageBuffer, 
						int *resolution, 
						bool greyscale) {

	FILE *fp = fopen(filename,"wb");
	if (!fp) {
		printf("Unable to open file '%s'\n",filename);
		return false;
	}

	const int maxVal=255;

	fprintf(fp, "P6 ");
	fprintf(fp, "%d %d ", resolution[0], resolution[1]);
	fprintf(fp, "%d", maxVal);
	putc(13,fp);

	int numChannels=3;
	if(greyscale)
		numChannels=1;

	for(int y=resolution[1]-1; y>=0; y-- ) {
		for(int x=0; x<resolution[0]; x++) {
			// red or depth
			putc(imageBuffer[numChannels*(x+y*resolution[0])],fp);
			if(!greyscale) {
				// green
				putc(imageBuffer[numChannels*(x+y*resolution[0])+1],fp);
				// blue
				putc(imageBuffer[numChannels*(x+y*resolution[0])+2],fp);
			} else {
				// depth
				putc(imageBuffer[numChannels*(x+y*resolution[0])],fp);
				// depth
				putc(imageBuffer[numChannels*(x+y*resolution[0])],fp);
			}
		}
	}
	fclose(fp);
	return true;
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
