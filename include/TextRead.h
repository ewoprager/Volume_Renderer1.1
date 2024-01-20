#ifndef TextRead_h
#define TextRead_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

class TextRead {
public:
	
	char *FileToBuffer(const char *file, long *fileSize=nullptr); // returned buffer needs to be freed when finished with
	
	void BeginRead(const char *file);
	void FinishRead();
	
	long GetLength(){ return fileLength; }
	long GetPlace(){ return place; }
	void SetPlace(long newPlace){ place = newPlace; }
	
	// return true if they reach the end of the file
	bool ReadNumber(char *buffer,  int number, bool terminate, bool move=true);
	bool SkipNumber(int number, bool move=true);
	bool ReadUntil(char *buffer, unsigned int maxN, const char *untilChars, bool terminate, bool move=true);
	bool SkipUntil(const char *untilChars, bool move=true);
	bool ReadWhile(char *buffer, unsigned int maxN, const char *whileChars, bool terminate, bool move=true);
	bool SkipWhile(const char *whileChars, bool move=true);
	
	bool SkipToNextLine(bool move=true);
	bool SkipLinesNumber(unsigned int number, bool move=true);
	bool SkipLinesUntil(const char *untilChars);
	bool SkipLinesWhile(const char *whileChars);
	
	char GetCharacter(bool move){
		char ret = masterBuffer[place];
		if(move && place < fileLength - 1) place++;
		return ret;
	}
	bool CheckCharacter(const char *characters, long index=-1){
		if(index < 0) index = place;
		for(unsigned int i=0; characters[i] != '\0'; i++) if(masterBuffer[index] == characters[i]) return true;
		return false;
	}
	bool CheckString(const char *string, long index=-1){
		if(index < 0) index = place;
		for(unsigned int i=0; string[i] != '\0'; i++){
			if(index + i >= fileLength) return false;
			if(masterBuffer[index + i] != string[i]) return false;
		}
		return true;
	}
	
	const char whiteSpaceHorizontalString[3] = " \t";
	const char newLineString[3] = "\r\n";
	const char whiteSpaceString[5] = " \t\r\n";
	
private:
	bool fileOpen = false;
	long place;
	long fileLength;
	char *masterBuffer;
};

// Reads a file into an allocated char pointer buffer
char *TextRead::FileToBuffer(const char *file, long *fileSize){
	FILE *fptr;
	long length;
	char *buffer;
	
	fptr = fopen(file, "rb"); // Open file for reading
	if(!fptr) return NULL; // Return NULL on failure
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	if(!length){ std::cout << "ERROR: File empty." << std::endl; return nullptr; }
	buffer = (char *)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buffer, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buffer[length] = 0; // Null terminator
		
	if(fileSize) *fileSize = length;

	return buffer; // Return the buffer
}

void TextRead::BeginRead(const char *file){
	fileOpen = true;
	masterBuffer = FileToBuffer(file, &fileLength);
	if(!masterBuffer){ std::cout << "ERROR: Unable to open file.\n"; return; }
	place = 0;
}
void TextRead::FinishRead(){
	free(masterBuffer);
	fileOpen = false;
}

bool TextRead::ReadNumber(char *buffer, int number, bool terminate, bool move){
	if(!fileOpen){ std::cout << "ERROR: No file open to read from." << std::endl; return true; }
	bool ret = (place + number > fileLength);
	int n = ret ? (int)(fileLength - place) : number;
	memcpy(buffer, masterBuffer + place, n);
	if(terminate) buffer[n] = '\0';
	if(move) place += n;
	return ret;
}
bool TextRead::SkipNumber(int number, bool move){
	if(!fileOpen){ std::cout << "ERROR: No file open to read from." << std::endl; return true; }
	bool ret = (place + number > fileLength);
	if(move) place = ret ? fileLength : place + number;
	return ret;
}

bool TextRead::ReadUntil(char *buffer, unsigned int maxN, const char *untilChars, bool terminate, bool move){
	if(!fileOpen){ std::cout << "ERROR: No file open to read from." << std::endl; return true; }
	bool ret = false;
	unsigned int i = 0;
	while(true){
		if(place + i >= fileLength){ ret = true; break; }
		if(CheckCharacter(untilChars, place + i)) break;
		if(i >= maxN) break;
		buffer[i] = masterBuffer[place + i];
		i++;
	}
	if(terminate) buffer[i] = '\0';
	if(move){
		place += i;
		if(place >= fileLength) place = fileLength - 1;
	}
	return ret;
}
bool TextRead::SkipUntil(const char *untilChars, bool move){
	if(!fileOpen){ std::cout << "ERROR: No file open to read from." << std::endl; return true; }
	bool ret = false;
	long index = place;
	while(true){
		if(index + 1 >= fileLength){ ret = true; break; }
		if(CheckCharacter(untilChars, index)) break;
		index++;
	}
	if(move) place = index;
	return ret;
}

bool TextRead::ReadWhile(char *buffer, unsigned int maxN, const char *whileChars, bool terminate, bool move){
	if(!fileOpen){ std::cout << "ERROR: No file open to read from." << std::endl; return true; }
	bool ret = false;
	unsigned int i = 0;
	while(true){
		if(place + i >= fileLength){ ret = true; break; }
		if(!CheckCharacter(whileChars, place + i)) break;
		if(i >= maxN) break;
		buffer[i] = masterBuffer[place + i];
		i++;
	}
	if(terminate) buffer[i] = '\0';
	if(move){
		place += i;
		if(place >= fileLength) place = fileLength - 1;
	}
	return ret;
}
bool TextRead::SkipWhile(const char *whileChars, bool move){
	if(!fileOpen){ std::cout << "ERROR: No file open to read from." << std::endl; return true; }
	bool ret = false;
	long index = place;
	while(true){
		if(index + 1 >= fileLength){ ret = true; break; }
		if(!CheckCharacter(whileChars, index)) break;
		index++;
	}
	if(move) place = index;
	return ret;
}

bool TextRead::SkipToNextLine(bool move){
	if(SkipUntil(newLineString, move)) return true;
	if(SkipWhile(newLineString, move)) return true;
	return false;
}
bool TextRead::SkipLinesNumber(unsigned int number, bool move){
	for(int i=0; i<number && place < fileLength - 1; i++) if(SkipToNextLine(move)) return true;
	return false;
}
bool TextRead::SkipLinesUntil(const char *untilChars){
	while(true){
		if(CheckCharacter(untilChars)) break;
		if(SkipToNextLine(true)) return true;
	}
	return false;
}
bool TextRead::SkipLinesWhile(const char *whileChars){
	while(true){
		if(!CheckCharacter(whileChars)) break;
		if(SkipToNextLine(true)) return true;
	}
	return false;
}

#endif /* TextRead_h */
