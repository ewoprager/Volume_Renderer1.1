#include "Render.h"

#include "Plot.h"

namespace Plot {

static const int memrySize = 100;
static float memry[memrySize];
static float latest = 1.0f;
float Latest(){ return latest; }
static float memryXs[memrySize];
static const int nMemryIs = 2*(memrySize - 1);
static GLuint memryIs[nMemryIs];
static const float memryRect[4] = {0.0f, 0.6f, 0.95f, 0.95f};
void InitMemry(){
	for(int i=0; i<memrySize; i++){
		memry[i] = memryRect[1];
		memryXs[i] = memryRect[0] + (float)i/(float)(memrySize - 1)*(memryRect[2] - memryRect[0]);
	}
	for(int i=0; i<memrySize - 1; i++){
		memryIs[2*i] = i;
		memryIs[2*i + 1] = i + 1;
	}
}
void UpdateMemry(float newVal){
	latest = newVal;
	memcpy(memry, &memry[1], (memrySize - 1)*sizeof(float));
	memry[memrySize - 1] = memryRect[1] + newVal*(memryRect[3] - memryRect[1]);
}
void DrawMemry(){
	// the plotted line
	glUseProgram(PROGRAM(Render::SP::plot));
	glUniform1f(UNIFORM(Render::SP::plot, Render::U_plot::depth), 0.0);
	
	glBindBuffer(GL_ARRAY_BUFFER, Render::VBO(0));
	glVertexAttribPointer(ATTRIBUTE(Render::SP::plot, Render::A_plot::xPos), 2, GL_FLOAT, GL_FALSE, 1*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(Render::SP::plot, Render::A_plot::xPos));
	glBufferData(GL_ARRAY_BUFFER, memrySize*sizeof(float), memryXs, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, Render::VBO(1));
	glVertexAttribPointer(ATTRIBUTE(Render::SP::plot, Render::A_plot::yPos), 2, GL_FLOAT, GL_FALSE, 1*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(Render::SP::plot, Render::A_plot::yPos));
	glBufferData(GL_ARRAY_BUFFER, memrySize*sizeof(float), memry, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render::VBO(2));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nMemryIs*sizeof(GLuint), memryIs, GL_STATIC_DRAW);
	glDrawElements(GL_LINES, nMemryIs, GL_UNSIGNED_INT, 0);
	
	// the axes
	glUseProgram(PROGRAM(Render::SP::axes));
	glUniform1f(UNIFORM(Render::SP::axes, Render::U_axes::depth), 0.0);
	
	static const float positions[3*3] = {
		memryRect[0], memryRect[1], 0.0f,
		memryRect[2], memryRect[1], 0.0f,
		memryRect[0], memryRect[3], 0.0f
	};
	static const float colours[3*4] = {
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f
	};
	static const GLuint indices[4] = {
		0, 1,
		0, 2
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, Render::VBO(0));
	glVertexAttribPointer(ATTRIBUTE(Render::SP::axes, Render::A_axes::position), 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(Render::SP::axes, Render::A_axes::position));
	glBufferData(GL_ARRAY_BUFFER, 3*3*sizeof(float), positions, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, Render::VBO(1));
	glVertexAttribPointer(ATTRIBUTE(Render::SP::axes, Render::A_axes::colour), 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(ATTRIBUTE(Render::SP::axes, Render::A_axes::colour));
	glBufferData(GL_ARRAY_BUFFER, 3*4*sizeof(float), colours, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render::VBO(2));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(GLuint), indices, GL_STATIC_DRAW);
	glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, 0);
}

}
