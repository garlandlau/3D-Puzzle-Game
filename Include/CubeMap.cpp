// CubeMap.cpp

#include "CubeMap.h"
#include "IO.h"
#include "GLXtras.h"

void CubeMap::Read(const char *filename, int texUnit) {
	textureName = ReadTexture(filename, false);
//	textureName = LoadTexture(filename, texUnit, false);
	if (!textureName) printf("can't open %s\n", filename);
	float	l = -1, r = 1, b = -1, t = 1, n = -1, f = 1;
	vec3	p[8] = {{l,b,n}, {l,b,f}, {l,t,n}, {l,t,f}, {r,b,n}, {r,b,f}, {r,t,n}, {r,t,f}};
	enum	{lbn=0, lbf, ltn, ltf, rbn, rbf, rtn, rtf};
	vec3	pts[6][4] = {{p[ltf],  p[ltn],  p[lbn],  p[lbf]},   // left face
						 {p[ltn],  p[rtn],  p[rbn],  p[lbn]},   // near
						 {p[rtn],  p[rtf],  p[rbf],  p[rbn]},   // right
						 {p[rtf],  p[ltf],  p[lbf],  p[rbf]},   // far
						 {p[ltf],  p[rtf],  p[rtn],  p[ltn]},   // top
						 {p[lbn],  p[rbn],  p[rbf],  p[lbf]}};  // bottom
	float	u0 = 0, u1 = .25f, u2 = .5f, u3 = .75f, u4 = 1, v0 = 0, v1 = 1.f/3, v2 = 2.f/3, v3 = 1;
	vec2	uvs[6][4] = {{{u0,v2}, {u1,v2}, {u1,v1}, {u0,v1}},  // left
						 {{u1,v2}, {u2,v2}, {u2,v1}, {u1,v1}},  // near
						 {{u2,v2}, {u3,v2}, {u3,v1}, {u2,v1}},  // right
						 {{u3,v2}, {u4,v2}, {u4,v1}, {u3,v1}},  // far
						 {{u1,v3}, {u2,v3}, {u2,v2}, {u1,v2}},  // top
						 {{u1,v1}, {u2,v1}, {u2,v0}, {u1,v0}}}; // bottom
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pts)+sizeof(uvs), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pts), pts);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pts), sizeof(uvs), uvs);
}

void CubeMap::Display(Camera camera) {
	const char *vertexShader = R"(
		#version 130
		in vec3 point;
		in vec2 uv;
		out vec2 vuv;
		uniform mat4 view;
		void main() { gl_Position = view*vec4(point, 1); vuv = uv; }
	)";
	const char *pixelShader = R"(
		#version 330
		in vec2 vuv;
		out vec4 color;
		uniform sampler2D textureImage;
		void main() { color = vec4(texture(textureImage, vuv).rgb, 1); }
	)";
	if (!shaderProgram) shaderProgram = LinkProgramViaCode(&vertexShader, &pixelShader);
	glUseProgram(shaderProgram);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	VertexAttribPointer(shaderProgram, "point", 3, 0, (void *) 0);
	VertexAttribPointer(shaderProgram, "uv", 2, 0, (void *) (24*sizeof(vec3)));
	glActiveTexture(GL_TEXTURE0+textureName);
	glBindTexture(GL_TEXTURE_2D, textureName);
	SetUniform(shaderProgram, "view", camera.fullview*transform);
	SetUniform(shaderProgram, "textureImage", (int) textureName);
	glDrawArrays(GL_QUADS, 0, 24);
}

