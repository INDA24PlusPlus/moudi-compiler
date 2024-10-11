#include "common/io.h"

FILE * open_file (const char * filename, const char * options) {
	FILE * out = fopen(filename, options);
	if (out == NULL) {
		println("[Error]: Cannot open file: '{s}'", filename);
		exit(1);
	}
	return out;
}

char* read_file(const char * filename, size_t * length) {

	FILE * fp;
	char * buffer;
    size_t size;

	fp = open_file(filename, "rb");	
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

	buffer = malloc((size + 1) * sizeof(char));
    
    fread_unlocked(buffer, sizeof(char), size, fp);
	fclose(fp);
    buffer[size] = '\0';
    
    *length = size;

	return buffer;
}
