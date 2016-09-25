#include <cmath>
#include <float.h>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include "trimesh.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const glm::dvec3 &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const glm::dvec3 &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt ) return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    if (!newFace->degen) faces.push_back( newFace );


    // Don't add faces to the scene's object list so we can cull by bounding box
    // scene->add(newFace);
    return true;
}

const char* Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	typedef Faces::const_iterator iter;
	bool have_one = false;
	for( iter j = faces.begin(); j != faces.end(); ++j )
	{
		isect cur;
		if( (*j)->intersectLocal( r, cur ) )
		{
			if( !have_one || (cur.t < i.t) )
			{
				i = cur;
				have_one = true;
			}
		}
	}
	if( !have_one ) i.setT(1000.0);
	return have_one;
} 

bool TrimeshFace::intersect(ray& r, isect& i) const {
  return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
    // YOUR CODE HERE
    glm::dvec3 a_coords = parent->vertices[ids[0]];
    glm::dvec3 b_coords = parent->vertices[ids[1]];
    glm::dvec3 c_coords = parent->vertices[ids[2]];

    // SMOOTH SHADING
    if (parent->normals.size() == 0) {
        parent->generateNormals();
    }

    glm::dvec3 a_normal = parent->normals[ids[0]];
    glm::dvec3 b_normal = parent->normals[ids[1]];
    glm::dvec3 c_normal = parent->normals[ids[2]];
    // /SMOOTH SHADING
    
    glm::dvec3 d_normalized = glm::normalize(r.d);

    if (glm::dot(normal, d_normalized) == 0 ) return false;
    double d = glm::dot(normal, a_coords);
    double t = (d - glm::dot(normal, r.p)) / (glm::dot(normal, d_normalized));
    if (t < 0.00001) return false;
    glm::dvec3 q = r.p + t * d_normalized; 
    

    if (glm::dot(glm::cross(b_coords - a_coords, q - a_coords), normal) >= 0 
        && glm::dot(glm::cross(c_coords - b_coords, q - b_coords), normal) >= 0
        && glm::dot(glm::cross(a_coords - c_coords, q - c_coords), normal) >= 0) {
        // compute barycentric coordinates
        double alpha = glm::dot(glm::cross(c_coords - b_coords, q - b_coords), normal)
            / glm::dot(glm::cross(b_coords - a_coords, c_coords - a_coords), normal);

        double beta = glm::dot(glm::cross(a_coords - c_coords, q - c_coords), normal)
            / glm::dot(glm::cross(b_coords - a_coords, c_coords - a_coords), normal);

        double gamma = glm::dot(glm::cross(b_coords - a_coords, q - a_coords), normal)
            / glm::dot(glm::cross(b_coords - a_coords, c_coords - a_coords), normal);

        // SMOOTH SHADING
        glm::dvec3 interpolated_normal = glm::normalize(alpha * a_normal + beta * b_normal + gamma * c_normal);

        i.setUVCoordinates(glm::dvec2(alpha, beta));
        i.setBary(q);
        i.setT(t);
        //i.setN(normal);
        i.setN(interpolated_normal);
        i.setObject(this);
        i.setMaterial(this->getMaterial());
        return true;
    } 
    else {
       return false;
    } 
    // FIXME: Add ray-trimesh intersection

}

void Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
		glm::dvec3 faceNormal = (**fi).getNormal();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
    vertNorms = true;
}

