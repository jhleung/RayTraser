// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "scene/scene.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <string.h> // for memset

#include <iostream>
#include <fstream>

using namespace std;
extern TraceUI* traceUI;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = true;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

glm::dvec3 RayTracer::trace(double x, double y, unsigned char *pixel, unsigned int ctr)
{
    // Clear out the ray cache in the scene for debugging purposes,
  if (TraceUI::m_debug) scene->intersectCache.clear();

    ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), pixel, ctr, glm::dvec3(1,1,1), ray::VISIBILITY);
    scene->getCamera().rayThrough(x,y,r);
    double dummy;
    glm::dvec3 ret = traceRay(r, glm::dvec3(1.0,1.0,1.0), traceUI->getDepth() , dummy);
    ret = glm::clamp(ret, 0.0, 1.0);
    return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j, unsigned int ctr)
{
	glm::dvec3 col(0,0,0);

	if( ! sceneLoaded() ) return col;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;
	col = trace(x, y, pixel, ctr);

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	return col;
}

glm::dvec3 RayTracer::aaTracePixel(int i, int j, int samples, unsigned int ctr)
{
	glm::dvec3 col(0,0,0);

	if( ! sceneLoaded() ) return col;

	double x = double(i)/double(buffer_width * samples);
	double y = double(j)/double(buffer_height * samples);
	unsigned char *pixel = buffer + ( i + j * buffer_width) * 3;
	col = trace(x, y, pixel, ctr);

	// pixel[0] = (int)( 255.0 * col[0]);
	// pixel[1] = (int)( 255.0 * col[1]);
	// pixel[2] = (int)( 255.0 * col[2]);
	return col;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray& r, const glm::dvec3& thresh, int depth, double& t )
{
    // YOUR CODE HERE
    // getting color of intersection point

    // An intersection occurred!  We've got work to do.  For now,
    // this code gets the material for the surface that was intersected,
    // and asks that material to provide a color for the ray.  

    // This is a great place to insert code for recursive ray tracing.
    // Instead of just returning the result of shade(), add some
    // more steps: add in the contributions from reflected and refracted
    // rays.
    isect i;
    glm::dvec3 colorC;
    glm::dvec3 reflect; 
    glm::dvec3 refract;  
    glm::dvec3 d = glm::normalize(r.d);
    if(scene->intersect(r, i)) {
        const Material& m = i.getMaterial();
        glm::dvec3 intersection_point = r.p + i.t * r.d;
            //refration: keep track of incoming/outgoing
        colorC = m.shade(scene, r, i); // Phong
        if (depth > 0) {
            if (m.Refl()) {
            	glm::dvec3 R = r.d - 2.0 * glm::dot(r.d, i.N) * i.N;
            	ray refl(intersection_point, R, r.pixel, r.ctr, r.atten, ray::REFLECTION);
            	reflect += traceRay(refl, thresh, depth - 1, t) * m.kr(i);
            }

            if (m.Trans()) {
            	double index_refr;
            	glm::dvec3 normal;
            	//If two vectors point along the same 'general direction', their dot product is positive
            	if (glm::dot(d, i.N) > 0) {
            		normal = -1.0 * i.N;
            		index_refr = m.index(i) / 1.0029;
            	} 
            	else {
            		normal = i.N;
            		index_refr = 1.0029 / m.index(i);
            	}

            	// check for TIR
        		if ((1.0 - pow(index_refr,2) * (1.0 - pow(glm::dot(i.N, -1.0 * r.d), 2))) < 0) {
        			refract += colorC;
        		} 
        		else {
            		glm::dvec3 R = glm::refract(r.d, normal, index_refr);
	              	ray refr(intersection_point, R, r.pixel, r.ctr, r.atten, ray::REFRACTION);
            		refract += traceRay(refr, thresh, depth - 1, t) * m.kt(i);
        		}
            }
        }
    } else {
        // No intersection.  This ray travels to infinity, so we color
        // it according to the background color, which in this (simple) case
        // is just black.
        // 
        // FIXME: Add CubeMap support here.
        if (getCubeMap()) {
        	// cout << "cubemap exists in raytracer" << endl;
    		colorC = cubemap->getColor(r);
    	}
    	else
        	colorC = glm::dvec3(0.0, 0.0, 0.0);
    }
    return colorC + reflect + refract;
}

RayTracer::RayTracer()
	: scene(0), buffer(0), thresh(0), buffer_width(256), buffer_height(256), m_bBufferReady(false), cubemap (0)
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

bool RayTracer::loadScene( char* fn ) {
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos ) path = ".";
	else path = path.substr(0, path.find_last_of( "\\/" ));

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

	if( !sceneLoaded() ) return false;

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	if (buffer_width != w || buffer_height != h)
	{
		buffer_width = w;
		buffer_height = h;
		bufferSize = buffer_width * buffer_height * 3;
		delete[] buffer;
		buffer = new unsigned char[bufferSize];
	}
	memset(buffer, 0, w*h*3);
	m_bBufferReady = true;
}

void RayTracer::traceImage(int w, int h, int bs, double thresh) // TODO: investigate vs/thresh
{
	// YOUR CODE HERE
	// FIXME: Start one or more threads for ray tracing
        traceSetup(w,h);
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                tracePixel(i,j,0); // TODO: investigate 3rd param ctr
            }
        }
}

int RayTracer::aaImage(int samples, double aaThresh)
{
	// YOUR CODE HERE
	// FIXME: Implement Anti-aliasing here
	traceSetup(buffer_width, buffer_height);
  
	for (int i = 0; i < buffer_width; i++) {
	    for (int j = 0; j < buffer_height; j++) {
	    	glm::dvec3 total;
	    	for (int s = 0; s < samples; s++)
				for (int t = 0; t < samples; t++) {
					total += aaTracePixel(i*samples + s, j*samples + t, samples, 0);
				}
			total /= (samples * samples);
			setPixel(i, j, total);
		} 	
	}
}

bool RayTracer::checkRender()
{
	// YOUR CODE HERE
	// FIXME: Return true if tracing is done.
	return true;
}

glm::dvec3 RayTracer::getPixel(int i, int j)
{
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;
	return glm::dvec3((double)pixel[0]/255.0, (double)pixel[1]/255.0, (double)pixel[2]/255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * color[0]);
	pixel[1] = (int)( 255.0 * color[1]);
	pixel[2] = (int)( 255.0 * color[2]);
}

