#include "lame.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define PCM_SIZE 8192
#define MP3_SIZE 8192

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: %s <wav file>\n", argv[0]);
		exit(0);
	}

	lame_global_flags *gfp;
	
	
	//set params
	gfp = lame_init();
	
	lame_set_num_channels(gfp,2);
	lame_set_in_samplerate(gfp,44100);
	lame_set_brate(gfp,128);
	lame_set_mode(gfp,1);
	lame_set_quality(gfp,2); 
	
	int ret_code = lame_init_params(gfp);
	if(ret_code < 0)
	{
		printf("ret_code < 0\n");
		exit(-1);
	}
	
	//read input file
	int read, write;
	FILE* pcm = fopen(argv[1], "rb");
	
	char* outPath;
	outPath = strdup(argv[1]);
	sprintf(outPath+strlen(argv[1])-3, "mp3");
	
	FILE* mp3 = fopen(outPath, "wb");
	

	
	short int pcm_buffer[PCM_SIZE * 2];
	unsigned char mp3_buffer[MP3_SIZE];
	
	do
	{
		read = fread(pcm_buffer, 2*sizeof(short int), PCM_SIZE, pcm);
		if(read == 0)
			write = lame_encode_flush(gfp, mp3_buffer, MP3_SIZE);
		else
			write = lame_encode_buffer_interleaved(gfp, pcm_buffer, read, mp3_buffer, MP3_SIZE);
		fwrite(mp3_buffer, write, sizeof (unsigned char), mp3);
	}
	while (read != 0);
	
	lame_close(gfp);
	fclose(mp3);
	fclose(pcm);
	
	exit(0);
}
