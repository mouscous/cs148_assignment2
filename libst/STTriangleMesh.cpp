// STTriangleMesh.cpp
#include "STTriangleMesh.h"
/* Include-order dependency!
*
* GLEW must be included before the standard GL.h header.
* In this case, it means we must violate the usual design
* principle of always including Foo.h first in Foo.cpp.
*/
#ifdef __APPLE__
#define GLEW_VERSION_2_0 1
#include <OpenGL/gl.h>
#else
#define GLEW_STATIC
#include "GL/glew.h"
#include "GL/gl.h"
#endif

#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <string.h>
#include <algorithm>
#define PI 3.14159265

//
// Initialization
//
STTriangleMesh::STTriangleMesh()
{
    mSimpleMesh = true;
}

STTriangleMesh::STTriangleMesh(const std::string& filename)
{
    mSimpleMesh = true;
    Read(filename);
    Build();
}

//
// Delete and clean up an existing image.
//
STTriangleMesh::~STTriangleMesh()
{
    for(unsigned int i=0;i<mVertices.size();i++)delete mVertices[i];
    for(unsigned int i=0;i<mNormals.size();i++)delete mNormals[i];
    for(unsigned int i=0;i<mFaces.size();i++)delete mFaces[i];
}

//
// Draw the triangle mesh to the OpenGL window using GL_TRIANGLES.
//
void STTriangleMesh::Draw(bool smooth) const
{
    glBegin(GL_TRIANGLES);
    for (unsigned int i = 0; i < mFaces.size(); i++) {
        STFace* f=mFaces[i];
        for (unsigned int j = 0; j < 3; j++) {
            if(smooth)
                if(!mSimpleMesh)
                    glNormal3f(f->normals[j]->x,f->normals[j]->y,f->normals[j]->z);
                else 
                    glNormal3f(f->v[j]->normal.x,f->v[j]->normal.y,f->v[j]->normal.z);
            else
                glNormal3f(f->normal.x,f->normal.y,f->normal.z);
            glVertex3f(f->v[j]->pt.x,f->v[j]->pt.y,f->v[j]->pt.z);
        }
    }
    glEnd();
}

//
// Read the triangle mesh from files.
//
bool STTriangleMesh::Read(const std::string& filename)
{
    // Determine the right routine based on the file's extension.
    // The format-specific subroutines are each implemented in
    // a different file.
    std::string ext = STGetExtension( filename );
    if (ext.compare("OBJ") == 0){
        std::ifstream in( filename.c_str(), std::ios::in );

        if( !in ){
            std::cout << "cannot open file" << filename << std::endl;
            return false;
        }

        for(unsigned int i=0;i<mVertices.size();i++)delete mVertices[i];
        mVertices.clear();
        for(unsigned int i=0;i<mNormals.size();i++)delete mNormals[i];
        mNormals.clear();
        for(unsigned int i=0;i<mFaces.size();i++)delete mFaces[i];
        mFaces.clear();

        //std::string comments;
        //std::string token;
        char comments[256];
        char token[128];
        float x,y,z;
        int p1,p2,p3;
        int n1,n2,n3;
        while(in>>token){
            if(strcmp(token,"#")==0){
                in.getline(comments,256);
            }
            else if(strcmp(token,"v")==0){
                in>>x>>y>>z;
                mVertices.push_back(new STVertex(x,y,z));
            }
            else if(strcmp(token,"vn")==0){
                in>>x>>y>>z;
                mNormals.push_back(new STVector3(x,y,z));
                mSimpleMesh=false;
            }
            else if(strcmp(token,"f")==0){
                if(mSimpleMesh){
                    in>>p1; in.ignore(100, ' ');
                    in>>p2; in.ignore(100, ' ');
                    in>>p3; 
                    mFaces.push_back(new STFace(mVertices[p1-1],mVertices[p2-1],mVertices[p3-1]));
                }
                else{
                    in>>p1; in.ignore(100, '/'); in.ignore(100, '/'); in>>n1;
                    in>>p2; in.ignore(100, '/'); in.ignore(100, '/'); in>>n2;
                    in>>p3; in.ignore(100, '/'); in.ignore(100, '/'); in>>n3;
                    mFaces.push_back(new STFace(mVertices[p1-1],mVertices[p2-1],mVertices[p3-1],
                                mNormals[n1-1],mNormals[n2-1],mNormals[n3-1]));
                }
            }
        }

        std::cout << "Finish loading mesh" << std::endl;
        return true;
    }
    else {
        fprintf(stderr,
            "STTriangleMesh::STTriangleMesh() - Unknown file type \"%s\".\n",
            filename.c_str());
        return false;
    }
}

//
// Write the triangle mesh to files.
//
unsigned int getIndex(std::vector<STVertex*>& mVertices, const STVertex* v){
    return std::find(mVertices.begin(),mVertices.end(),v)-mVertices.begin()+1;
}

bool STTriangleMesh::Write(const std::string& filename)
{
    // Determine the right routine based on the file's extension.
    // The format-specific subroutines are each implemented in
    // a different file.
    std::string ext = STGetExtension( filename );
    if (ext.compare("OBJ") == 0){
        std::ofstream out( filename.c_str(), std::ios::out );

        if( !out ){
            std::cout << "cannot open file" << filename << std::endl;
            return false;
        }

        for(unsigned int i=0;i<mVertices.size();i++){
            out<<"v "<<mVertices[i]->pt.x<<" "<<mVertices[i]->pt.y<<" "<<mVertices[i]->pt.z<<" "<<std::endl;
        }
        for(unsigned int i=0;i<mFaces.size();i++){
            out<<"f "<<getIndex(mVertices,mFaces[i]->v[0])<<" "<<getIndex(mVertices,mFaces[i]->v[1])<<" "<<getIndex(mVertices,mFaces[i]->v[2])<<" "<<std::endl;
        }

        return true;
    }
    else {
        fprintf(stderr,
            "STTriangleMesh::STTriangleMesh() - Unknown file type \"%s\".\n",
            filename.c_str());
        return false;
    }
}

//
// Build topology  and calculate normals for the triangle mesh.
//
bool STTriangleMesh::Build()
{
    BuildTopology();
    CalculateNormal();
    return true;
}


bool STTriangleMesh::BuildTopology()
{
    // this function only works for maniford mesh
    if(!mSimpleMesh) return false;
    //Build topology
    std::map<std::pair<STVertex*,STVertex*>,STFace*> he2f;
    for(unsigned int i=0;i<mFaces.size();i++){
        for(unsigned int j=0;j<3;j++){
            mFaces[i]->v[j]->f=mFaces[i];
            he2f.insert(std::pair<std::pair<STVertex*,STVertex*>,STFace*>(std::pair<STVertex*,STVertex*>(mFaces[i]->v[j],mFaces[i]->v[(j+1)%3]),mFaces[i]));
        }
    }
    for(unsigned int i=0;i<mFaces.size();i++){
        for(unsigned int j=0;j<3;j++){
            std::map<std::pair<STVertex*,STVertex*>,STFace*>::iterator itr=he2f.find(std::pair<STVertex*,STVertex*>(mFaces[i]->v[(j+2)%3],mFaces[i]->v[(j+1)%3]));
            if(itr!=he2f.end())
                mFaces[i]->adjF[j]=itr->second;
            else
                mFaces[i]->adjF[j]=0;
        }
    }
    return true;
}

bool STTriangleMesh::CalculateNormal()
{
    //Calculate normals
    for(unsigned int i=0;i<mFaces.size();i++){
        STFace* face=mFaces[i];
        face->normal=STVector3::Cross(face->v[0]->pt-face->v[1]->pt,face->v[0]->pt-face->v[2]->pt);
        if(mSimpleMesh)
            for(unsigned int j=0;j<3;j++)
                face->v[j]->normal+=face->normal;
    }
    for(unsigned int i=0;i<mFaces.size();i++){
        mFaces[i]->normal.Normalize();
    }
    if(mSimpleMesh){
        for(unsigned int i=0;i<mVertices.size();i++){
            mVertices[i]->normal.Normalize();
        }
    }
    return true;
}

STFace* STTriangleMesh::NextAdjFace(STVertex *v, STFace *f)
{
    if( v == f->v[0] ) 
        return f->adjF[1];
    else if( v == f->v[1] ) 
        return f->adjF[2];
    else if( v == f->v[2] ) 
        return f->adjF[0];
    else 
        return 0;
}

STFace* STTriangleMesh::NextAdjFaceReverse(STVertex *v, STFace *f)
{
    if( v == f->v[0] ) 
        return f->adjF[2];
    else if( v == f->v[1] ) 
        return f->adjF[0];
    else if( v == f->v[2] ) 
        return f->adjF[1];
    else 
        return 0;
}

void STTriangleMesh::LoopSubdivide()
{
    if(!mSimpleMesh) return;
    unsigned int newVerticesStart=mVertices.size();

    // Add Odd Vertices
    std::map<STFace*,std::vector<STVertex *> > oldFaces2newVertices;
    std::vector<STVertex *> empty_vertices_3;for(unsigned int i=0;i<3;i++)empty_vertices_3.push_back(0);
    for(unsigned int i=0;i<mFaces.size();i++){
        oldFaces2newVertices.insert(std::pair<STFace*,std::vector<STVertex *> >(mFaces[i],empty_vertices_3) );
    }
    for(unsigned int i=0;i<mFaces.size();i++){
        for(unsigned int j=0;j<3;j++){
            if(oldFaces2newVertices[mFaces[i]][j]==0){
                if(mFaces[i]->adjF[j]!=0){
                    int adjF_j;
                    for(unsigned int k=0;k<3;k++){
                        if(mFaces[i]->adjF[j]->adjF[k]==mFaces[i]){
                            adjF_j=k;
                            break;
                        }
                    }
                    STPoint3 newPoint=(mFaces[i]->v[(j+1)%3]->pt+mFaces[i]->v[(j+2)%3]->pt)*0.375f
                        +(mFaces[i]->v[j]->pt+mFaces[i]->adjF[j]->v[adjF_j]->pt)*0.125f;
                    STVertex* newVertex=new STVertex(newPoint);
                    mVertices.push_back(newVertex);
                    oldFaces2newVertices[mFaces[i]][j]=oldFaces2newVertices[mFaces[i]->adjF[j]][adjF_j]=newVertex;
                }
                else{
                    STPoint3 newPoint=(mFaces[i]->v[(j+1)%3]->pt+mFaces[i]->v[(j+2)%3]->pt)*0.5f;
                    STVertex* newVertex=new STVertex(newPoint);
                    mVertices.push_back(newVertex);
                    oldFaces2newVertices[mFaces[i]][j]=newVertex;
                }
            }
        }
    }

    std::vector<STPoint3> newEvenVerticesPoints;newEvenVerticesPoints.resize(newVerticesStart);
    // Adjust Even Vertices
    for(unsigned int i=0;i<newVerticesStart;i++){
        STVertex* vertex=mVertices[i];
        STFace* nextface=vertex->f;
        std::vector<STPoint3> neighborPoints;
        bool boundary=false;
        do {
            if(nextface==0){
                boundary=true;
                break;
            }
            for(int j=0;j<3;j++){
                if(nextface->v[j]==vertex){
                    neighborPoints.push_back(nextface->v[(j+2)%3]->pt);
                    break;
                }
            }
        } while((nextface=NextAdjFace(vertex,nextface))!=vertex->f);

        if(boundary){
            STPoint3 temp=neighborPoints.back();
            neighborPoints.clear();
            neighborPoints.push_back(temp);
            nextface=vertex->f;
            do {
                if(nextface==0)
                    break;
                for(int j=0;j<3;j++){
                    if(nextface->v[j]==vertex){
                        temp=nextface->v[(j+1)%3]->pt;
                        break;
                    }
                }
            } while((nextface=NextAdjFaceReverse(vertex,nextface))!=vertex->f);
            neighborPoints.push_back(temp);
        }

        if(neighborPoints.size()>3){
            float weight=3.0f/8.0f/(float)neighborPoints.size();
            newEvenVerticesPoints[i]=vertex->pt*(5.0f/8.0f);
            for(unsigned j=0;j<neighborPoints.size();j++)
                newEvenVerticesPoints[i]=newEvenVerticesPoints[i]+neighborPoints[j]*weight;
        }
        else if(neighborPoints.size()==3){
            float weight=3.0f/16.0f;
            newEvenVerticesPoints[i]=vertex->pt*(7.0f/16.0f);
            for(unsigned j=0;j<neighborPoints.size();j++)
                newEvenVerticesPoints[i]=newEvenVerticesPoints[i]+neighborPoints[j]*weight;
        }
        else{ // assert(neighborPoints.size()==2) boundary vertex
            newEvenVerticesPoints[i]=vertex->pt*0.75f+neighborPoints[0]*0.125f+neighborPoints[1]*0.125f;
        }
    }
    for(unsigned int i=0;i<newVerticesStart;i++)
        mVertices[i]->pt=newEvenVerticesPoints[i];

    // Rebuild faces
    std::vector<STFace*> newFaces;
    for(unsigned int i=0;i<mFaces.size();i++){
        for(unsigned int j=0;j<3;j++){
            STFace* newFace=new STFace(mFaces[i]->v[j],oldFaces2newVertices[mFaces[i]][(j+2)%3],oldFaces2newVertices[mFaces[i]][(j+1)%3]);
            newFaces.push_back(newFace);
        }
        STFace* newFace=new STFace(oldFaces2newVertices[mFaces[i]][0],oldFaces2newVertices[mFaces[i]][1],oldFaces2newVertices[mFaces[i]][2]);
        newFaces.push_back(newFace);
    }
    std::swap(mFaces,newFaces);
    for(unsigned int i=0;i<newFaces.size();i++)delete newFaces[i];
    
    Build();
}

unsigned int STTriangleMesh::AddVertex(float x, float y, float z)
{
    mVertices.push_back(new STVertex(STPoint3(x,y,z)));
    return mVertices.size()-1;
}

unsigned int STTriangleMesh::AddVertex(const STPoint3& pt)
{
    mVertices.push_back(new STVertex(pt));
    return mVertices.size()-1;
}

unsigned int STTriangleMesh::AddFace(unsigned int id0,unsigned int id1,unsigned int id2)
{
    mFaces.push_back(new STFace(mVertices[id0],mVertices[id1],mVertices[id2]));
    return mFaces.size()-1;
}

std::ostream& operator <<(std::ostream& stream, const STVertex& v) {
    stream<<"["<<v.pt.x<<" "<<v.pt.y<<" "<<v.pt.z<<"]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const STFace& f) {
    stream<<"{"<<*(f.v[0])<<" "<<*(f.v[1])<<" "<<*(f.v[2])<<"}";
    return stream;
}
