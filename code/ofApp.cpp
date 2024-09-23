/* 
Name: RAMYA NAYAK	
Date: September 2024
*/

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // replace the string parameter with the path of an .obj file within the Project1 file
    if (!mesh.processFileOBJ("/Users/ramyanayak/Documents/of_v0.12.0_osx_release/apps/myApps/Project1/pyramid.obj")) {
        cout << "error processing .obj file" << endl;
    }
    else {
        mesh.printDiagnostics();            // if no error occurs processing the file, the structure's info is printed

        mesh.calculateNormals();            // normals are calculated at this stage since they only need to be calculated once
    }                                       // (doing this in the draw() method below would call the mesh's calculateNormals()
                                            // function in every frame, therefore it would lead to a greater runtime)

    cam.setDistance(10.0);
    cam.setNearClip(.1);
    cam.setTarget(glm::vec3(0, 0, 0));
   
    ofNoFill();

    gui.setup();
    gui.add(showNormals.setup("Show Normals", false));       // add a toggle button to control whether or not normals are shown
    gui.add(normalLength.setup("Length of Normals", 5.0, 0.0, 30.0));   // add a slider, where 5 is the initial value, and
                                                                        // 0 and 30 are the min and max values of the slider
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    cam.begin();


    mesh.draw();                                    // draws the image from the .obj file

    if (showNormals) {                              // if the "show normals" toggle is switched on, 
        mesh.drawNormals(normalLength);             // the normals drawn
    }
    

    cam.end();

    if (!bHide) {
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::exit() {

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
