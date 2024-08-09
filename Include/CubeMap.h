// CubeMap.h

#ifndef CUBE_MAP_HDR
#define CUBE_MAP_HDR

#include "Mesh.h"

class CubeMap {
public:
	mat4 transform;
	unsigned int /*GLuint*/ textureName = 0, vertexBuffer = 0, shaderProgram = 0;
	int textureUnit = 0;
	void Read(const char *filename, int texUnit = 5);
	void Display(Camera camera);
	~CubeMap() { if (vertexBuffer) glDeleteBuffers(1, &vertexBuffer); }
};

#endif
