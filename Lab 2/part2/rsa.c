#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct uint128 {
	uint64_t hi;
	uint64_t lo;
} uint128;

typedef struct uint256 {
	uint32_t cells[8];
} uint256;

uint128
shift_right_128(uint128 a, uint8_t count) {
	for (int i = 0; i < count; i++) {
		uint64_t a_hi_lsb = a.hi & 1u;
		a.hi >>= 1u;
		a.lo >>= 1u;
		a.lo |= a_hi_lsb << 63u;
	}
	return a;
}

uint128
shift_left_128(uint128 a, uint8_t count) {
	for (int i = 0; i < count; i++) {
		uint64_t a_lo_msb = (a.lo >> 63u) & 1u;
		a.lo <<= 1u;
		a.hi <<= 1u;
		a.hi |= a_lo_msb;
	}
	return a;
}

uint256
shift_right(uint256 a, uint8_t count) {
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < 8; j++) {
			int next_cell_lsb = 0;
			if (j != 7) {
				next_cell_lsb = cells[j + 1] & 1u;
			}
            a.cells[j] >>= 1u;
            a.cells[j] |= next_cell_lsb << 31u;
		}
	}
	return a;
}

#define COMPARE_EQUAL 0
#define COMPARE_A_GREATER 1
#define COMPARE_B_GREATER 2
int8_t
compare(uint256 a, uint256 b) {
	for (int i = 7; i >= 0; i--) {
		if (a.cells[i] > b.cells[i]) {
			return COMPARE_A_GREATER;
		} else if (a.cells[i] < b.cells[i]) {
			return COMPARE_B_GREATER;
		}
	}
	return COMPARE_EQUAL;
}

int8_t
compare_128(uint128 a, uint128 b) {
	if (a.hi > b.hi) {
		return COMPARE_A_GREATER;
	} else if (a.hi < b.hi) {
		return COMPARE_B_GREATER;
	} else if (a.lo > b.lo) {
		return COMPARE_A_GREATER;
	} else if (a.lo < b.lo) {
		return COMPARE_B_GREATER;
	} else {
		return COMPARE_EQUAL;
	}
}

uint128
subtract_128(uint128 a, uint128 b) {
	// return a - b, assume a >= b
	if (a.lo < b.lo) {
		a.hi--;
		uint64_t a_lo = a.lo;
		a.lo          = (0xFFFFFFFFFFFFFFFF - b.lo) + a_lo + 1;
	} else {
		a.lo -= b.lo;
	}
	a.hi -= b.hi;
	return a;
}

uint256
subtract(uint256 a, uint256 b) {
	uint32_t borrow = 0;

	for (int i = 0; i < 8; i++) {
		uint64_t a_cell = a.cells[i];
		uint64_t b_cell = b.cells[i];
		uint64_t diff   = a_cell - b_cell - borrow;
		a.cells[i]      = (uint32_t) (diff & 0xFFFFFFFF);
		borrow          = (diff >> 32u) & 1u;
	}

	return a;
}

uint128
modulus(uint128 a, uint128 b) {
	// return a % b, assume b > 0
	if (b.hi == 0 && b.lo == 0) {
		printf("modulus by zero\n");
		while (1) {}
	}

	uint128 x      = b;
	uint128 half_a = shift_right(a, 1);

	while (compare(x, half_a) == COMPARE_B_GREATER ||
	       compare(x, half_a) == COMPARE_EQUAL) {
		x = shift_left(x, 1);
	}

	while (compare(a, b) == COMPARE_A_GREATER ||
	       compare(a, b) == COMPARE_EQUAL) {
		if (compare(a, x) == COMPARE_A_GREATER ||
		    compare(a, x) == COMPARE_EQUAL) {
			a = subtract(a, x);
		}
		x = shift_right(x, 1);
	}
	return a;
}

uint256
multiply(uint256 a, uint256 b) {
	uint256 result = {0};
	for (int i = 0; i < 8; i++) {
		uint64_t carry = 0;
		for (int j = 0; i + j < 8; j++) {
			uint64_t product = (uint64_t) a.cells[i] * (uint64_t) b.cells[j];
			uint64_t sum     = (uint64_t) result.cells[i + j] + product + carry;
			result.cells[i + j] = (uint32_t) (sum & 0xFFFFFFFFu);
			carry               = sum >> 32u;
		}
	}
	return result;
}

uint128
mod_exponentiation(uint128 base, uint128 exponent, uint128 mod) {
	uint128 result = {0, 1};

	while (exponent.hi | exponent.lo) {
		if (compare(base, mod) == COMPARE_A_GREATER ||
		    compare(base, mod) == COMPARE_EQUAL) {
			base = modulus(base, mod);
		}

		if (exponent.lo & 1) {
			// odd exponent, multiply with base using partial products
			// ignore result.hi * base.hi since it will overflow
			uint64_t pp_high = (base.hi * result.lo + base.lo * result.hi +
			                    result.lo * result.hi) /
			                   mod.lo;
			uint64_t pp_low = base.lo * result.lo;

			uint128 mod_result = modulus((uint128){pp_high, pp_low}, mod);
			result.hi          = mod_result.hi;
			result.lo          = mod_result.lo;

			exponent.lo--;
		} else {
			// even exponent, square base and divide exponent by 2
			// ignore base.hi * base.hi since it will overflow
			uint64_t pp_high =
			    (base.hi * base.lo * 2 + base.lo * base.lo) / mod.lo;
			uint64_t pp_low = base.lo * base.lo;

			uint128 mod_result = modulus((uint128){pp_high, pp_low}, mod);
			base.hi            = mod_result.hi;
			base.lo            = mod_result.lo;

			exponent.lo = (exponent.lo >> 1) | (exponent.hi << 63);
			exponent.hi >>= 1;
		}
	}
	return result;
}

int
main() {
	/* Private-Key: (128 bit) */
	/* modulus: */
	/*    00:e0:37:d3:5a:8b:16:0e:b7:f1:19:19:bf:ef:44: */
	/*    09:17 */
	/* publicExponent: 65537 (0x10001) */
	/* privateExponent: */
	/*    00:ca:b1:0c:ca:a4:43:7b:67:11:c9:77:a2:77:fe: */
	/*    00:a1 */
	/* prime1: 18125493163625818823 (0xfb8aafffd4b02ac7) */
	/* prime2: 16442969659062640433 (0xe43129c94cf45f31) */
	/* exponent1: 5189261458857000451 (0x4803f5cd8dcbfe03) */
	/* exponent2: 12850891953204883393 (0xb2578a24fdb3efc1) */
	/* coefficient: 10155582946292377246 (0x8cefe0e210c5a69e) */

	// DO NOT MODIFY
	uint128 modulus        = {0xe037d35a8b160eb7LL, 0xf11919bfef440917LL};
	uint128 privateExp     = {0x00cab10ccaa4437b67LL, 0x11c977a277fe00a1LL};
	uint64_t pubExp        = 65537;
	const char plaintext[] = "Hello !";
	uint128 ciphertext;
	uint128 decrypted;
	// END DO NOT MODIFY

	uint128 test = multiply((uint128){0, 10}, (uint128){0, 10});
	printf("test=%lu %lu\n", test.hi, test.lo);
	// ciphertext = mod_exponentiation((uint128){0, 83}, (uint128){0,
	// 7},(uint128){0, 143}); printf("ciphertext=%lu %lu\n", ciphertext.hi,
	// ciphertext.lo);
	return 0;

	/* YOUR CODE HERE: Implement RSA encryption, write the encrypted output
	 * of plaintext to ciphertext */
	uint128 message = {0, 0};
	for (int i = 0; i < strlen(plaintext); i++) {
		uint8_t high_bits = message.lo >> 56u;
		message.lo <<= 8u;
		message.lo += plaintext[i];
		message.hi <<= 8u;
		message.hi += high_bits;
	}

	ciphertext = mod_exponentiation(message, (uint128){0, pubExp}, modulus);

	// DO NOT MODIFY
	char *encrypted_text = (char *) &ciphertext;
	printf("encrypted=%s\n", encrypted_text);
	// END DO NOT MODIFY

	/* YOUR CODE HERE: Implement RSA decryption, write the decrypted output
	 * of ciphertext to decrypted */

	decrypted = mod_exponentiation(ciphertext, privateExp, modulus);

	// DO NOT MODIFY
	char *decrypted_text = (char *) &decrypted;
	printf("decrypted=%s\n", decrypted_text);
	assert(strcmp(plaintext, decrypted_text) == 0);
}
