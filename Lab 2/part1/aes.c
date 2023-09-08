#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int NUM_ROUNDS = 14;
const int nk         = 8;
const int nb         = 4;

void
print_4by4(unsigned char text[]) {
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			printf("%02x ", text[row * 4 + col]);
		}
		printf("\n");
	}
}

uint8_t
galois_field_multiply(uint8_t byte1, uint8_t byte2) {
	uint8_t product = 0;
	for (int i = 0; i < 8; i++) {
		if (byte2 & 1) {
			product ^= byte1;
		}
		uint8_t carry = byte1 & 0x80;
		byte1 <<= 1;
		if (carry) {
			byte1 ^= 0x1b;
		}
		byte2 >>= 1;
	}
	return product;
}

void
calculate_round_keys(uint8_t *key, uint8_t round_keys[][4], uint8_t *s) {
	for (int i = 0; i < 8; i++) {
		round_keys[i][0] = key[i * 4];
		round_keys[i][1] = key[i * 4 + 1];
		round_keys[i][2] = key[i * 4 + 2];
		round_keys[i][3] = key[i * 4 + 3];
	}

	for (int i = nk; i < nb * (NUM_ROUNDS + 1); i++) {
		uint8_t temp[4];
		memcpy(temp, round_keys[i - 1], 4);
		if (i % nk == 0) {
			// rotate
			uint8_t temp2 = temp[0];
			for (int j = 0; j < 3; j++) { temp[j] = temp[j + 1]; }
			temp[3] = temp2;

			// substitute
			for (int j = 0; j < 4; j++) { temp[j] = s[temp[j]]; }

			// xor with Rcon[i/nk]
			// clang-format off
		    const uint8_t round_constants[] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36,0x6c,0xd8,0xab,0x4d};
			// clang-format on
			temp[0] ^= round_constants[i / nk];
		} else if (i % nk == 4) {
			// substitute
			for (int j = 0; j < 4; j++) { temp[j] = s[temp[j]]; }
		}
		// xor with previous column
		for (int j = 0; j < 4; j++) {
			round_keys[i][j] = temp[j] ^ round_keys[i - nk][j];
		}
	}
}

void
encode(uint8_t *plaintext,
       uint8_t *ciphertext_buf,
       uint8_t *key,
       uint8_t round_keys[][4],
       uint8_t *s) {
	// XOR plaintext with key
	uint8_t intermediate[16];
	for (int i = 0; i < 16; i++) { intermediate[i] = plaintext[i] ^ key[i]; }

	for (int round = 0; round < NUM_ROUNDS; round++) {
		// shuffle array with s-box s tart
		for (int i = 0; i < 16; i++) { intermediate[i] = s[intermediate[i]]; }
		// shuffle array with s-box end

		// shift array rows start
		for (int col = 0; col < 4; col++) {
			uint8_t temp[4];
			for (int i = 0; i < 4; i++) { temp[i] = intermediate[i * 4 + col]; }
			for (int rotations = 0; rotations < col; rotations++) {
				uint8_t temp2 = temp[0];
				for (int i = 0; i < 3; i++) { temp[i] = temp[i + 1]; }
				temp[3] = temp2;
			}
			for (int i = 0; i < 4; i++) { intermediate[i * 4 + col] = temp[i]; }
		}

		uint8_t temp[16];
		if (round != NUM_ROUNDS - 1) {
			// flip rows and columns
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					temp[row * 4 + col] = intermediate[col * 4 + row];
				}
			}
			memcpy(intermediate, temp, 16);

			// shift array rows end

			// mix columns start
			// clang-format off
            uint8_t galois_field[16] = {
                0x02, 0x03, 0x01, 0x01,
                0x01, 0x02, 0x03, 0x01,
                0x01, 0x01, 0x02, 0x03,
                0x03, 0x01, 0x01, 0x02
            };
			// clang-format on

			uint8_t mat_mult_result[16];
			for (int col = 0; col < 4; col++) {
				// matrix multiply against galois field
				for (int row = 0; row < 4; row++) {
					uint8_t result = 0;
					for (int i = 0; i < 4; i++) {
						uint8_t num1 = intermediate[i * 4 + col];
						uint8_t num2 = galois_field[row * 4 + i];
						result ^= galois_field_multiply(num1, num2);
					}
					mat_mult_result[row * 4 + col] = result;
				}
			}
			memcpy(intermediate, mat_mult_result, 16);

			// flip rows and columns
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					temp[row * 4 + col] = intermediate[col * 4 + row];
				}
			}
			memcpy(intermediate, temp, 16);
			// mix columns end
		}

		// add round key start
		for (int c = 0; c < nb; c++) {
			for (int r = 0; r < 4; r++) {
				intermediate[c * 4 + r] ^= round_keys[(round + 1) * nb + c][r];
			}
		}
		// add round key end
	}
	memcpy(ciphertext_buf, intermediate, 16);
}

void
decode(uint8_t *ciphertext,
       uint8_t *plaintext_buf,
       uint8_t *key,
       uint8_t round_keys[][4],
       uint8_t *s) {
	uint8_t intermediate[16];
	uint8_t s_inverse[256];

	for (int i = 0; i < 256; i++) { s_inverse[s[i]] = i; }

	// add round key
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			intermediate[c * 4 + r] =
			    ciphertext[c * 4 + r] ^ round_keys[NUM_ROUNDS * nb + c][r];
		}
	}

	for (int round = NUM_ROUNDS - 1; round >= 0; round--) {
		// inv shift array rows start
		for (int col = 0; col < 4; col++) {
			uint8_t temp[4];
			for (int i = 0; i < 4; i++) { temp[i] = intermediate[i * 4 + col]; }
			for (int rotations = 0; rotations < col; rotations++) {
				uint8_t temp2 = temp[3];
				for (int i = 3; i > 0; i--) { temp[i] = temp[i - 1]; }
				temp[0] = temp2;
			}
			for (int i = 0; i < 4; i++) { intermediate[i * 4 + col] = temp[i]; }
		}
		// inv shift array rows end

		// inv s box
		for (int i = 0; i < 16; i++) {
			intermediate[i] = s_inverse[intermediate[i]];
		}

		// add round key
		for (int c = 0; c < nb; c++) {
			for (int r = 0; r < 4; r++) {
				intermediate[c * 4 + r] ^= round_keys[round * nb + c][r];
			}
		}

		if (round != 0) {
			// inv mix columns
			// clang-format off
            uint8_t galois_field[16] = {
                0x0E, 0x0B, 0x0D, 0x09,
                0x09, 0x0E, 0x0B, 0x0D,
                0x0D, 0x09, 0x0E, 0x0B,
                0x0B, 0x0D, 0x09, 0x0E
            };
			// clang-format on

			uint8_t mat_mult_result[16];
			for (int col = 0; col < 4; col++) {
				// matrix multiply against galois field
				for (int row = 0; row < 4; row++) {
					uint8_t result = 0;
					for (int i = 0; i < 4; i++) {
						uint8_t num1 = intermediate[col * 4 + i];
						uint8_t num2 = galois_field[row * 4 + i];
						result ^= galois_field_multiply(num1, num2);
					}
					mat_mult_result[row * 4 + col] = result;
				}
			}
			memcpy(intermediate, mat_mult_result, 16);

			// flip rows and columns
			uint8_t temp[16];
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					temp[row * 4 + col] = intermediate[col * 4 + row];
				}
			}
			memcpy(intermediate, temp, 16);
		}
	}

    memcpy(plaintext_buf, intermediate, 16);
}

int
main() {
	// DO NOT MODIFY

	unsigned char s[256] = {
	    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
	    0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
	    0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
	    0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
	    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
	    0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
	    0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
	    0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
	    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
	    0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
	    0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
	    0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
	    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
	    0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
	    0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
	    0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
	    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
	    0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
	    0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
	    0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
	    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
	    0xB0, 0x54, 0xBB, 0x16};

	unsigned char enc_buf[128];
	// unsigned char plaintext[1][32] = {
	//     {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e,
	//      0x11, 0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03,
	//      0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51}};
	unsigned char ciphertext[1][32] = {
	    {0x60, 0x1e, 0xc3, 0x13, 0x77, 0x57, 0x89, 0xa5, 0xb7, 0xa7, 0xf5,
	     0x04, 0xbb, 0xf3, 0xd2, 0x28, 0xf4, 0x43, 0xe3, 0xca, 0x4d, 0x62,
	     0xb5, 0x9a, 0xca, 0x84, 0xe9, 0x90, 0xca, 0xca, 0xf5, 0xc5}};
	unsigned char plaintext[1][16] = {{0x00,
	                                   0x11,
	                                   0x22,
	                                   0x33,
	                                   0x44,
	                                   0x55,
	                                   0x66,
	                                   0x77,
	                                   0x88,
	                                   0x99,
	                                   0xaa,
	                                   0xbb,
	                                   0xcc,
	                                   0xdd,
	                                   0xee,
	                                   0xff}};
	unsigned char key[1][32]       = {
	          {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
	           0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
	           0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f}};
	// unsigned char iv[1][16] = {
	//     {0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff},
	// };
	// unsigned char key[1][32] = {
	//     {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae,
	//      0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61,
	//      0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4}};
	unsigned char decrypted_text[32];
	// END DO NOT MODIFY

	// create round key columns
	uint8_t round_keys[(NUM_ROUNDS + 1) * nb][4];
	calculate_round_keys(key[0], round_keys, s);

	// YOUR CODE HERE: Implement AES encryption, write the encrypted output of
	// plaintext to enc_buf

	encode(plaintext[0], enc_buf, key[0], round_keys, s);

	// YOUR CODE HERE: Implemente AES decryption, write the decrypted
	// output of enc_buf to decrypted_text

	decode(enc_buf, decrypted_text, key[0], round_keys, s);

	// DO NOT MODIFY
	assert(memcmp(enc_buf, ciphertext[0], 32) == 0);
	assert(memcmp(decrypted_text, plaintext[0], 32) == 0);
	return 0;
}
