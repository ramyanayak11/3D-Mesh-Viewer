/* 
Name: RAMYA NAYAK	
Date: October 2024
*/

// starter code provided by Kevin Smith
// modified by Ramya Nayak

#pragma once
#include "ofMain.h"
#include "ofxGUI.h"
#include "glm/gtx/intersect.hpp"
#include <vector>
#include <sstream> 
using namespace std;


// general purpose Ray class
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) {
		this->p = p;
		this->d = d;
	}
	void draw(float t) { 
		ofDrawLine(p, p + t * d); 
	}

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};


// base class for any renderable object in the scene
class SceneObject {
public:
	virtual void draw() = 0; // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return false;
	}

	virtual glm::vec3 getNormal(const glm::vec3& p) { 
		return glm::vec3(1, 0, 0); 
	}

	// returns the type of object
	virtual string getType() {
		return "";
	}

	// material properties (we will ultimately replace this with a Material class - TBD)
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
	

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);
	string name = "SceneObject";

	// default destructor - for delete bug on macs
	virtual ~SceneObject() = default;
};

// written by Ramya Nayak
// light class for any lights in the environment
class Light : public SceneObject {
public:
    float intensity = 0.0;
    glm::vec3 position = glm::vec3(0, 0, 0);
    ofColor diffuseColor = ofColor::lightGrey;
	ofColor specularColor = ofColor::white;

	// checks intersection with light using sphere at position as target
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return false;
	}
};

// written by Ramya Nayak
// point light - subclass of light
class PointLight : public Light {
public:
	ofSpherePrimitive sphere;
    float radius = 0.2;                             // radius of sphere at light position
    
	// constructors
    PointLight() {};
    PointLight(glm::vec3 p, float i) {
        position = p;
	    intensity = i;
	    diffuseColor = ofColor::white;
	    specularColor = ofColor::white;
    } 

	// draw sphere at light position
    void draw() {
		sphere.setRadius(radius);
		sphere.draw();
    }

    string getType() override{
	    return "pointLight"; 
    } 
};

// written by Ramya Nayak
// area light - subclass of light
class AreaLight : public Light {
public:
	int size = 0;
	vector<PointLight*> areaLight;
	ofPlanePrimitive square;

	AreaLight() {}
	AreaLight(glm::vec3 p, float i, int dimensions) {
		position = p;
		intensity = i;
		size = dimensions;
		initializeLight();
	}

	void initializeLight() {
		// assuming square area light has y-axis as normal
		// and square side lengths are even
		for (int i = position.x - size/2; i < size; ++i) {
			for (int j = position.z - size/2; j < size; ++j) {
				areaLight.push_back(new PointLight(glm::vec3(i, position.y, j), intensity));
			}
		}
	}

	// draw plane at area light position
    void draw() {
		square.setPosition(position);
		square.setWidth(size);
		square.setHeight(size);
		square.setResolution(size, size);
		square.draw();
    }

	string getType() override {
		return "areaLight";
	}
};

// general purpose sphere (assume parametric)
class Sphere : public SceneObject {
public:
	float radius = 1.0;

	Sphere() {}
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) {
		position = p; 
		radius = r; 
		diffuseColor = diffuse;
	}

	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}

	glm::vec3 getNormal(const glm::vec3& p) { 
		return glm::normalize(p - position);
	}

	void draw() {
		ofDrawSphere(position, radius);
	}

	void setRadius(float r) {
		radius = r;
	}

	string getType() override{
		return "sphere"; 
	}
};

// general purpose plane
class Plane : public SceneObject { 
public:
	ofPlanePrimitive plane;
	glm::vec3 normal = glm::vec3(0, 1, 0);
	float width = 20;
	float height = 20;

	ofImage diffuseMap;
	ofImage specularMap;
	float tilesUV = 4.0;		// square tiling 4x4

	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 50, float h = 50) {
		position = p;
		normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;

		if (normal == glm::vec3(0, 1, 0)) {
			plane.rotateDeg(-90, 1, 0, 0);
		}
		else if (normal == glm::vec3(0, -1, 0)) {
			plane.rotateDeg(90, 1, 0, 0);
		}
		else if (normal == glm::vec3(1, 0, 0)) {
			plane.rotateDeg(90, 0, 1, 0);
		}
		else if (normal == glm::vec3(-1, 0, 0)) {
			plane.rotateDeg(-90, 0, 1, 0);
		}
	}
	
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal);

	glm::vec3 getNormal() {
		return normal;
	}

	// written by Ramya Nayak:
	// 5 METHODS FOR PLANE TEXTURING 
	// following two methods load the diffuse and specular texture maps
	bool loadDiffuse(string filepath) {
		return diffuseMap.load(filepath);
	}
	bool loadSpecular(string filepath) {
		return specularMap.load(filepath);
	}

	// following two methods calculate the location (in the coord system
	// of the texture map image) and returns the color at that location
	ofColor diffuseTextureLookup(float u, float v) {
		int i = round(u * (diffuseMap.getWidth() - 0.5));
		int j = round(v * (diffuseMap.getHeight() - 0.5));

    	if (i < 0 || i >= diffuseMap.getWidth() || j < 0 || j >= diffuseMap.getHeight()) {
        	cout << "WARNING: Accessing out-of-bounds texture (diffuse) coordinates!" << endl;
        	return ofColor::black; // Return default color
    	}

		return diffuseMap.getColor(i, j);
	}
	ofColor specularTextureLookup(float u, float v) {
		int i = round(u * (specularMap.getWidth() - 0.5));
		int j = round(v * (specularMap.getHeight() - 0.5));

    	if (i < 0 || i >= specularMap.getWidth() || j < 0 || j >= specularMap.getHeight()) {
        	cout << "WARNING: Accessing out-of-bounds texture (specular) coordinates!" << endl;
        	return ofColor::black; // return default color 
    	}

		return specularMap.getColor(i, j);
	}

	// calculates the u,v point on the texture map
	// corresponding to the 3d coordinates on plane
	glm::vec2 calculateUV(glm::vec3 point) {
		float u, v;

		// if plane is horizontal
		if (normal == glm::vec3(0, 1, 0)) {
			u = (point.x - position.x + width * 0.5) / width;
    		v = (point.z - position.z + height * 0.5) / height;
		}
		// if plane is vertical
		else if (normal == glm::vec3(0, 0, 1)) {
			u = (point.x - position.x + width * 0.5) / width;
    		v = (point.y - position.y + height * 0.5) / height;
		}
		

		// for tiling texture onto the plane
		u *= tilesUV;
		v *= tilesUV;

		// wrapping to ensure texture repeats
		// and doesn't exceed [0, 1]
		u -= floor(u);
		v -= floor(v);

		return glm::vec2(u, v);
	}

	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);

		plane.drawWireframe();
	}

	string getType() override {
		return "plane"; 
	}
};

// written by Ramya Nayak
// mesh class
class Mesh : public SceneObject {
public:
    vector<glm::vec3> vertices;                     // holds all vertices of the mesh
    vector<int> triangles;                          // holds indices of the triangles' vertices
    vector<int> rectangles;                         // holds indices of the rectangles' vertices
    vector<glm::vec3> triangleFaceNormals;          // holds face normals of triangles
    vector<glm::vec3> rectangleFaceNormals;         // holds face normals of rectangles

	Mesh() {}
	Mesh(ofColor diffuse = ofColor::lightGray) {
		diffuseColor = diffuse;
	}

    // adds a vertex to the mesh (must call this before addTriangle/addRectangle,
    // as there need to be vertices before a triangle/rectangle can be added)
    void addVertex(glm::vec3 vert) {
        vertices.push_back(vert);
    }

    // adds a triangle to the mesh (through vertex indices), once its vertices are added to the mesh
    void addTriangle(int one, int two, int three) {
        triangles.push_back(one);
        triangles.push_back(two);
        triangles.push_back(three);
    }

    // adds a rectangle to the mesh (through vertex indices), once its vertices are added to the mesh
    void addRectangle(int one, int two, int three, int four) {
        rectangles.push_back(one);
        rectangles.push_back(two);
        rectangles.push_back(three);
        rectangles.push_back(four);
    }

    // draws the mesh by connecting triangles' and rectangles' vertices
    void draw() {

        for (size_t i = 0; i < triangles.size(); i += 3) {
            int ind1 = triangles[i];
            int ind2 = triangles[i+1];
            int ind3 = triangles[i+2];

            glm::vec3 v1 = vertices[ind1];          // use the indices retrieved above to get the vertices of
            glm::vec3 v2 = vertices[ind2];          // the current triangle
            glm::vec3 v3 = vertices[ind3];

            ofDrawTriangle(v1, v2, v3);
        }

        for (size_t i = 0; i < rectangles.size(); i += 4) {
            int ind1 = rectangles[i];
            int ind2 = rectangles[i+1];
            int ind3 = rectangles[i+2];
            int ind4 = rectangles[i+3];

            glm::vec3 v1 = vertices[ind1];          // use the indices retrieved above to get the vertices of
            glm::vec3 v2 = vertices[ind2];          // the current rectangle
            glm::vec3 v3 = vertices[ind3];
            glm::vec3 v4 = vertices[ind4];

            ofPolyline polyline;
            polyline.addVertex(v1);
            polyline.addVertex(v2);
            polyline.addVertex(v3);
            polyline.addVertex(v4);
            polyline.close();  						// close the shape
            polyline.draw();   						// draw the rectangle
        }
   }

    // reads and parses an .obj file, while loading the vertex and face data
    bool processFileOBJ(string filename) {
        ofFile objFile(filename);
        
        if (!objFile.exists()) {                    // if the file does not exist, print a warning and return false
            cout << "ERROR: FILE NOT FOUND" << endl;
            return false;
        }

        ofBuffer buff = objFile.readToBuffer();     // buffer to read the .obj file
        stringstream ss(buff.getText());            // load the ss object with text from the buffer
        string currLine;                            // hold the content of the current line

        while (getline(ss, currLine)) {             // iterate through each line in the file
            stringstream linestream(currLine);
            string kind;
            linestream >> kind;                     // read the first char of the line which contains the type (v or f)

            if (kind == "v") {                      // if line contains vertex data, parse it and add the vertex to our mesh
                float x, y, z;
                linestream >> x >> y >> z;
                addVertex(glm::vec3(x, y, z));
            }
            else if (kind == "f") {                 // if line has face data, parse it and add the face(triangle) to our mesh
                vector<int> indices;                // holds the indices (works with triangles and rectangles)
                string faceInfo;                    // holds the face info (i.e. vert/vertTexture/vertNorm)

                while (linestream >> faceInfo) {
                    size_t slashPos = faceInfo.find('/');  				// find position of the '/'
                    if (slashPos != string::npos) {                    	// if there is a '/' in the string, get the string
                        faceInfo = faceInfo.substr(0, slashPos); 		//      before it (one of the vertices of the face)
                    }
                    int vertex = stoi(faceInfo);            			// convert the vertex from a string to int
                    indices.push_back(vertex-1);            			// obj index starts at 1, convert to c++'s 0-based index
                }

                if (indices.size() == 3) {											// if 3 vertices, add to triangles vector
                    addTriangle(indices[0], indices[1], indices[2]);
                }
                else if (indices.size() == 4) {										// if 4 vertices, add to rectangle vector
                    addRectangle(indices[0], indices[1], indices[2], indices[3]);
                }
            }
            else {														// ignore any other lines
                continue;
            }
        }

		calculateNormals();							// calculate the normals of each face once the mesh data is read from obj file

        return true;
    }

	void calculateNormals() {
		for (size_t i = 0; i < triangles.size(); i += 3) {
			int ind1 = triangles[i];					// get the indices of the current triangle's vertices
			int ind2 = triangles[i+1];
			int ind3 = triangles[i+2];

			glm::vec3 vert1 = vertices[ind1];			// use the indices retrieved above to get the vertices of
			glm::vec3 vert2 = vertices[ind2];			// the current triangle
			glm::vec3 vert3 = vertices[ind3];

			glm::vec3 edge1 = vert3 - vert1;			// calculate two edges from the vertices
			glm::vec3 edge2 = vert2 - vert1;

			glm::vec3 normal = glm::normalize(glm::cross(edge2, edge1)) * -1;

			triangleFaceNormals.push_back(normal);
		}

		for (size_t i = 0; i < rectangles.size(); i += 4) {
			int ind1 = rectangles[i];					// get the indices of the current rectangle's vertices
            int ind2 = rectangles[i+1];
            int ind3 = rectangles[i+2];
            int ind4 = rectangles[i+3];

            glm::vec3 vert1 = vertices[ind1];          	// use the indices retrieved above to get the vertices of
            glm::vec3 vert2 = vertices[ind2];          	// the current rectangle
            glm::vec3 vert3 = vertices[ind3];
            glm::vec3 vert4 = vertices[ind4];

			glm::vec3 edge1 = vert3 - vert1;			// calculate two edges from the vertices
			glm::vec3 edge2 = vert2 - vert1;

			// calculate the cross product of the two edges and normalize it
			glm::vec3 normal = glm::normalize(glm::cross(edge2, edge1)) * -1;	// cross prod of edge1 x edge2 made normals inwards

			rectangleFaceNormals.push_back(normal);
		}
	}

	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { 

		bool hit = false;
		float closestDist = std::numeric_limits<float>::max();

		// checks ray intersection with triangle faces
		for (size_t i = 0; i < triangles.size(); i += 3) {
			int ind1 = triangles[i];					// get the indices of the current triangle's vertices
			int ind2 = triangles[i+1];
			int ind3 = triangles[i+2];

			glm::vec3 vert1 = vertices[ind1];			// use the indices retrieved above to get the vertices of
			glm::vec3 vert2 = vertices[ind2];			// the current triangle
			glm::vec3 vert3 = vertices[ind3];

			// for calculating the ray-triangle intersection
			float t;
			glm::vec2 baryPoint;
			bool intersects = glm::intersectRayTriangle(ray.p, ray.d, vert1, vert2, vert3, baryPoint, t);

			if (intersects && t > 0 && t < closestDist) {
				hit = true;
				closestDist = t;
				point = ray.p + ray.d * t;				// calculate and save intersection point between ray and triangle
				normal = triangleFaceNormals[i/3];		// save the triangle's face normal
			}
		}

		// checks ray intersection with rectangle faces
		// splits each rectangle into two triangles to check for ray intersection
		for (size_t i = 0; i < rectangles.size(); i += 4) {
			int ind1 = rectangles[i];					// get the indices of the current rectangle's vertices
            int ind2 = rectangles[i+1];
            int ind3 = rectangles[i+2];
            int ind4 = rectangles[i+3];

            glm::vec3 vert1 = vertices[ind1];          	// use the indices retrieved above to get the vertices of
            glm::vec3 vert2 = vertices[ind2];          	// the current rectangle
            glm::vec3 vert3 = vertices[ind3];
            glm::vec3 vert4 = vertices[ind4];

			// for calculating the ray-triangle intersection
			float t;
			glm::vec2 baryPoint;

			// split rectangle into two triangles to check for ray intersection:
			// triangle1 = vert1, vert2, vert3 , triangle2 = vert1, vert3, vert4
			bool intersects = 	glm::intersectRayTriangle(ray.p, ray.d, vert1, vert2, vert3, baryPoint, t) ||
								glm::intersectRayTriangle(ray.p, ray.d, vert1, vert3, vert4, baryPoint, t);
			if (intersects && t > 0 && t < closestDist) {
				hit = true;
				closestDist = t;
				point = ray.p + ray.d * t;				// calculate and save intersection point between ray and triangle
				normal = rectangleFaceNormals[i/4];		// save the triangle's face normal
			}

		}

		return hit; 
	}

	string getType() override {
		return "mesh"; 
	}
};

// view plane for render camera
class ViewPlane : public Plane {
public:
	glm::vec2 min, max;

	ViewPlane(glm::vec2 p0, glm::vec2 p1) {
		min = p0;
		max = p1;
	}

	ViewPlane() { // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 10);
		normal = glm::vec3(0, 0, 1); // viewplane currently limited to z-axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) {
		this->min = min;
		this->max = max;
	}

	float getAspect() { 
		return width() / height(); 
	}

	glm::vec3 toWorld(float u, float v); // (u, v) --> (x, y, z) [ world space ]
	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}
	
	// returns the width and height of the plane, respectively
	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	glm::vec2 topLeft() { 
		return glm::vec2(min.x, max.y); 
	}
	glm::vec2 topRight() { 
		return max; 
	}
	glm::vec2 bottomLeft() { 
		return min; 
	}
	glm::vec2 bottomRight() { 
		return glm::vec2(max.x, min.y); 
	}
};


// render camera - currently must be z axis aligned (we will improve this in project 4)
class RenderCam : public SceneObject {
public:
	glm::vec3 aim;
	ViewPlane view; // The camera viewplane, this is the view that we will render

	RenderCam() {
		//position = glm::vec3(0, 0, 15);
		position = glm::vec3(0, -0.5, 15);
		aim = glm::vec3(0, 0, -1);
	}

	Ray getRay(float u, float v);

	void draw() { 
		ofDrawBox(position, 1.0); 
	};

	void drawFrustum();

};



//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------



class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void drawGrid();
	void drawAxis(glm::vec3 position);


	// following methods implemented by Ramya Nayak
	void rayTrace();
	ofColor shaded(glm::vec3 &hitPoint, glm::vec3 &norm, ofColor diffuse, float power, SceneObject* closestObj);
	ofColor phongLambert(glm::vec3 &hitPoint, glm::vec3 &norm, Light* light, ofColor diffuse, ofColor specular, float power);
	void clampColor(ofColor &color);


	// cameras
	ofEasyCam mainCam;
	ofCamera topCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera *theCam;				// set to current camera either mainCam or sideCam

	// image rendering
	RenderCam renderCam;			// set up one render camera to render image through
	ofImage rendImage;				// to save the rendered image to
	int imageWidth = 1200;			// dimensions for rendered image
	int imageHeight = 800;

	vector<SceneObject *> scene;	// list of scene objects
	vector<Light *> lights;			// separate list for lights in the scene
	

	// state
	bool bHide = false;				// to help display gui 
	bool bShowImage = false;		// to overlay the image

	// gui
	ofxPanel gui;
	ofxFloatSlider lightIntensity;	// to adjust the lights' intensity
	ofxFloatSlider powerPhong;		// to adjust the power exponent used in phong shading

	// file reading
	bool meshProcessFail = false;
	bool textureProcessFail = false;

};