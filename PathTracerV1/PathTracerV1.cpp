
//**************************************//
//										//
//			Simple PathTracer V1		//
//			A personal Project			//
//			By Matthew Walker			//
//				12/22/2020				//
//										//
//		inspired by projects such as	//
//		scratchapixel.com and smallpt	//
//		thanks for the guidance!		//
//										//
//**************************************//


#define _USE_MATH_DEFINES
#define MAX_DEPTH 10
#define GAMMA 2.2
#define FOV 50
#define USE_MULTITHREADING 1


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
#include "Triangle.h"
#include "Rectangle.h"
#include <future>


Vec3f tracePixelColor(Vec3f& rayOrigin, Vec3f& rayDirection,
	std::vector<Object*>& scene, int& depth, bool calculateEmission);
Vec3f calculateRayDirection(int columns, int rows, int x, int y, float sX, float sY, Camera camera);
std::ofstream writeToPPMFile(Vec3f** image, int width, int height, int maximumColorValue, int frameNum);
int getClosestObject(Vec3f rayOrigin, Vec3f rayDirection, std::vector<Object*>& scene, float& closestPoint);
void renderRow(Vec3f** image, int y, int rows, int columns, int maxSamples, Camera camera,
	std::vector<Object*> scene, bool averageValues, int currentSamples);



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
	const int iterateSamples = atoi(argv[6]);
	int maximumColorValue = 255;

	//create a 2d grid of pixel objects
	//we initialize the ints in the pixels to 0 as a precaution
	Vec3f** image = new Vec3f * [rows];
	for (int i = 0; i < rows; i++) {
		image[i] = new Vec3f[columns];
	}
	Vec3f** image2 = new Vec3f * [rows];
	for (int i = 0; i < rows; i++) {
		image2[i] = new Vec3f[columns];
	}


	std::cout << "Starting Render..." << std::endl;

	if (iterateSamples == 1) {

		Camera camera;

		std::vector<Object*> scene;

		camera.pos.setEach(0, 0, -5);

		//complex scene
		//cube
		scene.push_back(new Rectangle(Vec3f(0.0, -0.820135, -0.278169), Vec3f(0.0, -0.0111184, -0.865954), Vec3f(0.707107, 0.404509, -0.293893), Vec3f(0.707107, -0.404509, 0.293893),
			Vec3f(0.2, 0.8, 1), Vec3f(0)));
		scene.push_back(new Rectangle(Vec3f(0.0, -0.820135, -0.278169), Vec3f(-0.707107, -0.404509, 0.293893), Vec3f(-0.707107, 0.404509, -0.293893), Vec3f(0.0, -0.0111184, -0.865954),
			Vec3f(0.2, 0.8, 1), Vec3f(0)));
		scene.push_back(new Rectangle(Vec3f(0.0, -0.0111184, -0.865954), Vec3f(-0.707107, 0.404509, -0.293893), Vec3f(0.0, 0.820135, 0.278169), Vec3f(0.707107, 0.404509, -0.293893),
			Vec3f(0.2, 0.8, 1), Vec3f(0)));


		scene.push_back(new Sphere(Vec3f(0), Vec3f(1), Vec3f(0.5), 10000, 0));
		scene.push_back(new Sphere(Vec3f(2, -0.95, 0), Vec3f(1), Vec3f(3), 0.5, 0));
		scene.push_back(new Sphere(Vec3f(-2, -0.95, 0), Vec3f(1), Vec3f(3), 0.5, 0));
		scene.push_back(new Sphere(Vec3f(0, -1.05, -1), Vec3f(1), Vec3f(0), 0.4, 2));
		scene.push_back(new Sphere(Vec3f(-1, -1.15, 1), Vec3f(1), Vec3f(0), 0.3, 1));
		scene.push_back(new Sphere(Vec3f(1, -1.15, 1), Vec3f(1), Vec3f(0), 0.3, 1));
		scene.push_back(new Triangle(Vec3f(0, 2, 5), Vec3f(-1, 3, 5), Vec3f(1, 3, 5), Vec3f(1), Vec3f(1)));

		scene.push_back(new Sphere(Vec3f(0, -10001.5, 0), Vec3f(0.9, 0.9, 0.9), Vec3f(0), 10000, 0));

		

		std::cout << "Iteration Mode" << std::endl;

		float aspectRatio = (float)columns / (float)rows;

		unsigned int numCores = std::thread::hardware_concurrency();
		std::vector<std::thread> threadList;
		unsigned int coreCount = 0;

		time_t timer;
		double startTime = time(&timer);
		double deltaTime = -1;
		bool calculateTime = false;
		double averageCycleTime = 0;
		double totalTime = 0;

		int s = 0;
		int checkpointThreshold = 5;


		std::ifstream file("./output/temp.txt");
		if (file.is_open()) {
			std::cout << "Temp file exists, pulling values from it" << std::endl;
			file >> s;
			for (int y = 0; y < rows; y++) {
				for (int x = 0; x < columns; x++) {
					float r, g, b;
					file >> r;
					file >> g;
					file >> b;
					image[y][x] = Vec3f(r, g, b);
				}
			}
			file.close();
		}

		while (s < maxSamples) {

			if (calculateTime) {
				deltaTime = time(&timer) - startTime;
				startTime = time(&timer);
				averageCycleTime = averageCycleTime == 0 ? (averageCycleTime + deltaTime) : (averageCycleTime + deltaTime) / 2.0;
				totalTime = totalTime + deltaTime;
				calculateTime = false;
				std::cout << "Cycle complete, current s: " << s << std::endl;
				std::cout << "  Elapsed time: " << (int)(totalTime / 60) << " minutes, " << (int)totalTime % 60 << " seconds" << std::endl;
				std::cout << "  Previous cycle took " << deltaTime << " seconds to complete" << std::endl;
				std::cout << "  Average cycle time: " << averageCycleTime << " seconds" << std::endl;

			}


			//For each pixel in the image
			for (int y = 0; y < rows; y++) {
				if(y % 100 == 0) std::cout << "y: " << y << std::endl;


				//create thread for the row
				threadList.push_back(std::thread(renderRow, std::ref(image), y,
					rows, columns, maxSamples, camera, std::ref(scene), true, s));

				//if there's max threads for cpu cores, wait until they're done before adding more
				coreCount++;
				if (coreCount % numCores == 0) {
					for (int i = 0; i < threadList.size(); i++) {
						threadList.at(i).join();
					}
					threadList.clear();

				}
			}
			calculateTime = true;
			s++; //multiply this by 4 to get actual amount (because of the 2x2 subpixel sampling)

			if (s % checkpointThreshold == 0) { //reached a checkpoint, save to file
				std::cout << "reached checkpoint" << std::endl;
				std::ofstream file("./output/temp.txt");
				std::cout << "writing to temp file, DO NOT EXIT" << std::endl;
				file << s << " ";

				int numSamples = s * 4;

				for (int y = 0; y < rows; y++) {
					for (int x = 0; x < columns; x++) {
						float r, g, b;
						r = image[y][x].x;
						g = image[y][x].y;
						b = image[y][x].z;
						file << r << " ";
						file << g << " ";
						file << b << " ";

					}
				}
				file.close();



				for (int y = 0; y < rows; y++) {
					for (int x = 0; x < columns; x++) {
						image2[y][x] = image[y][x] / numSamples;
						image2[y][x].clamp();
					}
				}

				std::cout << "Temp file written, writing to PPM File now" << std::endl;
				writeToPPMFile(image2, rows, columns, maximumColorValue, 1);
			}

		}



		return 0;
	}



	for (int frameNum = startFrame; frameNum <= endFrame; frameNum++) {
		float currentTime = (float)frameNum / 24.0;

		std::cout << "Rendering Frame " << frameNum << " of " << endFrame << std::endl;

		Camera camera;

		std::vector<Object*> scene;

		camera.pos.setEach(0, 0, -5 + ((frameNum - 1) % 12));

		//complex scene
		//cube
		scene.push_back(new Rectangle(Vec3f(0.0, -0.820135, -0.278169), Vec3f(0.0, -0.0111184, -0.865954), Vec3f(0.707107,	0.404509, -0.293893), Vec3f(0.707107, -0.404509, 0.293893),
			Vec3f(0.2, 0.8, 1), Vec3f(0)));
		scene.push_back(new Rectangle(Vec3f(0.0, -0.820135, -0.278169), Vec3f(-0.707107, -0.404509, 0.293893), Vec3f(-0.707107, 0.404509, -0.293893), Vec3f(0.0, -0.0111184, -0.865954),
			Vec3f(0.2, 0.8, 1), Vec3f(0)));
		scene.push_back(new Rectangle(Vec3f(0.0, -0.0111184, -0.865954), Vec3f(-0.707107,0.404509, -0.293893), Vec3f(0.0,0.820135,	0.278169), Vec3f(0.707107,	0.404509, -0.293893),
			Vec3f(0.2, 0.8, 1), Vec3f(0)));


		scene.push_back(new Sphere(Vec3f(0), Vec3f(1), Vec3f(0.5), 10000, 0));
		scene.push_back(new Sphere(Vec3f(2, -0.95, 0), Vec3f(1), Vec3f(3), 0.5, 0));
		scene.push_back(new Sphere(Vec3f(-2, -0.95, 0), Vec3f(1), Vec3f(3), 0.5, 0));
		scene.push_back(new Sphere(Vec3f(0, -1.05, -1), Vec3f(1), Vec3f(0), 0.4, 2));
		scene.push_back(new Sphere(Vec3f(-1, -1.15, 1), Vec3f(1), Vec3f(0), 0.3, 1));
		scene.push_back(new Sphere(Vec3f(1, -1.15, 1), Vec3f(1), Vec3f(0), 0.3, 1));
		scene.push_back(new Triangle(Vec3f(0,2,5), Vec3f(-1,3,5), Vec3f(1,3,5), Vec3f(1), Vec3f(1)));

		scene.push_back(new Sphere(Vec3f(0, -10001.5, 0), Vec3f(0.9, 0.9, 0.9), Vec3f(0), 10000, 0));


		float aspectRatio = (float)columns / (float)rows;

		unsigned int numCores = std::thread::hardware_concurrency();
		std::vector<std::thread> threadList;
		unsigned int coreCount = 0;

		time_t timer;
		double startTime = time(&timer);
		double deltaTime = -1;
		bool calculateTime = false;
		double averageCycleTime = 0;
		double totalTime = 0;

		//For each pixel in the image
		for (int y = 0; y < rows; y++) {

			if (calculateTime) {
				deltaTime = time(&timer) - startTime;
				startTime = time(&timer);
				averageCycleTime = averageCycleTime == 0 ? (averageCycleTime + deltaTime) : (averageCycleTime + deltaTime) / 2.0;
				totalTime = totalTime + deltaTime;
				calculateTime = false;
				std::cout << "Cycle complete, current y: " << y << std::endl;
				std::cout << "  Elapsed time: " << (int)(totalTime / 60) << " minutes, " << (int)totalTime % 60 << " seconds" << std::endl;
				std::cout << "  Previous cycle took " << deltaTime << " seconds to complete" << std::endl;
				std::cout << "  Average cycle time: " << averageCycleTime << " seconds" << std::endl;
				int remainingRows = rows - y;
				double remainingCycles = remainingRows / numCores;
				double estimatedTime = remainingCycles * averageCycleTime;
				int minutesLeft = estimatedTime / 60;
				int secondsLeft = (int)estimatedTime % 60;
				std::cout << "  " << remainingCycles << " cycles left" << std::endl;
				std::cout << "  Estimated time remaining: " << minutesLeft << " minutes, " << secondsLeft << " seconds" << std::endl << std::endl;


			}

			if (USE_MULTITHREADING == 0) {
				//option to render single-threaded, uncomment this if you want to use it
				std::cout << "y: " << y << std::endl;
				renderRow(image, y, rows, columns, maxSamples, camera, scene, false, 0);
				calculateTime = true;
			}
			else {
				//create thread for the row
				threadList.push_back(std::thread(renderRow, std::ref(image), y,
					rows, columns, maxSamples, camera, std::ref(scene), false, 0));

				//if there's max threads for cpu cores, wait until they're done before adding more
				coreCount++;
				if (coreCount % numCores == 0) {
					for (int i = 0; i < threadList.size(); i++) {
						threadList.at(i).join();
					}
					threadList.clear();
					calculateTime = true;
				}
			}





		}
		//Write image
		writeToPPMFile(image, rows, columns, maximumColorValue, frameNum);
	}



	return 0;
}


Vec3f tracePixelColor(Vec3f& rayOrigin, Vec3f& rayDirection,
	std::vector<Object*>& scene, int& depth, bool calculateEmission) {

	if (depth > MAX_DEPTH) return Vec3f(0);
	depth += 1;

	Vec3f backgroundColor = Vec3f(0.1, 0.1, 0.1);


	float closestPoint = INFINITY;

	//check for intersection with object
	int closestObject = getClosestObject(rayOrigin, rayDirection, 
		scene, closestPoint);

	if (closestObject == -1) { //nothing was hit
		//only return the background if it's from a primary ray
		if(depth == 1) return backgroundColor;

		//return black otherwise so it doesn't affect object illumination
		return Vec3f(0);
	}

 	//it's a light so just return the light color
	if (scene[closestObject]->emissionColor.calculateMagnitude() != 0) {
		return Vec3f(scene[closestObject]->emissionColor.x,
			scene[closestObject]->emissionColor.y,
			scene[closestObject]->emissionColor.z) *
			scene[closestObject]->gammaBoost;
	}
	Vec3f objectColor = scene[closestObject]->color;

	float brightestColor = objectColor.x;
	if (objectColor.y > brightestColor) brightestColor = objectColor.y;
	if (objectColor.z > brightestColor) brightestColor = objectColor.z;
	if (depth > 5) {
		if ((float)rand() / RAND_MAX < brightestColor) {
			objectColor = objectColor * (1 / brightestColor);
		}
		else {
			return scene[closestObject]->emissionColor;
		}
	}

	//calculate spot where it intersected and the normal
	Vec3f intersectionPoint = rayOrigin + rayDirection * closestPoint;
	Vec3f normal = scene[closestObject]->computeNormal(intersectionPoint);
	normal.normalize();
	Vec3f alignedNormal = normal; //normal that is pointing out of the sphere
	bool outsideSphere = true;
	if (normal.dot(rayDirection) >= 0) {
		outsideSphere = false;
		alignedNormal = normal * -1;
	}

	float epsilon = 8e-5;
	if (scene[closestObject]->toString() == "SPHERE") {
		Sphere* sphereLight = static_cast<Sphere*>(scene[closestObject]);
		epsilon = 8e-6 * (sphereLight->radius + 1.0);
	}
	Vec3f normalOrigin = intersectionPoint + normal * epsilon;

	Vec3f lightValue = Vec3f(0);

	switch (scene[closestObject]->type) {
	case 0: {//diffuse
		//diffuse reflections
		//calculate a random ray direction in the hemisphere around the normal
		float randomAngle = M_PI * 2 * ((float)rand() / RAND_MAX);
		float distanceModifier = (float)rand() / RAND_MAX;
		float distanceModifier2 = sqrt(distanceModifier);

		//get a set of basis vectors to have a hemisphere around object's normal
		Vec3f e1 = Vec3f(0);

		//make sure the alignedNormal is the "up" vector in the hemisphere
		if (fabs(alignedNormal.x) > fabs(alignedNormal.y)) {
			e1 = Vec3f(alignedNormal.z, 0, -alignedNormal.x) /
				sqrt(alignedNormal.x * alignedNormal.x +
				alignedNormal.z * alignedNormal.z);
		}
		else {
			e1 = Vec3f(0, -alignedNormal.z, alignedNormal.y) /
				sqrt(alignedNormal.y * alignedNormal.y +
				alignedNormal.z * alignedNormal.z);
		}
		e1.normalize();
		Vec3f e2 = alignedNormal.cross(e1);
		e2.normalize();

		Vec3f randomReflection = (e1 * cos(randomAngle) * distanceModifier2 +
			e2 * sin(randomAngle) * distanceModifier2 +
			alignedNormal * sqrt(1 - distanceModifier));
		randomReflection.normalize();

		//Calculate color/lit/unlit (no material)
		for (unsigned int i = 0; i < scene.size(); i++) {
			if (scene[i]->emissionColor.calculateMagnitude() <= 0) continue;

			//shoot another ray towards the face of the light
			Vec3f shadowRay = scene[i]->pos - intersectionPoint;
			Vec3f normalFromLight = intersectionPoint - scene[i]->pos;
			float lightDist = shadowRay.calculateMagnitude();
			shadowRay.normalize();

			bool insideLight = false;
			if (scene[i]->toString() == "SPHERE") {
				Sphere* sphereLight = static_cast<Sphere*>(scene[i]);
				if (lightDist < sphereLight->radius) { 
					//we're inside the light
					continue;
				}
			}


			float angleToObject = 0;
			Vec3f newShadowRay = scene[i]->
				getNewDirectionTowardsLight(shadowRay,alignedNormal,
				normalFromLight, angleToObject, intersectionPoint);

			float s0 = 0;
			float s1 = 0;
			bool inShadow = false;


			//check to see if another object is inbetween the light
			for (unsigned int j = 0; j < scene.size(); j++) {
				if (scene.at(j)->intersect(
					normalOrigin, newShadowRay, s0, s1) && 
					s0 < lightDist) {
					inShadow = true;
					break;
				}
			}
			if (!inShadow) {
				double brdf = 2 * M_PI * (1.0 - angleToObject);
				lightValue = lightValue + objectColor * 
					(alignedNormal.dot(newShadowRay) * brdf) * M_1_PI;
			}

		}
		if (calculateEmission) {
			objectColor = scene[closestObject]->emissionColor + lightValue +
						  objectColor * tracePixelColor(normalOrigin,
						  randomReflection, scene, depth, false);
		}
		else {
			objectColor = lightValue + objectColor *
					tracePixelColor(normalOrigin, randomReflection, 
					scene, depth, true);
		}

		return objectColor;
	}
	case 1: {//mirror
		Vec3f reflDirection = rayDirection - normal * 2 * (rayDirection.dot(normal));
		return scene[closestObject]->emissionColor + objectColor * tracePixelColor(normalOrigin, reflDirection, scene, depth, true);
	}
	case 2: {//glass
		//first calculate reflection
		Vec3f reflDirection = rayDirection - normal * 2 * (rayDirection.dot(normal));

		epsilon = 1e-3;

		float iorValue = scene[closestObject]->IOR;
		bool outGoingIn = false;
		if (normal.dot(alignedNormal) > 0) outGoingIn = true;
		if (outGoingIn) iorValue = 1 / iorValue;
		float normDirAngle = rayDirection.dot(alignedNormal);

		//then check for total internal reflection
		float interiorAngle = 1 - (iorValue * iorValue) * (1 - normDirAngle * normDirAngle);
		if (interiorAngle < 0) {
			return scene[closestObject]->emissionColor + objectColor * tracePixelColor(normalOrigin, reflDirection, scene, depth, true);

		}

		//then calculate refraction
		Vec3f transmissionRay = Vec3f(0);
		Vec3f refractOrigin = intersectionPoint - normal * epsilon;
		float theta = 0;

		if (outGoingIn) {
			transmissionRay = rayDirection * iorValue - normal * (normDirAngle * iorValue + sqrt(interiorAngle));
			transmissionRay.normalize();

			refractOrigin = intersectionPoint - normal * epsilon;

			theta = -normDirAngle;
		}
		else {
			transmissionRay = rayDirection * iorValue - normal * (-1 * (normDirAngle * iorValue + sqrt(interiorAngle)));
			transmissionRay.normalize();

			refractOrigin = intersectionPoint + normal * epsilon;

			theta = transmissionRay.dot(normal);
		}

		//calculate incidence of relfection (f0) and the fresnel equation
		float f0 = ((scene[closestObject]->IOR - 1) * (scene[closestObject]->IOR - 1)) / ((scene[closestObject]->IOR + 1) * (scene[closestObject]->IOR + 1));
		float fresnel = f0 + (1 - f0) * ((1 - theta) * (1 - theta) * (1 - theta) * (1 - theta) * (1 - theta));


		//check if angle of incidence is too shallow,
		if (depth > 2) {
			float fresnelThreshold = fresnel * 0.5 + 0.25;
			if (((float)rand() / RAND_MAX) < fresnelThreshold) {
				Vec3f col = scene[closestObject]->emissionColor + objectColor * tracePixelColor(normalOrigin, reflDirection, scene, depth, true) * (fresnel / fresnelThreshold) * scene[closestObject]->gammaBoost;
				return col;

			}
			else {
				Vec3f col = scene[closestObject]->emissionColor + objectColor * tracePixelColor(refractOrigin, transmissionRay, scene, depth, true) * ((1 - fresnel) / (1 - fresnelThreshold)) * scene[closestObject]->gammaBoost;
				return col;
			}
		}
		else {
			Vec3f refl = tracePixelColor(normalOrigin, reflDirection, scene, depth, true) * fresnel;
			Vec3f refr = tracePixelColor(refractOrigin, transmissionRay, scene, depth, true) * (1 - fresnel);
			Vec3f col = scene[closestObject]->emissionColor + objectColor * (refl + refr)*scene[closestObject]->gammaBoost;
			return col;
		}
	}
	case 3: {//glossy

		float roughness = 0.1; //0 is mirror, 1 is completely rough

		Vec3f reflDirection = rayDirection - normal * 2 * (rayDirection.dot(normal));

		//build a coordinate space in the hemisphere of the shadowray light
		Vec3f se1 = Vec3f(0);
		if (fabs(alignedNormal.x) > fabs(alignedNormal.y)) {
			se1 = Vec3f(reflDirection.z, 0, -reflDirection.x) / sqrt(reflDirection.x * reflDirection.x + reflDirection.z * reflDirection.z);
		}
		else {
			se1 = Vec3f(0, -reflDirection.z, reflDirection.y) / sqrt(reflDirection.y * reflDirection.y + reflDirection.z * reflDirection.z);
		}
		se1.normalize();
		Vec3f se2 = reflDirection.cross(se1);
		se2.normalize();

		//calculate a random direction towards light
		float angleToSphere = sqrt(1 - roughness / normal.dot(normal));
		float randX = (float)rand() / RAND_MAX; //get us a random point
		float randomAngle2 = M_PI * 2 * ((float)rand() / RAND_MAX);
		float angleCos = 1 - randX + randX * angleToSphere;
		float angleSin = sqrt(1 - angleCos * angleCos);
		Vec3f newReflRay = se1 * cos(randomAngle2) * angleSin + se2 * sin(randomAngle2) * angleSin + reflDirection * angleCos;
		newReflRay.normalize();

		return scene[closestObject]->emissionColor + objectColor * tracePixelColor(normalOrigin, newReflRay, scene, depth, true);

	}
	default: { //the sphere is something else and we don't know what it is
		return Vec3f(0);
	}
	}
}

Vec3f calculateRayDirection(int columns, int rows, int x, int y, float sX, float sY, Camera camera) {
	//convert the pixel grid into essentially an object in 3d space
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

int getClosestObject(Vec3f rayOrigin, Vec3f rayDirection, std::vector<Object*>& scene, float& closestPoint) {

	int closestObject = -1;
	float p0 = INFINITY;
	float p1 = INFINITY;

	for (int i = 0; i < scene.size(); i++) {
		if (scene.at(i)->intersect(rayOrigin, rayDirection, p0, p1)) {
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
	std::vector<Object*> scene, bool averageValues, int currentSamples) {

	srand(std::hash<std::thread::id>{}(std::this_thread::get_id()) + currentSamples);

	if (averageValues) {
		for (int x = 0; x < columns; x++) {
			for (int subY = 0; subY < 2; subY++) { //calculate 2x2 subpixel grid
				for (int subX = 0; subX < 2; subX++) {
					float rX = ((float)rand() / RAND_MAX) / 2.0;
					float rY = ((float)rand() / RAND_MAX) / 2.0;
					if (subX == 1) rX += 0.5;
					if (subY == 1) rY += 0.5;

					float closestPoint = INFINITY;

					Vec3f rayOrigin(0);
					Vec3f rayDirection(0);

					rayOrigin.setEach(camera.pos.x, camera.pos.y, camera.pos.z);
					rayDirection = calculateRayDirection(columns, rows, x, y, rX, rY, camera);

					int depth = 0;

					image[y][x] = image[y][x] + tracePixelColor(rayOrigin, rayDirection, scene,
						depth, true);
				}
			}
		}
	}
	else {
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

						float closestPoint = INFINITY;

						Vec3f rayOrigin(0);
						Vec3f rayDirection(0);

						rayOrigin.setEach(camera.pos.x, camera.pos.y, camera.pos.z);
						rayDirection = calculateRayDirection(columns, rows, x, y, rX, rY, camera);

						int depth = 0;


						image[y][x] = image[y][x] + tracePixelColor(rayOrigin, rayDirection, scene,
							depth, true);
						timesSampled++;
					}
				}
			}
			image[y][x] = (image[y][x] / timesSampled);
			image[y][x].clamp();
		}
	}

}



std::ofstream writeToPPMFile(Vec3f** image, int rows, int columns, int maximumColorValue, int frameNum) {

	//Start creating the file
	std::ostringstream oss;
	char s[10];
	sprintf_s(s, "%04d", frameNum);

	oss << "./output/image." << s << ".ppm";
	std::string outputLocation = oss.str();
	std::ofstream file(outputLocation, std::ios::binary);

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
