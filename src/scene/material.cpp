#include "material.h"
#include "ray.h"
#include "light.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

#include "../fileio/bitmap.h"
#include "../fileio/pngimage.h"

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene *scene, const ray& r, const isect& i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	//	if( debugMode )
	//		std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( vector<Light*>::const_iterator litr = scene->beginLights(); 
	// 		litr != scene->endLights(); 
	// 		++litr )
	// {
	// 		Light* pLight = *litr;
	// 		.
	// 		.
	// 		.
	// }

    glm::dvec3 diffuse = glm::dvec3(0.0,0.0,0.0);
    glm::dvec3 specular = glm::dvec3(0.0,0.0,0.0);

    glm::dvec3 I_l = glm::dvec3(0.0,0.0,0.0);
    glm::dvec3 atten = glm::dvec3(0.0,0.0,0.0);

    glm::dvec3 L = glm::dvec3(0.0,0.0,0.0); 

    glm::dvec3 R = glm::dvec3(0.0,0.0,0.0);

    glm::dvec3 intersection_point = r.p + i.t * r.d;
    glm::dvec3 V =  glm::normalize(intersection_point - scene->getCamera().getEye());//-1.0 * r.d;//scene->getCamera().getLook();// -1.0 * r.d; // TODO: check this calculation , multiply by -1?

    vector<Light*>::const_iterator it;
    for (it = scene->beginLights(); it != scene->endLights(); it++) {
        I_l = (*it)->getColor();
        ray shadow(intersection_point, (*it)->getDirection(intersection_point), r.getPixel() , r.ctr, r.getAtten(),ray::SHADOW);
        atten = (*it)->distanceAttenuation(intersection_point) * (*it)->shadowAttenuation(shadow, intersection_point);

        L = (*it)->getDirection(intersection_point); // TODO investigate getDirection arg
        diffuse += atten * kd(i) * I_l * max(0.0, glm::dot(i.N, L));

        R = glm::normalize(L-(2.0 * glm::dot(i.N, L) * i.N));// normalize?
        specular += atten * ks(i) * I_l * pow(max(0.0, glm::dot(R, V)), shininess(i));
    }

    glm::dvec3 colorC = ke(i) + ka(i)*scene->ambient() + diffuse + specular;
    return colorC;
}

TextureMap::TextureMap( string filename ) {

	int start = (int) filename.find_last_of('.');
	int end = (int) filename.size() - 1;
	if (start >= 0 && start < end) {
		string ext = filename.substr(start, end);
		if (!ext.compare(".png")) {
			png_cleanup(1);
			if (!png_init(filename.c_str(), width, height)) {
				double gamma = 2.2;
				int channels, rowBytes;
				unsigned char* indata = png_get_image(gamma, channels, rowBytes);
				int bufsize = rowBytes * height;
				data = new unsigned char[bufsize];
				for (int j = 0; j < height; j++)
					for (int i = 0; i < rowBytes; i += channels)
						for (int k = 0; k < channels; k++)
							*(data + k + i + j * rowBytes) = *(indata + k + i + (height - j - 1) * rowBytes);
				png_cleanup(1);
			}
		}
		else
			if (!ext.compare(".bmp")) data = readBMP(filename.c_str(), width, height);
			else data = NULL;
	} else data = NULL;
	if (data == NULL) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

glm::dvec3 TextureMap::getMappedValue( const glm::dvec2& coord ) const
{
	// YOUR CODE HERE
	// 
	// In order to add texture mapping support to the 
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.
	//printf("getMappedValue");
	double a = coord[0] * width;
	double b = coord[1] * height;

	int i = (int) floor(a);
	int j = (int) floor(b);

	double x_delta = a - i;
	double y_delta = b - j;
	
	return (1-x_delta)*(1-y_delta)*getPixelAt(i,j)
			+ x_delta*(1-y_delta)*getPixelAt(i+1,j)
			+ (1-x_delta)*y_delta*getPixelAt(i,j+1)
			+ x_delta*y_delta*getPixelAt(i+1,j+1);
}


glm::dvec3 TextureMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return glm::dvec3(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return glm::dvec3(double(data[pos]) / 255.0, 
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0);
}

glm::dvec3 MaterialParameter::value( const isect& is ) const
{
    if( 0 != _textureMap )
        return _textureMap->getMappedValue( is.uvCoordinates );
    else
        return _value;
}

double MaterialParameter::intensityValue( const isect& is ) const
{
    if( 0 != _textureMap )
    {
        glm::dvec3 value( _textureMap->getMappedValue( is.uvCoordinates ) );
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    }
    else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}

