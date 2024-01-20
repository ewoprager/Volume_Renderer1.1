#include "Tools.h"

unsigned long UTime(){
	timeval tv;
	gettimeofday(&tv, nullptr);
	return 1000000*(unsigned long)tv.tv_sec + (unsigned long)tv.tv_usec;
}

GLsizei GLTypeSize(const GLenum &glType){
	switch(glType){
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_HALF_FLOAT:
			return 2;
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
		case GL_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_FIXED:
			return 4;
		case GL_DOUBLE:
			return 8;
		default:
			std::cout << "ERROR: Unrecognised type given.\n";
			return 1;
	}
}

GLenum TypePLYtoGL(const char *name){
	if(!strcmp(name, "char") || !strcmp(name, "int8")){
		return GL_BYTE;
	} else if(!strcmp(name, "uchar") || !strcmp(name, "uint8")){
		return GL_UNSIGNED_BYTE;
	} else if(!strcmp(name, "short") || !strcmp(name, "int16")){
		return GL_SHORT;
	} else if(!strcmp(name, "ushort") || !strcmp(name, "uint16")){
		return GL_UNSIGNED_SHORT;
	} else if(!strcmp(name, "int") || !strcmp(name, "int32")){
		return GL_INT;
	} else if(!strcmp(name, "uint") || !strcmp(name, "uint32")){
		return GL_UNSIGNED_INT;
	} else if(!strcmp(name, "float") || !strcmp(name, "float32")){
		return GL_FLOAT;
	} else if(!strcmp(name, "double") || !strcmp(name, "float64")){
		return GL_DOUBLE;
	} else {
		std::cout << "ERROR: Unrecognised PLY type given.\n";
		return GL_BYTE;
	}
}

void Flip2DByteArrayVertically(uint8_t *array, int width, int height){
	static uint8_t buf[30000000];
	memcpy(buf, array, width*height);
	for(int j=0; j<height; j++) memcpy(&array[j*width], &buf[(height - j - 1)*width], width);
}

float Uniform(float lb, float ub, float prec){
	return lb + (float)(rand() % ((int)floor(prec*(ub - lb)) + 1))/prec;
}
