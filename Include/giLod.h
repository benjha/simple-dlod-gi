
#ifndef __GI_LOD_H__
#define __GI_LOD_H__

#include <GL/glew.h>
#include	"GI.h"
#include	"ModelObject.h"

class cGILod : public cGI
{
public:
	cGILod (int width, int height, float sightMag);
	~cGILod ();
	void	generateMipMap ();
	void	DisplayTextures();
	void	calcDistMipMap ();
	void	upDateCameraPosAndDir (float *camPos, float *camDir);
	
private:
	int mipMap_w, mipMap_h;
	FramebufferObject	*geometryImageRTTMipMap; //stores vertices (Front Buffer) & normals (Back Buffer)
	unsigned int mipMapTexId[2], mipMapDistTexId[3];
	FramebufferObject	*geometryImageRTTMipMapDist;
	ShaderObject *calcDistanceMipMap;
	ShaderObject *genLODSelectionImage;
	int lodLevels;
	float sightMagnitude, sightLOD;
	

};

#endif