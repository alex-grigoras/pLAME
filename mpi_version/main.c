#include "lame.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>

#define PCM_CHUNK_SIZE (819200 * 2)
#define MP3_CHUNK_SIZE 819200


int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: %s <wav file>\n", argv[0]);
		exit(0);
	}

	MPI_Init(&argc, &argv);
	int CHUNKS;
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &CHUNKS);
	int PCM_SIZE = (CHUNKS * PCM_CHUNK_SIZE);
	int MP3_SIZE = (CHUNKS * MP3_CHUNK_SIZE);
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
	FILE* pcm ;
	if(rank == 0)
		pcm = fopen(argv[1], "rb");
	
	char* outPath;
	outPath = strdup(argv[1]);
	sprintf(outPath+strlen(argv[1])-3, "mp3");

	FILE* mp3;
	if(rank == 0)
		mp3 = fopen(outPath, "wb");
	if(rank == 0)
	fflush(stdout);
	int read, write;
	short int pcm_buffer[PCM_SIZE];
	short int my_pcm_buffer[PCM_CHUNK_SIZE];
	unsigned char mp3_buffer[MP3_CHUNK_SIZE];
	unsigned char my_mp3_buffer[MP3_SIZE];
	int i;
	int writes[CHUNKS];
	int chunk_size;
	do
	{
		
		memset(pcm_buffer, 0, PCM_SIZE * sizeof(short int));
		memset(writes, 0, CHUNKS * sizeof(int));
		
		if(rank == 0)
			read = fread(pcm_buffer, sizeof(short int), PCM_SIZE, pcm);
		
		MPI_Bcast( &read, 1, MPI_INT, 0,  MPI_COMM_WORLD );
		
		chunk_size = read / CHUNKS;
		if(read == 0)
		{
			if(rank == 0) 
			{
				
				write = lame_encode_flush(gfp, mp3_buffer, MP3_CHUNK_SIZE);
				
				fwrite(mp3_buffer, write, sizeof (unsigned char), mp3);
			}
			break;
		}
		else 
		{
	
	        	MPI_Scatter(pcm_buffer, chunk_size, MPI_SHORT, my_pcm_buffer, chunk_size, MPI_SHORT, 0, MPI_COMM_WORLD);	
			write = lame_encode_buffer_interleaved(gfp,
						my_pcm_buffer, chunk_size / 2,
						mp3_buffer, MP3_CHUNK_SIZE);

		}

		//gather
		MPI_Gather(mp3_buffer, MP3_CHUNK_SIZE, MPI_UNSIGNED_CHAR, my_mp3_buffer, MP3_CHUNK_SIZE, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
		MPI_Gather(&write, 1, MPI_INT, writes, 1, MPI_INT, 0, MPI_COMM_WORLD);

	
		// keep this serial :)
		if (rank == 0)
		{
			for (i = 0; i < CHUNKS; i++) {
				fwrite(my_mp3_buffer+(i*MP3_CHUNK_SIZE), writes[i], sizeof (unsigned char), mp3);
			}
		}
	}
	while (read != 0); 

	lame_close(gfp);
	if(rank == 0)
	{
		fclose(mp3);
		fclose(pcm);
	}
	MPI_Finalize();
	exit(0);
}
