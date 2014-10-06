// STTriangleMesh.h
#ifndef __STTRIANGLEMESH_H__
#define __STTRIANGLEMESH_H__

#include "STPoint3.h"

#include <string>
#include <vector>
#include <iostream>

struct STFace;

struct STVertex{
    STVertex(float x, float y, float z){
        pt=STPoint3(x,y,z);
    }
    STVertex(const STPoint3& pt){
        this->pt=pt;
    }
    STPoint3 pt;
    STVector3 normal;
    STFace *f;
};
inline std::ostream& operator <<(std::ostream& stream, const STVertex& v);

struct STFace{
    STFace(STVertex* v0, STVertex* v1, STVertex* v2){
        v[0]=v0;
        v[1]=v1;
        v[2]=v2;
    }
    STFace(STVertex* v0, STVertex* v1, STVertex* v2, STVector3* n1, STVector3* n2, STVector3* n3){
        v[0]=v0;
        v[1]=v1;
        v[2]=v2;
        normals[0]=n1;
        normals[1]=n2;
        normals[2]=n3;
    }
    STVertex *v[3];
    STFace *adjF[3];
    STVector3 normal;
    STVector3 *normals[3];
};
inline std::ostream& operator <<(std::ostream& stream, const STFace& f);

/**
* STTriangleMesh use a simple data structure to represent a triangle mesh.
*/
class STTriangleMesh
{
public:
    //
    // Initialization
    //
    STTriangleMesh();
    STTriangleMesh(const std::string& filename);

    //
    // Delete and clean up.
    //
    ~STTriangleMesh();

    bool mSimpleMesh;

    //
    // Draw the triangle mesh to the OpenGL window using GL_TRIANGLES.
    //
    void Draw(bool smooth) const;

    //
    // Read and Write the triangle mesh from/to files.
    //
    bool Read(const std::string& filename);

    bool Write(const std::string& filename);

    unsigned int AddVertex(float x, float y, float z);

    unsigned int AddVertex(const STPoint3& pt);

    unsigned int AddFace(unsigned int id0,unsigned int id1,unsigned int id2);
    
    //
    // Build topology and calculate normals for the triangle mesh.
    //
    bool Build();
    bool BuildTopology();
    bool CalculateNormal();

    STFace* NextAdjFace(STVertex *v, STFace *f);
    STFace* NextAdjFaceReverse(STVertex *v, STFace *f);

    void LoopSubdivide();
    //
    // Local members
    //
    std::vector<STVertex*> mVertices;
    std::vector<STVector3*> mNormals;
    std::vector<STFace*> mFaces;
};

#endif  // __STTRIANGLEMESH_H__

