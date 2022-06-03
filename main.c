#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Parses through the first 9 bytes of stream for magic number (does not reset stream)
 * Arguments: file pointer
 * Returns: exit code
 * 		0: magic number found
 * 		1: no magic numbers found */
int magic_number (FILE *fp);

/* Encodes data into message and writes it to text
 * Arguments: file pointers to message, data, and text
 * Returns: void */
void encode_data(FILE *fpm, FILE *fpd, FILE *fpt);

/* Decodes data from text and writes it to data file stream
 * Arguments: file pointers to text and data
 * Returns: void */
void decode_data(FILE *fpd, FILE *fpt);

/* Takes char and checks if nth bit is set
 * Arguments: char, n
 * Returns: exit code
 * 		0: bit not set
 * 		1: bit is set */
int checkbit (char data, int bit);

/* Takes first byte of utf8 character and returns how many bits are set
 * Arguments: char
 * Returns: number of bits set */
int checkbytes (char data);

char *prog;
static const char magic[] = {0xe2, 0x80, 0x8b, 0xcd, 0x8f, 0};
int main(int argc, char *argv[])
{
	FILE *fpd; /* file pointer to data */
	FILE *fpt; /* file pointer to text */
	FILE *fpm; /* file pointer to message */
	prog = argv[0]; /* Program name for errors */
	int encode;

	/* TODO: Add command line option parsing here */
	if (argc == 1) {
		//fprintf(stderr, "%s: No file(s) specified\n", prog);
		//exit(22);

		fpt = stdin;
		if ( !magic_number(fpt) ) {
			fprintf(stderr, "%s: Assuming stdin is text\n", prog);
			encode = 0;
			fpd = stdout;
		} else {
			fprintf(stderr, "%s: stdin does not contain magic numbers\n", prog);
			exit(2);
		}

	} else if (argc == 2) {
		if ( (fpd = fopen(argv[1], "r")) == NULL ) {
			fprintf(stderr, "%s: cannot access '%s': ", prog, argv[1]);
			perror("");
			exit(errno);
		}
		if ( encode = magic_number(fpd) ) {
			if ( fseek(fpd, 0L, SEEK_SET) != 0 ) {
				fprintf(stderr, "%s: error resetting file pointer: ", prog);
				perror("");
				exit(errno);
			}
			
			fprintf(stderr, "Assuming file given is data\n");
			fpm = stdin;
			fpt = stdout;
		} else {
			fprintf(stderr, "Assuming file given is text\n");
			fpt = fpd;
			fpd = stdout;
		}
	}



	fprintf(stderr, "The value of encode is %d\n", encode);

	if (encode) {
		fprintf(stderr, "Encode is true\n");
		encode_data(fpm, fpd, fpt);
	} else {
		fprintf(stderr, "encode is false\n");
		decode_data(fpd, fpt);
	}

	exit(0);
}

int magic_number (FILE *fp)
{
	int i = 0, j = 0;
	unsigned char data;
	while (i < 9) {
		data = fgetc(fp);
		if ( ferror(fp) ) {
			fprintf(stderr, "%s: Error reading text stream: ", prog);
			perror("");
			exit(errno);
		}
		if ( feof(fp) ) {
			break;
		}

		if ( data == 0xe2 && j == 0 ) {
			++j;
		} else if ( data == 0x80 && j == 1 ) {
			++j;
		} else if ( data == 0x8b && j == 2 ) {
			++j;
		} else if ( data == 0xcd && j == 3 ) {
			++j;
		} else if ( data == 0x8f && j == 4 ) {
			return 0;
		} else if ( j > 0 ) {
			j = 0;
			if ( data == 0xe2 ) {
                        	++j;
                	}
		}

		if ( i == 4 && j == 0 ) {
			break;
		}

		++i;

	}
	return 1;

	//unsigned char buffer[10];
        //fgets(buffer, 10, fp);
	//
	//int i;
	//for (i = 0; i < 5; i++) {
	//	if ( buffer[i] == 0xe2 ) {
	//		if ( buffer[i+1] == 0x80 ) {
	//			if ( buffer[i+2] == 0x8b ) {
	//				if ( buffer[i+3] == 0xcd ) {
	//					if ( buffer[i+4] == 0x8f ) {
	//						return 0;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
	//return 1;
}

void encode_data(FILE *fpm, FILE *fpd, FILE *fpt)
{
	/* check size of data file */
	off_t data_file_size;
	if ( fseek(fpd, 0L, SEEK_END) ) { /* seek to end of file */
		fprintf(stderr, "%s: Error getting file size of data: ", prog);
		perror("");
		exit(errno);
	}

	data_file_size = ftello(fpd); /* Sets file size equal to current offset */
	if ( data_file_size == -1 ) {
		fprintf(stderr, "%s: Error getting file size of data: ", prog);
                perror("");
                exit(errno);
	}
	
	if ( fseek(fpd, 0L, SEEK_SET) ) { /* seek to beginning of file */
                fprintf(stderr, "%s: Error getting file size of data: ", prog);
                perror("");
                exit(errno);
        }

	if ( data_file_size == 0 ) {
		fprintf(stderr, "%s: Size of data file is zero, no data to encode\n", prog);
		exit(1);
	}
	fprintf(stderr, "Data size: %d\n", data_file_size);


	size_t i = 0, n = 32;
	char c;
	/* allocate memory */
	char *ptr;
	if ( (ptr = (char*) calloc(n, sizeof(char))) == NULL ) {
		fprintf(stderr, "%s: error allocating memory: ", prog);
		perror("");
		exit(errno);
	}

	/* copy message into memory */
	while ( (c = fgetc(fpm)) != EOF ) {
		if (i == n) { /* allocates more memory */
			n = n + 32;
			ptr = reallocarray(ptr, n, sizeof(char));
			if (ptr == NULL) {
				fprintf(stderr, "%s: error allocating memory: ", prog);
				perror("");
				exit(errno);
			}
		}
		ptr[i] = c;
		++i;
	}

	/* count number of characters in message */
	i = 0;
	n = 0;
	while ( ptr[i] != 0 ) {
		i = i + checkbytes(ptr[i]);
		n = n + 1;
	}
	if ( n < 3 ) {
		fprintf(stderr, "%s: Not enough characters in message\n", prog);
		exit(1);
	}
	size_t number_of_characters = n;
	fprintf(stderr, "Number of characters: %d\n", number_of_characters);
	size_t number_of_spaces = (n - 2);
	fprintf(stderr, "Number of usable spaces: %d\n", number_of_spaces);
	
	/* calculates the number of bytes to put in each space */
	unsigned long long int bytes_per_space;
	bytes_per_space = ( data_file_size + (number_of_spaces - 1) ) / number_of_spaces;
        fprintf(stderr, "Bytes per space: %llu\n", bytes_per_space);


	unsigned char data;
	off_t j = 1;
	unsigned long long int k = 1;
	int char_size, x = 0;
	i = 0;
	n = 0; /* counter for number of characters written */

	/* output one character from message and magic number */
	char_size = checkbytes(ptr[i]);
        for ( x = 1; x <= char_size; ++x ) {
                if ( fputc(ptr[i], fpt) == EOF ) {
                        fprintf(stderr, "%s: Error writing to text: ", prog);
                        perror("");
                        exit(errno);
                }
                ++i;
        }
        ++n;
	if ( fputs(magic, fpt) == EOF ) {
		fprintf(stderr, "%s: Error writing to text: ", prog);
		perror("");
		exit(errno);
	}

	char text[4] = {0};
	const char text0[4] = {0xe2, 0x80, 0x8b, 0};
	const char text1[4] = {0xe2, 0x80, 0x8c, 0};
	const char text2[4] = {0xe2, 0x80, 0x8d, 0};
	const char text3[4] = {0xe2, 0x81, 0xa0, 0};
	int l;
	int m;
	while ( n < number_of_characters ) {
		/* output data as utf8 characters */
		for ( k = 1; k <= bytes_per_space && j <= data_file_size; ++k ) {
			data = fgetc(fpd);
			for ( m = 3; m >= 0; --m ) {
				l = (data >> (m*2)) & 3;
				if ( l == 0 ) {
					if ( fputs(text0, fpt) == EOF ) {
                                        	fprintf(stderr, "%s: Error writing to text: ", prog);
                                        	perror("");
                                        	exit(errno);
                                	}
				} else if ( l == 1 ) {
					if ( fputs(text1, fpt) == EOF ) {
                                        	fprintf(stderr, "%s: Error writing to text: ", prog);
                                        	perror("");
                                        	exit(errno);
                                	}
				} else if ( l == 2 ) {
					if ( fputs(text2, fpt) == EOF ) {
                                        	fprintf(stderr, "%s: Error writing to text: ", prog);
                                        	perror("");
                                        	exit(errno);
                                	}
				} else if ( l == 3 ) {
					if ( fputs(text3, fpt) == EOF ) {
                                        	fprintf(stderr, "%s: Error writing to text: ", prog);
                                        	perror("");
                                        	exit(errno);
                                	}
				} else {
					fprintf(stderr, "%s: Error writing to text: ", prog);
					exit(2);
				}

			}
			++j;
		}

		/* output one character from message */
		char_size = checkbytes(ptr[i]);
		for ( x = 1; x <= char_size; ++x ) {
			if ( fputc(ptr[i], fpt) == EOF ) {
				fprintf(stderr, "%s: Error writing to text: ", prog);
				perror("");
				exit(errno);
			}
			++i;
		}
		++n;

	}

	free(ptr);

}

void decode_data(FILE *fpd, FILE *fpt)
{
	unsigned char data = 0;
	unsigned char text;
	unsigned char buffer[4] = {0};
	int i = 0, j = 0, k = 3;

	/* decode data */
	while (1) {
		text = fgetc(fpt);
		if ( ferror(fpt) ) {
			fprintf(stderr, "%s: Error reading text stream: ", prog);
			perror("");
			exit(errno);
		}
		if ( feof(fpt) ) {
			break;
		}

		if ( i == j ) {
			j = checkbytes(text);
			i = 0;
		}
		
		buffer[i] = text;
		++i;

		
		if ( i == j ) {
			if ( j == 3 && buffer[0] == 0xe2 ) {
				if ( buffer[1] == 0x80 ) {
					if ( buffer[2] == 0x8b ) {
						--k;
						if ( k < 0 ) {
							fputc(data, fpd);
							data = 0;
							k = 3;
						}

					} else if ( buffer[2] == 0x8c ) {
						data = data | (1 << (k*2));
						--k;
						if ( k < 0 ) {
							fputc(data, fpd);
							data = 0;
							k = 3;
						}
						
					} else if ( buffer[2] == 0x8d ) {
						data = data | (2 << (k*2));
						--k;
						if ( k < 0 ) {
							fputc(data, fpd);
							data = 0;
							k = 3;
						}	
					}
				} else if ( buffer[1] == 0x81 ) {
					if ( buffer[2] == 0xa0 ) {
						data = data | (3 << (k*2));
						--k;
						if ( k < 0 ) {
							fputc(data, fpd);
							data = 0;
							k = 3;
						}
					}
				}

			} else if ( j == 2 && buffer[0] == 0xcd && buffer[1] == 0x8f ) {
				;
			}
		}
	}
}

int checkbit (char data, int bit)
{
	if ( data & (1<<bit) ) {
		return 1;
	} else {
		return 0;
	}
}

int checkbytes (char data)
{
	int bytes = 1, i = 7;
	
	while ( data & (1<<i) ) {
		if ( i != 7 ) {
			++bytes;
		}
		--i;
	}
	
	if ( i == 6 || i < 3 ) {
		fprintf(stderr, "%s: Error parsing UTF-8 string\n", prog);
		exit(2);
	}

	return bytes;
}
