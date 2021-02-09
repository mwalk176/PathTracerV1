
//**************************************//
//										//
//			Simple PathTracer V1		//
//			A personal Project			//
//			By Matthew Walker			//
//				12/22/2020				//
//										//
//										//
//**************************************//


#define _USE_MATH_DEFINES
#define MAX_DEPTH 3
#define GAMMA 2.2
#define FOV 50

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>

#include <time.h>

#include "Vec3f.h"
#include "Camera.h"
#include "Sphere.h"
#include <future>


Vec3f tracePixelColor(Vec3f& rayOrigin, Vec3f& rayDirection,
	std::vector<Sphere>& scene, std::vector<Sphere>& lights, int& depth);
Vec3f calculateRayDirection(int columns, int rows, int x, int y, float sX, float sY, Camera camera);
std::ofstream writeToPPMFile(Vec3f** image, int width, int height, int maximumColorValue, int frameNum);
int getClosestObject(Vec3f rayOrigin, Vec3f rayDirection, std::vector<Sphere>& scene, float& closestPoint);
void renderRow(Vec3f** image, int y, int rows, int columns, int maxSamples, Camera camera,
	std::vector<Sphere> scene, std::vector<Sphere> lights);



int main(int argc, char* argv[]) {

	bool isPerspective = true;
	srand(time(NULL));

	//error checking
	if (argc < 2) {
		std::cout << "error: please specify a width, height, start frame, end frame, and max samples" << std::endl;
		return 0;

	}


	//Declare image variables
	const int columns = atoi(argv[1]);
	const int rows = atoi(argv[2]);
	const int startFrame = atoi(argv[3]);
	const int endFrame = atoi(argv[4]);
	const int maxSamples = atoi(argv[5]);
	int maximumColorValue = 255;

	//create a 2d grid of pixel objects
	//we initialize the ints in the pixels to 0 as a precaution
	Vec3f** image = new Vec3f * [rows];
	for (int i = 0; i < rows; i++) {
		image[i] = new Vec3f[columns];
	}

	for (int frameNum = startFrame; frameNum <= endFrame; frameNum++) {
		float currentTime = (float)frameNum / 24.0;




		/*THIS IS THE MAIN SCENE, BUILD THE SCENE YOU WANT BY MODIFYING THESE VALUES*/

		Camera camera;
		camera.pos.setEach(0, 0, -15 + ((frameNum - 1.0) / 24.0) * 2);
		//camera.pos.setEach(0, 0, -15);

		std::vector<Sphere> lights;

		/*
		lights.push_back(Sphere(10, 5, 0, true));
		lights.push_back(Sphere(-10, 5, 0, true));
		lights.push_back(Sphere(5, 5, -5, true));
		lights.push_back(Sphere(0, 5000, 0, true));
		*/
		//lights.push_back(Sphere(10, 5, 0, 1.0, true));
		lights.push_back(Sphere(0, 0, 0, 10000, true, 1, 1, 1));

		std::vector<Sphere> scene;

		//scene.push_back(Sphere(0, 0, 0, 0.18, 0.18, 0.18, 3));
		//scene.push_back(Sphere(0, -10004, 0, 0.9, 0.9, 0.9, 10000));
		
		scene.push_back(Sphere(0, 0, 0, 0.7, 1, 1, 3));

		scene.push_back(Sphere(4, -2.75, -2, 0.2, 0.9, 0.5, 1));
		scene.push_back(Sphere(-5, -1.5, 1.5, 1, 0.4, 0.4, 2));
		scene.push_back(Sphere(-3, -2.75, -2, 1, 1, 0, 1));


		scene.push_back(Sphere(0, -10004, 0, 0.9, 0.9, 0.9, 10000));
		//scene.push_back(Sphere(0, 10010, 0, 0.9, 0.9, 0.9, 10000));
		
		/*END MAIN SCENE*/

		float aspectRatio = (float)columns / (float)rows;

		unsigned int numCores = std::thread::hardware_concurrency();
		std::vector<std::thread> threadList;
		unsigned int coreCount = 0;

		//For each pixel in the image
		for (int y = 0; y < rows; y++) {
			std::cout << "y: " << y << std::endl;
			
			//std::thread t = std::thread(renderRow, std::ref(image), y,
			//	rows, columns, maxSamples, camera, std::ref(scene), std::ref(lights));
			//t.join();

			threadList.push_back(std::thread(renderRow, std::ref(image), y,
				rows, columns, maxSamples, camera, std::ref(scene), std::ref(lights)));
			
			//threadList.push_back(t);

			coreCount++;
			if (coreCount % numCores == 0) {
				for (int i = 0; i < threadList.size(); i++) {
					threadList.at(i).join();
				}
				threadList.clear();
			}
			


			/*
			for (int x = 0; x < columns; x++) {

				image[y][x].setAll(0); //SET BACKGROUND COLOR
				

				int timesSampled = 0;
				for (int subY = 0; subY < 2; subY++) { //calculate 2x2 subpixel grid
					for (int subX = 0; subX < 2; subX++) {
						for (int samples = 0; samples < maxSamples; samples++) {
							float rX = ((float)rand() / RAND_MAX) / 2.0;
							float rY = ((float)rand() / RAND_MAX) / 2.0;
							if (subX == 1) rX += 0.5;
							if (subY == 1) rY += 0.5;
							//if 0, 0-0.50
							//if 1, 0.51-0.99

							float closestPoint = INFINITY;

							for (unsigned int i = 0; i < scene.size(); i++) {
								Vec3f rayOrigin(0);
								Vec3f rayDirection(0);

								rayOrigin.setEach(camera.pos.x, camera.pos.y, camera.pos.z);
								rayDirection = calculateRayDirection(columns, rows, x, y, rX, rY, camera);

								int depth = 0;

								//call trace function
								//std::future<Vec3f> col = std::async(std::launch::async, tracePixelColor, 
								//	std::ref(rayOrigin), std::ref(rayDirection), std::ref(scene), 
								//	std::ref(lights), std::ref(depth));
								//image[y][x] = image[y][x] + col.get();
								image[y][x] = image[y][x] + tracePixelColor(rayOrigin, rayDirection, scene, 
									lights, depth);
								//image[y][x].clamp();
								timesSampled++;
							}
						}
					}
				}
				image[y][x] = (image[y][x] / timesSampled);
				image[y][x].clamp();
			}*/
		}
		//Write image
		writeToPPMFile(image, rows, columns, maximumColorValue, frameNum);
	}



	return 0;
}


Vec3f tracePixelColor(Vec3f& rayOrigin, Vec3f& rayDirection,
	std::vector<Sphere>& scene, std::vector<Sphere>& lights, int& depth) {

	if (depth > MAX_DEPTH) return Vec3f(0);
	depth += 1;
	
	float closestPoint = INFINITY;

	//check for intersection with object
	int closestObject = getClosestObject(rayOrigin, rayDirection, scene, closestPoint);
	if (closestObject == -1) {
		float closestPoint2 = INFINITY;
		//check for intersection with light
		int closestLight = getClosestObject(rayOrigin, rayDirection, lights, closestPoint2);
		if (closestLight == -1) return Vec3f(0); //no object OR light was hit
		Sphere light = lights.at(closestLight);
		return Vec3f(light.emissionColor.x, light.emissionColor.y, light.emissionColor.z); //color pixel with light
	}
	Sphere currentObject = scene.at(closestObject);
	Vec3f objectColor = currentObject.color;

	//calculate spot where it intersected and the normal
	Vec3f intersectionPoint = rayOrigin + rayDirection * closestPoint;
	Vec3f normal = intersectionPoint - currentObject.pos;
	normal.normalize();

	float epsilon = 8e-6 * (currentObject.radius + 1.0);
	Vec3f normalOrigin = intersectionPoint + normal * epsilon;

	Vec3f lightValue = Vec3f(0);

	//diffuse reflections
	//1. calculate a random ray direction in the hemisphere around the normal
	float randomAngle = M_PI * 2 * ((float)rand() / RAND_MAX);
	float distanceModifier = (float)rand() / RAND_MAX;

	//get a set of basis vectors to have a hemisphere around the normal of the object
	Vec3f e1 = Vec3f(0);
	if (fabs(normal.x) > fabs(normal.y)) { //make sure the normal is the "up" vector in the hemisphere
		e1 = Vec3f(normal.z, 0, -normal.x) / sqrt(normal.x * normal.x + normal.z * normal.z);
	}
	else {
		e1 = Vec3f(0, -normal.z, normal.y) / sqrt(normal.y * normal.y + normal.z * normal.z);
	}
	e1.normalize();
	Vec3f e2 = normal.cross(e1);
	e2.normalize();

	Vec3f randomReflection = (e1 * sin(randomAngle) * distanceModifier + 
							  e2 * cos(randomAngle) * distanceModifier + 
							  normal * distanceModifier);
	randomReflection.normalize();

	Vec3f indirectDiffuse = tracePixelColor(normalOrigin, randomReflection, scene, lights, depth);


	//Calculate color/lit/unlit (no material)
	for (unsigned int k = 0; k < lights.size(); k++) {
		//shoot another ray towards the face of the light
		Vec3f shadowRay = lights.at(k).pos - intersectionPoint;
		Vec3f normalFromLight = intersectionPoint - lights.at(k).pos;
		float lightDist = shadowRay.calculateMagnitude();
		shadowRay.normalize();

		//build a coordinate space in the hemisphere of the shadowray light
		Vec3f se1 = Vec3f(0);
		if (fabs(normal.x) > fabs(normal.y)) {
			se1 = Vec3f(shadowRay.z, 0, -shadowRay.x) / sqrt(shadowRay.x * shadowRay.x + shadowRay.z * shadowRay.z);
		}
		else {
			se1 = Vec3f(0, -shadowRay.z, shadowRay.y) / sqrt(shadowRay.y * shadowRay.y + shadowRay.z * shadowRay.z);
		}
		se1.normalize();
		Vec3f se2 = shadowRay.cross(se1);
		se2.normalize();
		
		//calculate a random direction towards light
		float angleToSphere = sqrt((1 - lights.at(k).radius) * (lights.at(k).radius)) / normalFromLight.dot(normalFromLight);
		float randX = (float)rand() / RAND_MAX; //get us a random point
		float randomAngle2 = M_PI * 2 * ((float)rand() / RAND_MAX);
		float angleCos = 1 - randX + (randX * angleToSphere);
		float angleSin = sqrt(1 - angleCos * angleCos);
		Vec3f newShadowRay = se1 * cos(randomAngle2) * angleSin + se2 * sin(randomAngle2) * angleSin + shadowRay * angleCos;
		newShadowRay.normalize();

		//newShadowRay = shadowRay;

		float s0 = 0;
		float s1 = 0;
		bool inShadow = false;
		bool insideLight = false;
		if (lightDist < lights.at(k).radius) { //we're inside the light
			//std::cout << "INSIDE LIGHT" << std::endl;
			lightValue.setEach(lightValue.x + lights.at(k).emissionColor.x, lightValue.y + lights.at(k).emissionColor.y, lightValue.z + lights.at(k).emissionColor.z);
		}


		//check to see if another object is inbetween the light
		for (unsigned int j = 0; j < scene.size(); j++) {
			if (scene.at(j).intersect(normalOrigin, newShadowRay, s0, s1) && s0 < lightDist) {
				inShadow = true;
				break;
			}
		}
		if (!inShadow) {
			lightValue = lightValue + normal.dot(newShadowRay);
			//if (lightValue.calculateMagnitude() < 0.02) lightValue = (float)0.02;
		}
		
	}
	//lightValue = (lightValue / lights.size());
	//lightValue.clamp();
	//objectColor = objectColor * lightValue;
	Vec3f totalLight = (indirectDiffuse + lightValue);
	totalLight.clamp();
	objectColor = objectColor * totalLight;
	//objectColor = objectColor * lightValue;

	return objectColor;

}

Vec3f calculateRayDirection(int columns, int rows, int x, int y, float sX, float sY, Camera camera) {
	//so I have to convert the pixel grid into essentially an object in 3d space
	//the 0.5 places the ray directly in the center of the pixel
	float NDCx = (x + sX) / columns;
	float NDCy = (y + sY) / rows;

	float aspectRatio = (float)columns / (float)rows;

	//these are the x and y coordinates of a grid that is 1 unit in front of the camera origin
	float xPixel = (2 * NDCx - 1) * aspectRatio * tan((FOV * M_PI / 180) / 2);
	float yPixel = (1 - 2 * NDCy) * tan((FOV * M_PI / 180) / 2);
	Vec3f pixelLocation(xPixel, yPixel, camera.pos.z + 1);

	//so now we set the ray origin and ray direction
	Vec3f rayOrigin(camera.pos.x, camera.pos.y, camera.pos.z);
	Vec3f rayDirection = pixelLocation - rayOrigin;


	rayDirection.normalize();
	return rayDirection;
}

int getClosestObject(Vec3f rayOrigin, Vec3f rayDirection, std::vector<Sphere>& scene, float& closestPoint) {

	int closestObject = -1;
	float p0 = INFINITY;
	float p1 = INFINITY;
	//float closestPoint = INFINITY;

	for (int i = 0; i < scene.size(); i++) {
		if (scene.at(i).intersect(rayOrigin, rayDirection, p0, p1)) {
			if (p0 < closestPoint) {
				closestPoint = p0;
				closestObject = i;
			}
			else {
				continue; 
			}
			
		}
	}

	return closestObject;

}

void renderRow(Vec3f** image, int y, int rows, int columns, int maxSamples, Camera camera,
	std::vector<Sphere> scene, std::vector<Sphere> lights) {
	srand(std::hash<std::thread::id>{}(std::this_thread::get_id()));
	for (int x = 0; x < columns; x++) {

		image[y][x].setAll(0); //SET BACKGROUND COLOR
		//srand(y * columns + x);
		

		int timesSampled = 0;
		for (int subY = 0; subY < 2; subY++) { //calculate 2x2 subpixel grid
			for (int subX = 0; subX < 2; subX++) {
				for (int samples = 0; samples < maxSamples; samples++) {
					float rX = ((float)rand() / RAND_MAX) / 2.0;
					float rY = ((float)rand() / RAND_MAX) / 2.0;
					if (subX == 1) rX += 0.5;
					if (subY == 1) rY += 0.5;
					//if 0, 0-0.50
					//if 1, 0.51-0.99

					float closestPoint = INFINITY;

					for (unsigned int i = 0; i < scene.size(); i++) {
						Vec3f rayOrigin(0);
						Vec3f rayDirection(0);

						rayOrigin.setEach(camera.pos.x, camera.pos.y, camera.pos.z);
						rayDirection = calculateRayDirection(columns, rows, x, y, rX, rY, camera);

						int depth = 0;


						image[y][x] = image[y][x] + tracePixelColor(rayOrigin, rayDirection, scene,
							lights, depth);
						//image[y][x].clamp();
						timesSampled++;
					}
				}
			}
		}
		image[y][x] = (image[y][x] / timesSampled);
		image[y][x].clamp();
	}
}

std::ofstream writeToPPMFile(Vec3f** image, int rows, int columns, int maximumColorValue, int frameNum) {

	//Start creating the file
	std::ostringstream oss;
	char s[10];
	sprintf_s(s, "%04d", frameNum);
	//std::string path = "C:/Users/mwalk/OneDrive - BYU/Visual Studio/Programs/source/repos/PathTracerV1";
	//std::string path = "C:/Users/mwalk/Desktop";
	oss << "./output/image." << s << ".ppm";
	std::string outputLocation = oss.str();
	std::ofstream file(outputLocation, std::ios::binary);
	//std::ofstream file; 
	//file.open("out.ppm", std::ios::binary);

	file << "P6\n";
	file << columns << " " << rows << "\n";
	file << maximumColorValue << "\n";



	//for each pixel in the image
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {

			//each pixel has 3 values, R, G, and B
			//we have to convert the ints into chars because
			//otherwise the image isn't correct

			file <<
				(unsigned char)(image[i][j].x * 255) <<
				(unsigned char)(image[i][j].y * 255) <<
				(unsigned char)(image[i][j].z * 255);

		}
	}

	std::cout << "wrote output file to " << outputLocation << std::endl;




	file.close();

	return file;
}


