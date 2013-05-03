#include <cmath>
#include <float.h>
#include "trimesh.h"

using namespace std;

Vec3d crossProd(const Vec3d& a, const Vec3d& b){
  // get the normal of the triangle
  double n1 = a[1]*b[2] - a[2]*b[1];
  double n2 = a[2]*b[0] - a[0]*b[2];
  double n3 = a[0]*b[1] - a[1]*b[0];

  return Vec3d(n1,n2,n3);
}


Trimesh::~Trimesh()
{
  for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
    delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
  vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
  materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
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
  faces.push_back( newFace );
  return true;
}

char *
Trimesh::doubleCheck()
    // Check to make sure that if we have per-vertex materials or normals
    // they are the right number.
{
  if( !materials.empty() && materials.size() != vertices.size() )
    return "Bad Trimesh: Wrong number of materials.";
  if( !normals.empty() && normals.size() != vertices.size() )
    return "Bad Trimesh: Wrong number of normals.";

  return 0;
}

bool Trimesh::intersectLocal(const ray&r, isect&i) const
{
  double tmin = 0.0;
  double tmax = 0.0;
  typedef Faces::const_iterator iter;
  bool have_one = false;
  for( iter j = faces.begin(); j != faces.end(); ++j ) {
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

// Intersect ray r with the triangle abc.  If it hits returns true,
// and puts the t parameter, barycentric coordinates, normal, object id,
// and object material in the isect object
bool TrimeshFace::intersectLocal( const ray& r, isect& i ) const
{
  const Vec3d& a = parent->vertices[ids[0]];
  const Vec3d& b = parent->vertices[ids[1]];
  const Vec3d& c = parent->vertices[ids[2]];

  // tangent vectors
  Vec3d t1 = b - a;
  Vec3d t2 = c - a;
  
  Vec3d n = crossProd(t1,t2);

  double D = -n*a;

  // if the surface is parallel to the ray there is no intersection
  if(r.getDirection()*n == 0)
  {
    return false;
  }  

  double t = -(n*r.getPosition() + D)/(n*r.getDirection() );
  if (t <= RAY_EPSILON)
    return false;

  // point of intersection with the same plane (doesn't mean intersection with triangle) p(t)=p+t*d
  Vec3d p = r.at(t);

  // triangle area
  double A = n.length()/2.0;

  // barycentric coords
  double wa = crossProd(c-b, p-b).length() / (2.0*A);
  double wb = crossProd(a-c, p-c).length() / (2.0*A);
  double wc = crossProd(b-a, p-a).length() / (2.0*A);

  if((wa >= 0.0) && (wb >= 0.0) && (wc >= 0.0) && (wa+wb+wc-1.0 <= 0.00001)) {
    i.setT(t);
    i.setBary(wa, wb, wc);
    i.setN(n);
    i.setObject(this);
    i.setMaterial(this->getMaterial() );
    return true;
  }

  return false;
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
    Vec3d faceNormal = (**fi).getNormal();
        
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


