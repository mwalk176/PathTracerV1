#ifndef SPHERE_H
#define SPHERE_H

#include "Vec3f.h"
#include "Object.h"

class Sphere : public Object {
public:
	//Vec3f pos = Vec3f();
	//Vec3f color = Vec3f(1);
	//Vec3f emissionColor = Vec3f(0);
	float radius = 1;
	//float refl = 0;
	//float refract = 0;
	//float IOR = 1.5;
	//float gammaBoost = 1.0;
	//int type = 0; //0 diffuse, 1 pure specular, 2 glass
	//bool light = false;

	Sphere() { pos = 0; radius = 1;  color = 1; }
	Sphere(float num) { pos = num; radius = num; color = num; }
	//Sphere(float xPos, float yPos, float zPos, float r) : pos(xPos, yPos, zPos), radius(r) {}
	//Sphere(float xPos, float yPos, float zPos, bool isLight) : pos(xPos, yPos, zPos), light(isLight) {}
	//Sphere(float xPos, float yPos, float zPos, float r, bool isLight) 
	//	: pos(xPos, yPos, zPos), radius(r), light(isLight) {}
	//Sphere(float xPos, float yPos, float zPos, float r, bool isLight, float eR, float eG, float eB) 
	//	: pos(xPos, yPos, zPos), radius(r), light(isLight), emissionColor(eR, eG, eB) {}
	Sphere(float xPos, float yPos, float zPos, float cR, float cG, float cB, float r) {
		pos = Vec3f(xPos, yPos, zPos);
		color = Vec3f(cR, cG, cB);
		radius = r;
	}
	Sphere(float xPos, float yPos, float zPos, float cR, float cG, float cB, float r,
		float inRefl, float inRefract, float inIOR) {
		pos = Vec3f(xPos, yPos, zPos);
		color = Vec3f(cR, cG, cB);
		radius = r;
		refl = inRefl;
		refract = inRefract;
		IOR = inIOR;
	}
	Sphere(Vec3f inPos, Vec3f inColor, Vec3f inEmission, float r, int inType) {
		pos = inPos; 
		color = inColor;
		emissionColor = inEmission; 
		radius = r; 
		type = inType;
	}
	Sphere(Vec3f inPos, Vec3f inColor, Vec3f inEmission, float r, int inType, float inGamma) {
		pos = inPos;
		color = inColor;
		emissionColor = inEmission;
		radius = r;
		type = inType;
		gammaBoost = inGamma;
	}


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

	Vec3f getNewDirectionTowardsLight(Vec3f shadowRay, Vec3f alignedNormal, Vec3f normalFromLight, float& angleToObject, Vec3f intersectionPoint) {
		//build a coordinate space in the hemisphere of the shadowray light
		Vec3f se1 = Vec3f(0);
		if (fabs(alignedNormal.x) > fabs(alignedNormal.y)) {
			se1 = Vec3f(shadowRay.z, 0, -shadowRay.x) / sqrt(shadowRay.x * shadowRay.x + shadowRay.z * shadowRay.z);
		}
		else {
			se1 = Vec3f(0, -shadowRay.z, shadowRay.y) / sqrt(shadowRay.y * shadowRay.y + shadowRay.z * shadowRay.z);
		}
		se1.normalize();
		Vec3f se2 = shadowRay.cross(se1);
		se2.normalize();

		//calculate a random direction towards light
		angleToObject = sqrt(1 - radius * radius / normalFromLight.dot(normalFromLight));
		float randX = (float)rand() / RAND_MAX; //get us a random point
		float randomAngle2 = M_PI * 2 * ((float)rand() / RAND_MAX);
		float angleCos = 1 - randX + randX * angleToObject;
		float angleSin = sqrt(1 - angleCos * angleCos);
		Vec3f newShadowRay = se1 * cos(randomAngle2) * angleSin + se2 * sin(randomAngle2) * angleSin + shadowRay * angleCos;
		newShadowRay.normalize();

		return newShadowRay;
	}

	Vec3f computeNormal(Vec3f intersectionPoint) {
		return intersectionPoint - pos;
	}
	std::string toString() {
		return "SPHERE";
	}
};



#endif

