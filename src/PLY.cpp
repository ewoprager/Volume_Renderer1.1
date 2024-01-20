#include "TextRead.h"

#include "PLY.h"

// reads the contents of a PLY file into a `DrawELementArray` struct, allocating memory for the vertex and index arrays
Render::DrawElementArray *ReadPLY(const char *fileName, bool reverseWinding, bool flipNormals){
	int verticesN;
	int parametersN;
	Render::DrawParameter parameters[MAX_VS];
	int indicesN;
	static GLuint indexBuffer[1000000];
	
	TextRead tr;
	tr.BeginRead(fileName);
	
	char buf[30];
	char buf2[30];
	char buf3[30];
	
	while(!tr.CheckString("element vertex ")){
		if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	}
	
	if(tr.SkipNumber(15)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	if(tr.ReadUntil(buf, 30, tr.newLineString, true)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	if(!sscanf(buf, "%u", &verticesN)){ std::cout << "ERROR: Unable to read integer.\n"; return nullptr; }
	
	if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	GLsizei vertexSize = 0;
	
	parametersN = 0;
	while(tr.CheckString("property ")){
		if(tr.SkipNumber(9)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		if(tr.ReadUntil(buf, 30, tr.whiteSpaceHorizontalString, true)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		// type string in buf
		if(tr.SkipWhile(tr.whiteSpaceHorizontalString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		if(tr.ReadUntil(buf2, 30, tr.newLineString, true)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		// parameter name in buf2
		if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		// now on next line
		static const char property[10] = "property ";
		int i = 0;
		memcpy(buf3, property, 9); i += 9;
		memcpy(buf3 + i, buf, strlen(buf)); i += strlen(buf);
		buf3[i] = ' '; i += 1;
		
		bool success = false;
		if(!strcmp(buf2, "x")){
			success = true;
			parameters[parametersN].n = 1;
			buf3[i] = 'y'; buf3[i + 1] = '\0';
			if(tr.CheckString(buf3)){
				parameters[parametersN].n++;
				buf3[i] = 'z'; buf3[i + 1] = '\0';
				if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				if(tr.CheckString(buf3)){
					parameters[parametersN].n++;
					if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				}
			}
			parameters[parametersN].glType = TypePLYtoGL(buf);
		} else if(!strcmp(buf2, "nx")){
			success = true;
			parameters[parametersN].n = 1;
			buf3[i] = 'n'; buf3[i + 1] = 'y'; buf3[i + 2] = '\0';
			if(tr.CheckString(buf3)){
				parameters[parametersN].n++;
				buf3[i] = 'n'; buf3[i + 1] = 'z'; buf3[i + 2] = '\0';
				if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				if(tr.CheckString(buf3)){
					parameters[parametersN].n++;
					if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				}
			}
			parameters[parametersN].glType = TypePLYtoGL(buf);
		} else if(!strcmp(buf2, "red")){
			success = true;
			parameters[parametersN].n = 1;
			buf3[i] = 'g'; buf3[i + 1] = 'r'; buf3[i + 2] = 'e'; buf3[i + 3] = 'e'; buf3[i + 4] = 'n'; buf3[i + 5] = '\0';
			if(tr.CheckString(buf3)){
				parameters[parametersN].n++;
				buf3[i] = 'b'; buf3[i + 1] = 'l'; buf3[i + 2] = 'u'; buf3[i + 3] = 'e'; buf3[i + 4] = '\0';
				if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				if(tr.CheckString(buf3)){
					parameters[parametersN].n++;
					if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				}
			}
			parameters[parametersN].glType = TypePLYtoGL(buf);
		}
		
		if(success){
			vertexSize += parameters[parametersN].n*GLTypeSize(parameters[parametersN].glType);
			parameters[parametersN].normalised = parameters[parametersN].glType == GL_BYTE || parameters[parametersN].glType == GL_UNSIGNED_BYTE || parameters[parametersN].glType == GL_SHORT || parameters[parametersN].glType == GL_UNSIGNED_SHORT || parameters[parametersN].glType == GL_INT || parameters[parametersN].glType == GL_UNSIGNED_INT;
			parametersN++;
		}
	}
	
	// now have set: `verticesN`, `parameters` array, `parametersN`, `vertexSize`
	
	while(!tr.CheckString("element face ")){
		if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	}
	if(tr.SkipNumber(13)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	if(tr.ReadUntil(buf, 30, tr.newLineString, true)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	int facesN;
	if(!sscanf(buf, "%u", &facesN)){ std::cout << "ERROR: Unable to read integer.\n"; return nullptr; }
	
	if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	if(!tr.CheckString("property list ")){ std::cout << "ERROR: Face format not supported.\n"; return nullptr; }
	
	if(tr.SkipNumber(14)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	enum class tp { b, ub, s, us, i, ui };
	tp lengthType, contentsType;
	
	if(tr.ReadUntil(buf, 30, tr.whiteSpaceHorizontalString, true)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	// list length type in buf
	if(!strcmp(buf, "char") || !strcmp(buf, "int8")) lengthType = tp::b;
	else if(!strcmp(buf, "uchar") || !strcmp(buf, "uint8")) lengthType = tp::ub;
	else if(!strcmp(buf, "short") || !strcmp(buf, "int16")) lengthType = tp::s;
	else if(!strcmp(buf, "ushort") || !strcmp(buf, "uint16")) lengthType = tp::us;
	else if(!strcmp(buf, "int") || !strcmp(buf, "int32")) lengthType = tp::i;
	else if(!strcmp(buf, "uint") || !strcmp(buf, "uint32")) lengthType = tp::ui;
	else { std::cout << "ERROR: Unrecognised type.\n"; return nullptr; }
	
	if(tr.SkipWhile(tr.whiteSpaceHorizontalString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	if(tr.ReadUntil(buf2, 30, tr.whiteSpaceHorizontalString, true)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	// list contents type in buf2
	if(!strcmp(buf2, "char") || !strcmp(buf2, "int8")) contentsType = tp::b;
	else if(!strcmp(buf2, "uchar") || !strcmp(buf2, "uint8")) contentsType = tp::ub;
	else if(!strcmp(buf2, "short") || !strcmp(buf2, "int16")) contentsType = tp::s;
	else if(!strcmp(buf2, "ushort") || !strcmp(buf2, "uint16")) contentsType = tp::us;
	else if(!strcmp(buf2, "int") || !strcmp(buf2, "int32")) contentsType = tp::i;
	else if(!strcmp(buf2, "uint") || !strcmp(buf2, "uint32")) contentsType = tp::ui;
	else { std::cout << "ERROR: Unrecognised type.\n"; return nullptr; }
	
	if(tr.SkipWhile(tr.whiteSpaceHorizontalString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	if(!tr.CheckString("vertex_indices")){ std::cout << "ERROR: Face format not supported.\n"; return nullptr; }
	
	while(!tr.CheckString("end_header")){
		if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
		if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	}
	if(tr.SkipUntil(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	if(tr.SkipWhile(tr.newLineString)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	// now at start of data
	
	// reading vertex data
	uint8_t *const dataHeap = (uint8_t *)malloc(verticesN*vertexSize);
	if(tr.ReadNumber((char *)dataHeap, vertexSize*verticesN, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
	
	// modifying data
	if(flipNormals && parametersN >= 2){ // we are to flip the normals, and normal data is provided
		if(parameters[1].n >= 2){ // the normal data is at least 2 dimensional
			size_t ptr = parameters[0].n*GLTypeSize(parameters[0].glType);
			const GLsizei sz = GLTypeSize(parameters[1].glType);
			for(int v=0; v<verticesN; v++){
				switch(parameters[1].glType){ // works for all the _SIGNED_ types that can be returned by 'TypePLYtoGL' as used above
					case GL_BYTE:
						for(int i=0; i<parameters[1].n; i++) *(int8_t *)&(dataHeap[ptr + i*sz]) *= -1;
						break;
					case GL_SHORT:
						for(int i=0; i<parameters[1].n; i++) *(int16_t *)&(dataHeap[ptr + i*sz]) *= -1;
						break;
					case GL_INT:
						for(int i=0; i<parameters[1].n; i++) *(int32_t *)&(dataHeap[ptr + i*sz]) *= -1;
						break;
					case GL_FLOAT:
						for(int i=0; i<parameters[1].n; i++) *(float32_t *)&(dataHeap[ptr + i*sz]) *= -1.0f;
						break;
					case GL_DOUBLE:
						for(int i=0; i<parameters[1].n; i++) *(float64_t *)&(dataHeap[ptr + i*sz]) *= -1.0;
						break;
					case GL_UNSIGNED_BYTE:
					case GL_UNSIGNED_SHORT:
					case GL_UNSIGNED_INT:
						std::cout << "WARNING: Normal data had unsigned type, so cannot be flipped.\n";
						break;
					default:
						std::cout << "ERROR: Normal data has unrecognised type.\n";
						break;
				}
				ptr += vertexSize;
			}
		}
	}
	
	// reading and re-formatting face data into index array
	indicesN = 0;
	
	while(tr.GetPlace() < tr.GetLength()){
		int length;
		switch(lengthType){
			case tp::b:
				if(tr.ReadNumber(buf, 1, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				length = *(int8_t *)buf;
				break;
			case tp::ub:
				if(tr.ReadNumber(buf, 1, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				length = *(uint8_t *)buf;
				break;
			case tp::s:
				if(tr.ReadNumber(buf, 2, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				length = *(int16_t *)buf;
				break;
			case tp::us:
				if(tr.ReadNumber(buf, 2, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				length = *(uint16_t *)buf;
				break;
			case tp::i:
				if(tr.ReadNumber(buf, 4, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				length = *(int32_t *)buf;
				break;
			case tp::ui:
				if(tr.ReadNumber(buf, 4, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
				length = *(uint32_t *)buf;
				break;
		}
		
		if(length < 3){ std::cout << "ERROR: Face with too few vertices given.\n"; return nullptr; }
		
		// reading vertices
		GLuint indBuf[length];
		switch(contentsType){
			case tp::b:
				for(int i=0; i<length; i++){
					if(tr.ReadNumber(buf, 1, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					indBuf[i] = *(int8_t *)buf;
				}
				break;
			case tp::ub:
				for(int i=0; i<length; i++){
					if(tr.ReadNumber(buf, 1, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					indBuf[i] = *(uint8_t *)buf;
				}
				break;
			case tp::s:
				for(int i=0; i<length; i++){
					if(tr.ReadNumber(buf, 2, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					indBuf[i] = *(int16_t *)buf;
				}
				break;
			case tp::us:
				for(int i=0; i<length; i++){
					if(tr.ReadNumber(buf, 2, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					indBuf[i] = *(uint16_t *)buf;
				}
				break;
			case tp::i:
				for(int i=0; i<length; i++){
					if(tr.ReadNumber(buf, 4, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					indBuf[i] = *(int32_t *)buf;
				}
				break;
			case tp::ui:
				for(int i=0; i<length; i++){
					if(tr.ReadNumber(buf, 4, false)){ std::cout << "ERROR: End of file reached unexpectedly.\n"; return nullptr; }
					indBuf[i] = *(uint32_t *)buf;
				}
				break;
		}
		
		// dividing into triangles
		for(int i=2; i<length; i++){
			indexBuffer[indicesN++] = indBuf[0];
			indexBuffer[indicesN++] = indBuf[reverseWinding ? i : i - 1];
			indexBuffer[indicesN++] = indBuf[reverseWinding ? i - 1 : i];
		}
	}
	
	// now have set: `indicesN`, `indexBuffer` array
	
	tr.FinishRead();
	
	GLuint *const indexHeap = (GLuint *)malloc(indicesN*sizeof(GLuint));
	memcpy(indexHeap, indexBuffer, indicesN*sizeof(GLuint));
	return new Render::DrawElementArray(verticesN, parametersN, parameters, dataHeap, indicesN, indexHeap, true);
}
