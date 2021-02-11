#ifndef SPHERE_H
#define SPHERE_H

#include "Vec3f.h"

struct Sphere {
	Vec3f pos = Vec3f();
	Vec3f color = Vec3f(1);
	Vec3f emissionColor = Vec3f(0);
	float radius = 1;
	float refl = 0;
	float refract = 0;
	float IOR = 1;
	int type = 0; //0 diffuse, 1 pure specular, 2 glass
	//bool light = false;

	Sphere() : pos(0), radius(1), color(1) {}
	Sphere(float num) : pos(num), radius(num), color(1) {}
	Sphere(float xPos, float yPos, float zPos, float r) : pos(xPos, yPos, zPos), radius(r) {}
	//Sphere(float xPos, float yPos, float zPos, bool isLight) : pos(xPos, yPos, zPos), light(isLight) {}
	//Sphere(float xPos, float yPos, float zPos, float r, bool isLight) 
	//	: pos(xPos, yPos, zPos), radius(r), light(isLight) {}
	//Sphere(float xPos, float yPos, float zPos, float r, bool isLight, float eR, float eG, float eB) 
	//	: pos(xPos, yPos, zPos), radius(r), light(isLight), emissionColor(eR, eG, eB) {}
	Sphere(float xPos, float yPos, float zPos, float cR, float cG, float cB, float r)
		: pos(xPos, yPos, zPos), color(cR, cG, cB), radius(r) {}
	Sphere(float xPos, float yPos, float zPos, float cR, float cG, float cB, float r,
		float inRefl, float inRefract, float inIOR)
		: pos(xPos, yPos, zPos), color(cR, cG, cB), radius(r), refl(inRefl), refract(inRefract), IOR(inIOR) {}
	Sphere(Vec3f inPos, Vec3f inColor, Vec3f inEmission, float r, int inType) 
		: pos(inPos), color(inColor), emissionColor(inEmission), radius(r), type(inType) {}


	bool intersect(Vec3f rayOrigin, Vec3f rayDirection, float& p0, float& p1) {
		bool useGeometric = false;

		//we first have to check if the object is even in front of us
		//because if not, then it's not going to be intersectable

		if (useGeometric) {
			//geometric way
			Vec3f scene = pos - rayOrigin;
			float rayAngle = scene.dot(rayDirection);
			if (rayAngle < 0) return false;
			float check2 = scene.dot(scene) - (rayAngle * rayAngle);
			if (check2 > (radius * radius)) return false; //in this pixel, the sphere isn't visible 
			return true;
		}
		else {
			//analytic way
			float a = rayDirection.dot(rayDirection); //should be 1
			Vec3f D2 = rayDirection * 2;
			Vec3f PR = rayOrigin - pos; //vector from the sphere's center to the rayOrigin
			float b = D2.dot(PR);
			float c = PR.dot(PR) - (radius * radius);
			float discriminant = (b * b) - (4 * a * c);
			if (discriminant < 0) return false; // the ray didn't hit the sphere
			if (discriminant == 0) { // then it has only one hit
				//std::cout << "discriminant equals 0" << std::endl;
				p0 = (-1 * b) / (2 * a);
				p1 = p0;
			}

			if (discriminant > 0) { // then it has two hits
				//-b + sqrt(b^2 - 4ac)/2a AND -b - sqrt(b^2 - 4ac)/2a
				//std::cout << "discriminant > 0" << std::endl;
				int sign = 1;
				if (b < 0) sign = -1;
				float quadratic = (-0.5) * ((double)b + (double)sign * sqrt(discriminant));
				p0 = quadratic / a;
				p1 = c / quadratic;

			}

			if (p0 > p1) std::swap(p0, p1);
			if (p0 < 0) {
				if (p1 < 0) return false;
				p0 = p1;
			}
			return true;



		}
	}
};



#endif

