#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <openssl/md5.h>

/* Parses through the first 9 bytes of stream for magic number (does not reset stream)
 * Arguments: file pointer
 * Returns: exit code
 * 		0: magic number found
 * 		1: no magic numbers found */
int check_magic_number(FILE *fp);

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
int checkbit(char data, int bit);

/* Takes first byte of utf8 character and returns how many bits are set
 * Arguments: char
 * Returns: number of bits set */
int checkbytes(char data);

char *prog;
int verbose = 0;
static const char magic[] = {0xe2, 0x80, 0x8b, 0xcd, 0x8f, 0};
int main(int argc, char *argv[])
{
	FILE *fpd = NULL; /* file pointer to data */
	FILE *fpt = NULL; /* file pointer to text */
	FILE *fpm = NULL; /* file pointer to message */
	prog = argv[0]; /* Program name for errors */
	int encode = -1, opt;

	/* command line option parsing */
	while ( (opt = getopt(argc, argv, "cexd:m:t:vh")) != -1 ) {
                switch (opt) {
		case 'c':
                case 'e':
                        encode = 1;
                        break;
                case 'x':
                        encode = 0;
                        break;
                case 'd':
			if ( (fpd = fopen(optarg, "r")) == NULL ) {
                        	fprintf(stderr, "%s: cannot access '%s': ", prog, optarg);
                        	perror("");
                        	exit(errno);
                	}
                        break;
                case 'm':
			if ( (fpm = fopen(optarg, "r")) == NULL ) {
                                fprintf(stderr, "%s: cannot access '%s': ", prog, optarg);
                                perror("");
                                exit(errno);
                        }
                        break;
                case 't':
			if ( (fpt = fopen(optarg, "r")) == NULL ) {
                                fprintf(stderr, "%s: cannot access '%s': ", prog, optarg);
                                perror("");
                                exit(errno);
                        }
                        break;
		case 'v':
			++verbose;
			break;
		case 'h':
			fprintf(stderr, "See 'man utf-8-steg(1)'\n");
			exit(0);
                default:
                        exit(1);
                }
        }

	if ( encode == -1 ) {
		fprintf(stderr, "%s: No action specified\n", prog);
		exit(1);
	}

	/* handles missing options */
	if ( encode == 1 ) {
		if (fpt == NULL) {
			fpt = stdout;
		}
		if ( fpd == NULL && fpm == NULL ) {
			fprintf(stderr, "%s: Need to specify either data file, message file, or both in encode mode\n", prog);
			exit(1);
		}
		if (fpd == NULL) {
			fpd = stdin;
		}
		if (fpm == NULL) {
			fpm = stdin;
		}
	}
	if ( encode == 0 ) {
		if (fpt == NULL) {
                        fpt = stdin;
                }
                if (fpd == NULL) {
                        fpd = stdout;
                }
                if (fpm != NULL) {
			fprintf(stderr, "%s: Ignoring message file because currently in decode mode\n", prog);
                        fpm = NULL;
                }
	}


	if (encode == 1) {
		if ( isatty(fileno(fpm)) && verbose ) {
			fprintf(stderr, "%s: Reading message from tty...\n", prog);
		} else if ( isatty(fileno(fpd)) && verbose ) {
			fprintf(stderr, "%s: Reading data from tty...\n", prog);
		}

		if (verbose) { fprintf(stderr, "%s: Encoding data...\n", prog); }
		encode_data(fpm, fpd, fpt);

	} else if (encode == 0) {
		if ( isatty(fileno(fpt)) && verbose ) {
			fprintf(stderr, "%s: Reading text from tty...\n", prog);
		}
		if (verbose) {
			fprintf(stderr, "%s: Decoding data...\n", prog);
		}

		/* check magic numbers */
		if (verbose > 1) { fprintf(stderr, "%s: Checking magic numbers...\n", prog); }
		if ( !check_magic_number(fpt) ) {
			decode_data(fpd, fpt);
		} else {
			fprintf(stderr, "%s: text stream does not contain magic numbers\n", prog);
			exit(2);
		}

	} else {
		fprintf(stderr, "Encode value is not set\n");
		exit(2);
	}

	exit(0);
}

int check_magic_number(FILE *fp)
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

}

void encode_data(FILE *fpm, FILE *fpd, FILE *fpt)
{
	off_t data_file_size = 0;
	int data_from_tty = 0;
	char *data_ptr;

	size_t i = 0, n = 32;
	char c;

	if (fpd != stdin) {
		/* check size of data file */
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
		if (verbose > 1) { fprintf(stderr, "%s: Data size: %i\n", prog, data_file_size); }

	} else if ( data_from_tty = isatty(fileno(fpd)) ) {
		/* buffer data into memory if data is being read from a tty */

		/* allocate memory for data */
        	if ( (data_ptr = (char*) calloc(n, sizeof(char))) == NULL ) {
        	        fprintf(stderr, "%s: error allocating memory: ", prog);
        	        perror("");
        	        exit(errno);
        	}

		/* copy data into memory */
		while (1) {
			c = fgetc(fpd);
			if ( ferror(fpd) ) {
				fprintf(stderr, "%s: error allocating memory: ", prog);
                                perror("");
                                exit(errno);
			}
			if ( feof(fpd) ) {
				break;
			}
                	if (i == n) { /* allocates more memory */
                	        n = n + 32;
                	        data_ptr = reallocarray(data_ptr, n, sizeof(char));
                	        if (data_ptr == NULL) {
                	                fprintf(stderr, "%s: error allocating memory: ", prog);
                	                perror("");
                	                exit(errno);
                	        }
                	}
                	data_ptr[i] = c;
                	++i;
		}
		
		data_file_size = i;

		if ( data_file_size == 0 ) {
			fprintf(stderr, "%s: No data to encode", prog);
			exit(1);
		}
		if (verbose > 1) { fprintf(stderr, "%s: Data size: %i\n", prog, data_file_size); }
		
	}

	
	i = 0;
	n = 32;
	/* allocate memory for message */
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
	if (verbose > 1) { fprintf(stderr, "%s: Number of characters: %zu\n", prog, number_of_characters); }
	size_t number_of_spaces = (n - 2);
	if (verbose > 1) { fprintf(stderr, "%s: Number of usable spaces: %zu\n", prog, number_of_spaces); }
	
	unsigned long long int bytes_per_space;
	if ( data_file_size != 0 ) {
		/* calculates the number of bytes to put in each space */
		bytes_per_space = ( data_file_size + (number_of_spaces - 1) ) / number_of_spaces;
        	if (verbose > 1) { fprintf(stderr, "%s: Bytes per space: %llu\n", prog, bytes_per_space); }
	}

	unsigned char data;
	off_t j = 0; /* counter for number of bytes written */
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

	/* initialize MD5_CTX structure */
	MD5_CTX context;
	uint8_t digest[MD5_DIGEST_LENGTH];
	MD5_Init(&context);

	/* boolean for whether or not the checksum has been outputted */
	int checksum_outputted = 0;

	const char text0[4] = {0xe2, 0x80, 0x8b, 0};
	const char text1[4] = {0xe2, 0x80, 0x8c, 0};
	const char text2[4] = {0xe2, 0x80, 0x8d, 0};
	const char text3[4] = {0xe2, 0x81, 0xa0, 0};
	const char text4[3] = {0xcd, 0x8f, 0};
	int l;
	int m;
	while ( n < number_of_characters ) {
		if (data_file_size != 0) {
			/* output data as utf8 characters */
			for ( k = 1; k <= bytes_per_space && j < data_file_size; ++k ) {
				/* get data from either memory or stream */
				if (data_from_tty) {
					data = data_ptr[j];
				} else {
					data = fgetc(fpd);
				}

				/* update md5 */
				if ( !MD5_Update(&context, &data, 1) ) {
					fprintf(stderr, "%s: Error calculating checksum\n", prog);
					exit(2);
				}
				
				/* encode data */
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
		} else {
			while (1) {
				data = fgetc(fpd);

				if ( ferror(fpd) ) {
                		        fprintf(stderr, "%s: Error reading data stream: ", prog);
                		        perror("");
                		        exit(errno);
                		}
                		if ( feof(fpd) ) {
                		        break;
                		}

				/* update md5 */
				if ( !MD5_Update(&context, &data, 1) ) {
					fprintf(stderr, "%s: Error calculating checksum\n", prog);
					exit(2);
				}

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
			}
		}

		/* calculates and encodes md5sum after all data is encoded */
		if ( checksum_outputted == 0 && 
				((data_file_size != 0 && j == data_file_size) ||
				 (data_file_size == 0 && feof(fpd))) ) {

			checksum_outputted = 1;

			MD5_Final(digest, &context);

			/* output delimiter to signal end of data and start of checksum */
			 if ( fputs(text4, fpt) == EOF ) {
                         	fprintf(stderr, "%s: Error writing to text: ", prog);
                         	perror("");
                         	exit(errno);
			 }

			for (x = 0; x < MD5_DIGEST_LENGTH; ++x) {
				for ( m = 3; m >= 0; --m ) {
                        	        l = (digest[x] >> (m*2)) & 3;
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
			}

			/* output delimiter to signal end of checksum */
			 if ( fputs(text4, fpt) == EOF ) {
                         	fprintf(stderr, "%s: Error writing to text: ", prog);
                         	perror("");
                         	exit(errno);
			 }

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
	if (data_from_tty) {
		free(data_ptr);
	}

	if (verbose >= 2) {
		fprintf(stderr, "%s: MD5 = ", prog);
		for(x = 0; x < MD5_DIGEST_LENGTH; ++x)
			fprintf(stderr, "%02x", digest[x]);
		fprintf(stderr, "\n");
	}

}

void decode_data(FILE *fpd, FILE *fpt)
{
	unsigned char data = 0;
	unsigned char text;
	unsigned char buffer[4] = {0};
	int i = 0, j = 0, k = 3;

	int is_checksum = 0;
	MD5_CTX context;
	MD5_Init(&context);

	int l = 0; /* iterator for given digest */
	uint8_t calculated_digest[MD5_DIGEST_LENGTH] = {0};
	uint8_t given_digest[MD5_DIGEST_LENGTH] = {0};

	/* buffer text if reading from a tty */
	int text_is_from_tty;
	unsigned char *text_ptr;
	int n = 32;
	off_t text_file_size = 0;
	if ( text_is_from_tty = isatty(fileno(fpt)) ) {

		unsigned char c;
		/* allocate memory for text */
        	if ( (text_ptr = (unsigned char*) calloc(n, sizeof(unsigned char))) == NULL ) {
        	        fprintf(stderr, "%s: error allocating memory: ", prog);
        	        perror("");
        	        exit(errno);
        	}

		/* copy text into memory */
		i = 0;
		while (1) {
			c = fgetc(fpt);
			if ( ferror(fpt) ) {
				fprintf(stderr, "%s: error allocating memory: ", prog);
                                perror("");
                                exit(errno);
			}
			if ( feof(fpt) ) {
				break;
			}
                	if (i == n) { /* allocates more memory */
                	        n = n + 32;
                	        text_ptr = reallocarray(text_ptr, n, sizeof(unsigned char));
                	        if (text_ptr == NULL) {
                	                fprintf(stderr, "%s: error allocating memory: ", prog);
                	                perror("");
                	                exit(errno);
                	        }
                	}
                	text_ptr[i] = c;
                	++i;
		}

		text_file_size = i;

		if ( text_file_size == 0 ) {
			fprintf(stderr, "%s: No text to decode", prog);
			exit(1);
		}
		if (verbose > 1)
			fprintf(stderr, "%s: Text size: %i\n", prog, text_file_size);

	}

	/* decode data */
	i = 0;
	n = 0;
	while (1) {
		if (text_is_from_tty) {
			if (n == text_file_size) 
				break;
			text = text_ptr[n];
			n++;
		} else {
			text = fgetc(fpt);
			if ( ferror(fpt) ) {
				fprintf(stderr, "%s: Error reading text stream: ", prog);
				perror("");
				exit(errno);
			}
			if ( feof(fpt) )
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
							if ( is_checksum && (l < MD5_DIGEST_LENGTH) ) {
								given_digest[l] = data;
								++l;
							} else {
								MD5_Update(&context, &data, 1);
								fputc(data, fpd);
							}
							data = 0;
							k = 3;
						}

					} else if ( buffer[2] == 0x8c ) {
						data = data | (1 << (k*2));
						--k;
						if ( k < 0 ) {
							if ( is_checksum && (l < MD5_DIGEST_LENGTH) ) {
								given_digest[l] = data;
								++l;
							} else {
								MD5_Update(&context, &data, 1);
								fputc(data, fpd);
							}
							data = 0;
							k = 3;
						}
						
					} else if ( buffer[2] == 0x8d ) {
						data = data | (2 << (k*2));
						--k;
						if ( k < 0 ) {
							if ( is_checksum && (l < MD5_DIGEST_LENGTH) ) {
								given_digest[l] = data;
								++l;
							} else {
								MD5_Update(&context, &data, 1);
								fputc(data, fpd);
							}
							data = 0;
							k = 3;
						}	
					}
				} else if ( buffer[1] == 0x81 ) {
					if ( buffer[2] == 0xa0 ) {
						data = data | (3 << (k*2));
						--k;
						if ( k < 0 ) {
							if ( is_checksum && (l < MD5_DIGEST_LENGTH) ) {
								given_digest[l] = data;
								++l;
							} else {
								MD5_Update(&context, &data, 1);
								fputc(data, fpd);
							}
							data = 0;
							k = 3;
						}
					}
				}

			} else if ( j == 2 && buffer[0] == 0xcd && buffer[1] == 0x8f ) {
				if (is_checksum) {
					is_checksum = 2;
				} else {
					is_checksum = 1;
				}
			}
		}
	}

	/* check if no checksum was given */
	for (l = 0; l < MD5_DIGEST_LENGTH; ++l) {
		if (given_digest[l] != 0) {
			break;
		}
		if ( (l+1) == MD5_DIGEST_LENGTH ) {
			fprintf(stderr, "%s: Warning: No checksum detected\n", prog);
		}
	}

	if (l != MD5_DIGEST_LENGTH) {
		/* verify checksum */
		MD5_Final(calculated_digest, &context);
		for (l = 0; l < MD5_DIGEST_LENGTH; ++l) {
			if (calculated_digest[l] != given_digest[l]) {
				fprintf(stderr, "%s: Checksum mismatch!\n", prog);

				fprintf(stderr, "%s: Calculated MD5 = ", prog);
				for (l = 0; l < MD5_DIGEST_LENGTH; ++l)
					fprintf(stderr, "%02x", calculated_digest[l]);
				fprintf(stderr, "\n");

				fprintf(stderr, "%s: Given MD5      = ", prog);
				for (l = 0; l < MD5_DIGEST_LENGTH; ++l)
					fprintf(stderr, "%02x", given_digest[l]);
				fprintf(stderr, "\n");

				exit(1);
			}
		}
		if (verbose) {
			fprintf(stderr, "%s: Checksum verified\n", prog);
		}

		if (verbose >= 2) {
			fprintf(stderr, "%s: MD5 = ", prog);
			for (l = 0; l < MD5_DIGEST_LENGTH; ++l)
				fprintf(stderr, "%02x", calculated_digest[l]);
			fprintf(stderr, "\n");
		}

	}

	/* check if final delimiter was read */
	if (is_checksum != 2) {
		fprintf(stderr, "%s: Warning: final delimiter not read, meaning text might have been cut off\n", prog);
	}

	if (text_is_from_tty) {
		free(text_ptr);
	}

}

int checkbit(char data, int bit)
{
	if ( data & (1<<bit) ) {
		return 1;
	} else {
		return 0;
	}
}

int checkbytes(char data)
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
