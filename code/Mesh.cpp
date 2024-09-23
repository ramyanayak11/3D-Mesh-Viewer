/* 
Name: RAMYA NAYAK	
Date: September 15, 2024
*/

#include <vector>
#include <glm/glm.hpp>
#include <sstream>
#include "ofxGUI.h"
using namespace std; 

class Mesh {

public:
    vector<glm::vec3> vertices;
    vector<int> triangles;
    vector<glm::vec3> faceNormals;

    // adds a vertex to the mesh (must call this before addTriangle, as
    // there need to be vertices before a triangle can be added/drawn)
    void addVertex(glm::vec3 vert) {
        vertices.push_back(vert);
    }

    //--------------------------------------------------------------
    // adds a triangle to the mesh (through vertex indices), 
    // once the vertices are added to the mesh
    void addTriangle(int one, int two, int three) {
        triangles.push_back(one);
        triangles.push_back(two);
        triangles.push_back(three);
    }

    //--------------------------------------------------------------
    // draws the image by connecting triangles' vertices
    void draw() {

        for (size_t i = 0; i < triangles.size(); i+=3) {
            int ind1 = triangles[i];                // get the indices of the current triangle's vertices
            int ind2 = triangles[i+1];
            int ind3 = triangles[i+2];

            glm::vec3 v1 = vertices[ind1];          // use the indices retrieved above to get the vertices of
            glm::vec3 v2 = vertices[ind2];              // the current triangle
            glm::vec3 v3 = vertices[ind3];

            ofDrawTriangle(v1, v2, v3);             // draw the triangle based on the data read above
        }
    }

    //--------------------------------------------------------------
    // reads and parses an .obj file, while loading the vertex and face data into
    // the mesh class' vertices and triangles vectors
    bool processFileOBJ(string filename) {
        ofFile objFile(filename);
        
        if (!objFile.exists()) {                    // if the file does not exist, print a warning and return false
            cout << "error: file not found" << endl;
            return false;
        }

        ofBuffer buff = objFile.readToBuffer();     // buffer to read the .obj file
        stringstream ss(buff.getText());            // load the ss object with text from the buffer
        string currLine;                            // hold the content of the current line

        while (getline(ss, currLine)) {             // iterate through each line in the file
            stringstream linestream(currLine);
            char kind;
            linestream >> kind;                     // read the first char of the line which contains the type (v or f)

            if (kind == 'v') {                      // if line contains vertex data, parse it and add the vertex to our mesh
                float x, y, z;
                linestream >> x >> y >> z;
                addVertex(glm::vec3(x, y, z));
            }
            else if (kind == 'f') {                 // if line has face data, parse it and add the face(triangle) to our mesh
                int v1, v2, v3;
                linestream >> v1 >> v2 >> v3;
                addTriangle(v1 - 1, v2 - 1, v3 - 1);// obj files start vertex index at 1, but c++ starts at 0
            }
        }

        return true;
    }

    //--------------------------------------------------------------
    // calculates the face normals for each triangle in the mesh
    void calculateNormals() {
        for (size_t i = 0; i < triangles.size(); i+=3) {
            int ind1 = triangles[i];                // get the indices of the current triangle's vertices
            int ind2 = triangles[i+1];
            int ind3 = triangles[i+2];

            glm::vec3 v1 = vertices[ind1];          // use the indices retrieved above to get the vertices of
            glm::vec3 v2 = vertices[ind2];              // the current triangle
            glm::vec3 v3 = vertices[ind3];

            glm::vec3 edge1 = v3 - v1;              // calculate two edges from the vertices
            glm::vec3 edge2 = v2 - v1;

            // calculate the cross product of the two edges and normalize it
            glm::vec3 normal = glm::normalize(glm::cross(edge2, edge1));

            faceNormals.push_back(normal);          // add the normal to faceNormals vector for storage and ease in drawing
        }
    }

    //--------------------------------------------------------------
    // draws the face normals according to the desired length
    void drawNormals(float normalLength) {
        for (size_t i = 0; i < triangles.size(); i+=3) {
            int ind1 = triangles[i];                // get the indices of the current triangle's vertices
            int ind2 = triangles[i+1];
            int ind3 = triangles[i+2];

            glm::vec3 v1 = vertices[ind1];          // use the indices retrieved above to get the vertices of
            glm::vec3 v2 = vertices[ind2];              // the current triangle
            glm::vec3 v3 = vertices[ind3];

            glm::vec3 center = (v1 + v2 + v3) / 3;  // formula for calculating the centroid (center point)
                                                        // of a triangle (since each vertex contains x,y,z)

            glm::vec3 normal = faceNormals[i/3];    // retrieve the normal of the current face (divide by 3 because
                                                        // 3 vertices are stored consecutively for 1 triangle)

            glm::vec3 endpoint = center + (normal * normalLength);  // point that marks the endpoint of the face normal
                                                                        // calculated using the center (starting point),
                                                                        // normal, and the desired length of the normal
            
            ofDrawLine(center, endpoint);           // draw the face's normal from the calculations made above
        }
    }

    //--------------------------------------------------------------
    // prints the total vertices, faces, and mesh size of the structure
    void printDiagnostics() {
        size_t numVertices = vertices.size();       // total number of vertices
        size_t numFaces = triangles.size() / 3;     // total number of faces (divide by 3 because the 'triangles'
                                                        // contains the indices of 3 vertices for 1 triangle)
        // size of the mesh structure in bytes, and then divided by 1000 to get the size in kilobytes
        size_t meshSize = ((numVertices * sizeof(glm::vec3)) + (numFaces * sizeof(int))) / 1000;

        // print the diagnostic info calculated above
        cout << endl << "Total vertices: " << numVertices << endl;
        cout << "Total faces: " << numFaces << endl;
        cout << "Mesh size (KB): " << meshSize << endl << endl;
    }



};
