#include "lame.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <omp.h>

// FIXME set number of chnks to appropriate value
#define CHUNKS 32
#define PCM_CHUNK_SIZE (8192 * 2)
#define MP3_CHUNK_SIZE 8192
#define PCM_SIZE (CHUNKS * PCM_CHUNK_SIZE)
#define MP3_SIZE (CHUNKS * MP3_CHUNK_SIZE)

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
	FILE* pcm = fopen(argv[1], "rb");
	
	char* outPath;
	outPath = strdup(argv[1]);
	sprintf(outPath+strlen(argv[1])-3, "mp3");
	
	FILE* mp3 = fopen(outPath, "wb");
	
	int read, write[CHUNKS];
	short int pcm_buffer[PCM_SIZE];
	unsigned char mp3_buffer[CHUNKS][MP3_CHUNK_SIZE];
	int i, chunks;

	do
	{
		memset(pcm_buffer, 0, PCM_SIZE * sizeof(short int));
		memset(write, 0, CHUNKS * sizeof(int));
		read = fread(pcm_buffer, sizeof(short int), PCM_SIZE, pcm);

		chunks = read / PCM_CHUNK_SIZE; //FIXME this might not encode up to the end of the file

		printf("bananas\n");
		if(read == 0)
			write[0] = lame_encode_flush(gfp, mp3_buffer[0], MP3_CHUNK_SIZE);
		else {
			#pragma omp parallel for private(i)
			for (i = 0; i < chunks; i++) {
				write[i] = lame_encode_buffer_interleaved(gfp,
													pcm_buffer + i * PCM_CHUNK_SIZE, PCM_CHUNK_SIZE / 2,
													mp3_buffer[i], MP3_CHUNK_SIZE);
			}
		}

		// keep this serial :)
		for (i = 0; i < chunks; i++) {
			if (write[i] == 0) break;
			fwrite(mp3_buffer[i], write[i], sizeof (unsigned char), mp3);
		}
	}
	while (read != 0); 
	
	lame_close(gfp);
	fclose(mp3);
	fclose(pcm);
	
	exit(0);
}
