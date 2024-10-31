/* 
Name: RAMYA NAYAK	
Date: October 2024
*/

// starter code provided by Kevin Smith
// modified by Ramya Nayak

#include "ofApp.h"

// intersect Ray with Plane (wrapper on glm::intersect*)
bool Plane::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2);
        glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2);
		
        if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) {
				insidePlane = true;
		}
    }
	return insidePlane;
}


// Convert (u, v) to (x, y, z)
// We assume u,v is in [0, 1]
glm::vec3 ViewPlane::toWorld(float u, float v) {
    float w = width();
    float h = height();
    return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on the ViewPlane
Ray RenderCam::getRay(float u, float v) {
    glm::vec3 pointOnPlane = view.toWorld(u, v);
    return (Ray(position, glm::normalize(pointOnPlane - position)));
}

// could be drawn a lot simpler but wanted to use the getRay call to test it at the corners
void RenderCam::drawFrustum() {
    Ray r1 = getRay(0, 0);
    Ray r2 = getRay(0, 1);
    Ray r3 = getRay(1, 1);
    Ray r4 = getRay(1, 0);

    float dist = glm::length((view.toWorld(0, 0) - position));

    r1.draw(dist);
    r2.draw(dist);
    r3.draw(dist);
    r4.draw(dist);
}

// draw an XYZ axis in RGB at world (0,0,0) for reference.
void ofApp::drawAxis(glm::vec3 position) {
    ofPushMatrix();
    ofTranslate(position);
    ofSetLineWidth(1.0);

    // X Axis
    ofSetColor(ofColor(255, 0, 0));
    ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));

    // Y Axis
    ofSetColor(ofColor(0, 255, 0));
    ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

    // Z Axis
    ofSetColor(ofColor(0, 0, 255));
    ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

    ofPopMatrix();
}

void ofApp::drawGrid() {
    float u = 0;
    float v = 0;
    float pixelWidth = 1.0 / imageWidth;
    float pixelHeight = 1.0 / imageHeight;
    for (int x = 0; x < imageWidth; x++) {
        glm::vec3 p1 = renderCam.view.toWorld(u, 0);
        glm::vec3 p2 = renderCam.view.toWorld(u, 1);
        ofDrawLine(p1, p2);
        u += pixelWidth;
    }
    for (int y = 0; y < imageHeight; y++) {
        glm::vec3 p1 = renderCam.view.toWorld(0, v);
        glm::vec3 p2 = renderCam.view.toWorld(1, v);
        ofDrawLine(p1, p2);
        v += pixelHeight;
    }
}


//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------


// written by Ramya Nayak
// ray tracing method
void ofApp::rayTrace() {
    rendImage.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);

    for (int j = 0; j < imageHeight; ++j) {
        for (int i = 0; i < imageWidth; ++i) {
            float u = (i + 0.5) / imageWidth;                   // convert from int pixel coordinates (i, j) to normalized coordinates (u, v)
            float v = 1.0 - ((j + 0.5) / imageHeight);          // subtracting from 1, because otherwise the image is flipped

            Ray viewingRay = renderCam.getRay(u, v);

            glm::vec3 hitPoint, hitSurfaceNormal;
            SceneObject *closestObj = nullptr;
            float closestDist = std::numeric_limits<float>::max();

            // check which scene object the ray intersects first
            for (int k = 0; k < scene.size(); ++k) {

                if (scene[k]->getType() == "light") {           // does not bother getting shading for the light objects
                    rendImage.setColor(i, j, scene[k]->diffuseColor);
                    continue;
                }

                glm::vec3 interPoint, interNormal;
                float distance;

                if (scene[k]->intersect(viewingRay, interPoint, interNormal)) {
                    distance = glm::distance(viewingRay.p, interPoint);

                    if (distance < closestDist) {
                        closestDist = distance;
                        closestObj = scene[k];
                        hitPoint = interPoint;
                        hitSurfaceNormal = interNormal;
                    }
                }
            }

            if (closestObj != nullptr) {                        // if the current ray intersects an object set color
                ofColor finalColor = shaded(hitPoint, hitSurfaceNormal, closestObj->diffuseColor, powerPhong, closestObj);
                rendImage.setColor(i, j, finalColor);
            }
            else {                                              // else if the ray does not intersect anything, set black
                rendImage.setColor(i, j, ofColor::black);
            }

        }
    }

   rendImage.update();
   rendImage.save("renderedImage.png");
} 


// written by Ramya Nayak
// checks if point is shadowed, otherwise calls phongLambert() for shading
ofColor ofApp::shaded(glm::vec3 &hitPoint, glm::vec3 &norm, ofColor diffuse, float power, SceneObject* closestObj) {
    ofColor pixelColor = ofColor::black;                                            // initialize with no color

    for (int i = 0; i < lights.size(); ++i) {
        glm::vec3 lightDirection = glm::normalize(lights[i]->position - hitPoint);
        Ray shadowRay(hitPoint + (lightDirection * 0.001f), lightDirection);

        bool shadowed = false;
        for (int j = 0; j < scene.size(); ++j) {                                    // check if object is shadowed
            glm::vec3 interPoint;
            glm::vec3 interNormal;         
            if (scene[j]->intersect(shadowRay, interPoint, interNormal)) {
                shadowed = true;
                break;
            }                    
        }

        if (!shadowed) {
            // if object is plane, get its texture 
            if (closestObj->getType() == "plane") {
                Plane* plane = dynamic_cast<Plane*>(closestObj);                    // to access methods specific to the plane class
                if (plane) {
                    glm::vec2 uv = plane->calculateUV(hitPoint);

                    ofColor textureDiffuse = plane->diffuseTextureLookup(uv.x, uv.y);
                    ofColor textureSpecular = plane->specularTextureLookup(uv.x, uv.y);

                    pixelColor += phongLambert(hitPoint, norm, lights[i], textureDiffuse, textureSpecular, power);
                }
            }
            else {
                pixelColor += phongLambert(hitPoint, norm, lights[i], diffuse, lights[i]->specularColor, power);
            }
            
        }
    }

    clampColor(pixelColor);

    return pixelColor;
}

// written by Ramya Nayak
// does phong and lambert shading
ofColor ofApp::phongLambert(glm::vec3 &hitPoint, glm::vec3 &norm, Light* light, ofColor diffuse, ofColor specular, float power) {
    glm::vec3 lightDirection = glm::normalize(light->position - hitPoint);
    glm::vec3 viewDirection = glm::normalize(renderCam.position - hitPoint);
    glm::vec3 bisector = glm::normalize(viewDirection + lightDirection);

    // DIFFUSE
    float dotProdDiffuse = glm::dot(lightDirection, norm);
    dotProdDiffuse = std::max(0.0f, dotProdDiffuse);
    ofColor diffuseColor = lightIntensity * diffuse * dotProdDiffuse;                   // equation for diffuse using above calculated values

    // SPECULAR
    float dotProdSpecular = glm::dot(norm, bisector);
    dotProdSpecular = std::max(0.0f, dotProdSpecular);
    ofColor specularColor = lightIntensity * specular * pow(dotProdSpecular, power);    // equation for specular using two values calculated above

    ofColor pixelColor = diffuseColor + specularColor;                   // add all components to get final shading color

    if (diffuse == ofColor::red) {
        pixelColor += diffuse * 0.2;                                    // ambient contribution
    }

    clampColor(pixelColor);

    return pixelColor;
}

// written by Ramya Nayak
// clamps the rgb values of the pixel color to ensure that
// they stay within the color range (0, 255)
void ofApp::clampColor(ofColor &color) {
    color.r = std::max(0, std::min(255, (int)color.r));
    color.g = std::max(0, std::min(255, (int)color.g));
    color.b = std::max(0, std::min(255, (int)color.b));
}


//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------


void ofApp::setup() {
    ofSetBackgroundColor(ofColor::black);
    ofEnableDepthTest();
    mainCam.setDistance(15);
    mainCam.setNearClip(.1);
    sideCam.setPosition(25, 0, 0);
    sideCam.lookAt(glm::vec3(0, 0, 0));
    topCam.setPosition(0, 25, 0);
    topCam.lookAt(glm::vec3(0, -1, 0));
    previewCam.setPosition(renderCam.position);
    previewCam.setNearClip(.1);
    previewCam.lookAt(glm::vec3(0, 0, 0));
    theCam = &mainCam;

    gui.setup();
    gui.add(lightIntensity.setup("Light intensity", 0.6, 0.0, 5.0));    // light intensity, initial value = 0.6 and (min, max) = (0, 5);
    gui.add(powerPhong.setup("Power exponent", 35.0, 10.0, 500.0));     // power phong, initial value = 35 and (min, max) = (10, 500); 

    // process mesh
    Mesh *mesh = new Mesh(ofColor::red);
    scene.push_back(mesh);
    if (!mesh->processFileOBJ("cherry.obj")) {
        cout << "ERROR PROCESSING .OBJ FILE" << endl << endl;
        meshProcessFail = true; 
    }

    // spheres
    scene.push_back(new Sphere(glm::vec3(4, 0.3, 2), 1.5, ofColor::darkOliveGreen));
    scene.push_back(new Sphere(glm::vec3(-2, 3, -5), 3, ofColor::blueSteel));
    scene.push_back(new Sphere(glm::vec3(-3, -0.8, 1), 1.20, ofColor::orangeRed));

    // planes
    Plane *ground = new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor::rosyBrown);     // horizontal plane (floor)
    scene.push_back(ground); 
    Plane *backWall = new Plane(glm::vec3(0, 0, -26), glm::vec3(0, 0, 1), ofColor::white);      // vertical plane (back wall)
    scene.push_back(backWall);

    // lights
    /*
    AreaLight *area = new AreaLight(glm::vec3(0, 20, 3), 0.08, 4);
    for (int i = 0; i < area->areaLight.size(); ++i) {
        lights.push_back(area->areaLight[i]);
        scene.push_back(area->areaLight[i]);
    }
    */

    PointLight *light1 = new PointLight(glm::vec3(-4, 2, 20), 0.6);         // left light
    lights.push_back(light1);
    scene.push_back(light1);

    PointLight *light2 = new PointLight(glm::vec3(12, 30, 10), 0.6);        // right light
    lights.push_back(light2); 
    scene.push_back(light2);

    // loading texture for the horizontal plane
    if(!ground->loadDiffuse("woodFloor_diffuse.jpeg")) {
        cout << "ERROR PROCESSING TEXTURE (DIFFUSE) FOR PLANE 1." << endl << endl;
        textureProcessFail = true;
    }
    if(!ground->loadSpecular("woodFloor_specular.jpeg")) {
        cout << "ERROR PROCESSING TEXTURE (SPECULAR) FOR PLANE 1" << endl << endl;
        textureProcessFail = true; 
    }

    // loading texture for the vertical plane
    if(!backWall->loadDiffuse("floralWall2_diffuse.jpeg")) {
        cout << "ERROR PROCESSING TEXTURE (DIFFUSE) FOR PLANE 2." << endl << endl;
        textureProcessFail = true;
    }
    if(!backWall->loadSpecular("floralWall2_specular.jpeg")) {
        cout << "ERROR PROCESSING TEXTURE (SPECULAR) FOR PLANE 2" << endl << endl;
        textureProcessFail = true;
    }

}

//--------------------------------------------------------------
void ofApp::update() {
    if (meshProcessFail || textureProcessFail) {
        ofExit();
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    theCam->begin();
    ofEnableDepthTest();
    theCam->setNearClip(0.1);
    ofNoFill();

    // draw objects in scene
	for (int i = 0; i < scene.size(); i++) {
		ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();
	}

    ofDisableLighting();

    theCam->end();

    rayTrace();

    if (bShowImage) {
        float scaleFactor = 0.67;

        float scaledW = rendImage.getWidth() * scaleFactor;
        float scaledH = rendImage.getHeight() * scaleFactor;

        ofSetColor(ofColor::white);                             // avoid colors in the overlay looking off

        rendImage.draw((ofGetWidth() - scaledW) / 2, (ofGetHeight() - scaledH) / 2, scaledW, scaledH);
    }

    if (!bHide) {                                               // display gui on screen
        ofDisableDepthTest();
        gui.draw();
    } 
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    switch (key) {
        case 'C':
        case 'c':
            if (mainCam.getMouseInputEnabled())
                mainCam.disableMouseInput();
            else
                mainCam.enableMouseInput();
            break;
        case 'F':
        case 'b':
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case 'h':
            bHide = !bHide;                     // hot key to show/hide the sliders
            break;
        case 'i':
            bShowImage = !bShowImage;           // hot key to display the rendered image as an overlap
            break;
        case 'n':
            scene.push_back(new Sphere(glm::vec3(0, 0, 0), 1.0, ofColor::violet));
            break;
        case 'r':
            rayTrace();
            cout << "rendering..." << endl;
            cout << "done..." << endl << endl;
            break;
        case 'm':
            break;
        case OF_KEY_F1:
            theCam = &mainCam;
            break;
        case OF_KEY_F2:
            theCam = &sideCam;
            break;
        case OF_KEY_F3:
            theCam = &previewCam;
            break;
        case OF_KEY_F4:
            theCam = &topCam;
		    break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}