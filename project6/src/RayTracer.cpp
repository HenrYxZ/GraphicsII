// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>

using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
Vec3d RayTracer::trace( double x, double y )
{
  // Clear out the ray cache in the scene for debugging purposes,
  scene->intersectCache.clear();

  ray r( Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY );

  scene->getCamera().rayThrough( x,y,r );
  Vec3d ret = traceRay( r, Vec3d(1.0,1.0,1.0), 0 );
  ret.clamp();
  return ret;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay( const ray& r, const Vec3d& thresh, int depth )
{

  if (depth == 5)
    return Vec3d( 0.0, 0.0, 0.0 );

  ++depth;

  isect i;

  if( scene->intersect( r, i ) ) {
    // YOUR CODE HERE

    // An intersection occured!  We've got work to do.  For now,
    // this code gets the material for the surface that was intersected,
    // and asks that material to provide a color for the ray.  

    // This is a great place to insert code for recursive ray tracing.
    // Instead of just returning the result of shade(), add some
    // more steps: add in the contributions from reflected and refracted
    // rays.


    const Material& m = i.getMaterial();

    Vec3d iPoint = r.at(i.t);  // point of intersection

    // --Shadows--
    Vec3d shadow = Vec3d(1.0, 1.0, 1.0);
    //for (vector<Light*>::const_iterator litr = scene->beginLights();
    //  litr != scene->endLights(); ++litr )
    //{
      vector<Light*>::const_iterator litr = scene->beginLights();
      Light* pLight = *litr;
      Vec3d shadowDir = pLight->getDirection(iPoint);
      isect aux;
      ray shadow_ray (iPoint,shadowDir,ray::SHADOW);
      if(scene->intersect(shadow_ray, aux)) {
        const Material& mm = aux.getMaterial();
        shadow = prod(shadow, mm.kt(aux));
      }
    //}

    // --Reflection--
    Vec3d reflection = Vec3d(0.0,0.0,0.0);
    if (m.kr(i) != Vec3d(0.0,0.0,0.0)) {
      Vec3d reflecDir = 2*((-1*r.getDirection()*i.N)*i.N) + r.getDirection();
      reflecDir.normalize();
      ray ReflRay (iPoint, reflecDir, ray::REFLECTION);
      reflection = traceRay(ReflRay, Vec3d(1.0,1.0,1.0), depth);
      
      // use threshold
      for(int i=0; i < 3; i++){
        if(reflection[i] > thresh[i]){
          reflection[i] = 1;
        }
      }
        
    }

    // --Refraction--
    Vec3d refraction = Vec3d(0.0,0.0,0.0);
    if (m.kt(i) != Vec3d(0.0,0.0,0.0)) {
      double IofR = 1.0 / m.index(i);  // the index of refraction
      Vec3d inorm = i.N;
      Vec3d d = -1.0*r.getDirection();  // reverse the ray direction for calculations
      double cos_i = inorm*d;
      // if the ray is inside of the object
      if (cos_i < 0.0) {
       IofR = m.index(i);
       cos_i *= -1.0;
       inorm *= -1.0;
      }
      double cos_t = 1.0 - (IofR*IofR)*(1.0 - (cos_i*cos_i));
      // if not Total Internal Reflection
      if (cos_t >= 0.0) {
        Vec3d refractDir = (IofR*cos_i - sqrt(cos_t))*inorm - IofR*d;
        refractDir.normalize();
        ray RefrRay (iPoint, refractDir, ray::REFRACTION);
        refraction = traceRay(RefrRay, Vec3d(1.0,1.0,1.0), depth);
        
        // use threshold
        for(int i=0; i < 3; i++){
          if(refraction[i] > thresh[i]){
            refraction[i] = 1;
          }
        }
      }
    }

    return prod(m.shade(scene, r, i), shadow) + prod(m.kr(i), reflection) + prod(m.kt(i), refraction);

	
  } else {
    // No intersection.  This ray travels to infinity, so we color
    // it according to the background color, which in this (simple) case
    // is just black.

    return Vec3d( 0.0, 0.0, 0.0 );
  }
}

RayTracer::RayTracer()
    : scene( 0 ), buffer( 0 ), buffer_width( 256 ), buffer_height( 256 ), m_bBufferReady( false )
{

}


RayTracer::~RayTracer()
{
  delete scene;
  delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
  buf = buffer;
  w = buffer_width;
  h = buffer_height;
}

double RayTracer::aspectRatio()
{
  return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn )
{
  ifstream ifs( fn );
  if( !ifs ) {
    string msg( "Error: couldn't read scene file " );
    msg.append( fn );
    traceUI->alert( msg );
    return false;
  }
	
  // Strip off filename, leaving only the path:
  string path( fn );
  if( path.find_last_of( "\\/" ) == string::npos )
    path = ".";
  else
    path = path.substr(0, path.find_last_of( "\\/" ));

  // Call this with 'true' for debug output from the tokenizer
  Tokenizer tokenizer( ifs, false );
  Parser parser( tokenizer, path );
  try {
    delete scene;
    scene = 0;
    scene = parser.parseScene();
  } 
  catch( SyntaxErrorException& pe ) {
    traceUI->alert( pe.formattedMessage() );
    return false;
  }
  catch( ParserException& pe ) {
    string msg( "Parser: fatal exception " );
    msg.append( pe.message() );
    traceUI->alert( msg );
    return false;
  }
  catch( TextureMapException e ) {
    string msg( "Texture mapping exception: " );
    msg.append( e.message() );
    traceUI->alert( msg );
    return false;
  }


  if( ! sceneLoaded() )
    return false;

  return true;
}

void RayTracer::traceSetup( int w, int h )
{
  if( buffer_width != w || buffer_height != h )
  {
    buffer_width = w;
    buffer_height = h;

    bufferSize = buffer_width * buffer_height * 3;
    delete [] buffer;
    buffer = new unsigned char[ bufferSize ];


  }
  memset( buffer, 0, w*h*3 );
  m_bBufferReady = true;
}

void RayTracer::tracePixel( int i, int j )
{
  Vec3d col;

  if( ! sceneLoaded() )
    return;

  double x = double(i)/double(buffer_width);
  double y = double(j)/double(buffer_height);


  col = trace( x,y );

  unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

  pixel[0] = (int)( 255.0 * col[0]);
  pixel[1] = (int)( 255.0 * col[1]);
  pixel[2] = (int)( 255.0 * col[2]);
}


