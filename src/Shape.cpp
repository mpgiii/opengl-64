#include "Shape.h"
#include <iostream>
#include <assert.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
   vaoID(0)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
	texOff = false;
}

Shape::~Shape()
{
}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
		posBuf = shape.mesh.positions;
		norBuf = shape.mesh.normals;
		texBuf = shape.mesh.texcoords;
		eleBuf = shape.mesh.indices;
}

void Shape::measure() {
  float minX, minY, minZ;
   float maxX, maxY, maxZ;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
   max.x = maxX;
   max.y = maxY;
   max.z = maxZ;
}

void Shape::createNorm() {
	size_t i;
	size_t i1, i2, i3;

	glm::vec3 v1, v2;
	glm::vec3 p1, p2, p3;
	glm::vec3 norm;
	glm::vec3 temp1, temp2;

	// clear out the norBuf (just in case)
	for (i = 0; i < posBuf.size(); i++) {
		norBuf.push_back(0);
	}

	// for every face, compute the normal and add it to every adjacent vertex
	for (i = 0; i < eleBuf.size() / 3; i++) {
		// get the three indeces for the face's points
		i1 = eleBuf[3 * i + 0];
		i2 = eleBuf[3 * i + 1];
		i3 = eleBuf[3 * i + 2];

		// store the three points in p1, p2 and p3
		p1 = glm::vec3(posBuf[3 * i1 + 0], posBuf[3 * i1 + 1], posBuf[3 * i1 + 2]);
		p2 = glm::vec3(posBuf[3 * i2 + 0], posBuf[3 * i2 + 1], posBuf[3 * i2 + 2]);
		p3 = glm::vec3(posBuf[3 * i3 + 0], posBuf[3 * i3 + 1], posBuf[3 * i3 + 2]);

		// get two vectors along the face's edges
		v1 = p1 - p2;
		v2 = p2 - p3;

		// get the cross product of these two vectors
		norm = glm::cross(v1, v2);

		// add these values into the normBuf
		norBuf[3 * i1 + 0] += norm.x;
		norBuf[3 * i1 + 1] += norm.y;
		norBuf[3 * i1 + 2] += norm.z;

		norBuf[3 * i2 + 0] += norm.x;
		norBuf[3 * i2 + 1] += norm.y;
		norBuf[3 * i2 + 2] += norm.z;

		norBuf[3 * i3 + 0] += norm.x;
		norBuf[3 * i3 + 1] += norm.y;
		norBuf[3 * i3 + 2] += norm.z;
	}

	for (i = 0; i < norBuf.size() / 3; i++) {
		temp1.x = norBuf[3 * i + 0];
		temp1.y = norBuf[3 * i + 1];
		temp1.z = norBuf[3 * i + 2];

		temp2 = glm::normalize(temp1);

		norBuf[3 * i + 0] = temp2.x;
		norBuf[3 * i + 1] = temp2.y;
		norBuf[3 * i + 2] = temp2.z;
	}
}



void Shape::init()
{
   // Initialize the vertex array object
   glGenVertexArrays(1, &vaoID);
   glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		norBufID = 0;
		createNorm();
	}
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	
	// Send the texture array to the GPU
	if(texBuf.empty() || texOff) {
		texBufID = 0;
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

   glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}
	
	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
