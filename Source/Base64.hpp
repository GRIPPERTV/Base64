#ifndef __BASE64__
#define __BASE64__

// Macros
#include "MyMacros.h"

namespace Base64 {
	constorexpr unsigned char EncodingTable[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
		'w', 'x', 'y', 'z', '0', '1', '2', '3', 
		'4', '5', '6', '7', '8', '9',
		
		#ifdef BASE64_URL
			'-', '_'
		#else
			'+', '/'
		#endif
	};

	constorexpr unsigned char DecodingTable[] = {
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 0,  62, 0,  63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
		0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
		0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0,  0,  0,  0,  0
	};

	/* 3 bytes (characters) represent 6 bits, 4x6 = 24 bits
	 that is 3 bytes, so just 4xLength and divide to 3 for
	 bytes representation

	 The addition of 3 and comparing ~3 is a method to round
	 to the next greater multiple of 4 for padding addition */
	inline constexpr size_t EncodeLength(const size_t& Length) {
		return ((4 * (Length / 3)) + 3) & ~3;
	}

	// Couldn't find a way to calculate the length without padding, so here's my solution
	constexpr14 size_t DecodeLength(char *Input, const size_t& Length) {
		unsigned char Padding = 0;

		if (Input[Length - 1] == '=') {
			Padding = 2;
		} else if (Input[Length] == '=') {
			Padding = 1;
		}

		return (3 * (Length / 4)) - Padding;
	}

	constexpr14 bool Encode3Bytes(char *Output, const char *Input, const size_t& Length) {
		unsigned char One = Input[0], Two = Input[1];

		/* [...] The next 4 bytes output is a 6 bit group of
		 the current input bytes translated into a character
		 of Base64 alphabet [...]

		 You can see how this is done here: https://t.ly/92w7 */

		Output[0] = EncodingTable[One >> 2];
		Output[1] = EncodingTable[((One & 0x03) << 4) | (Two >> 4)];

		/* Pad the next 2 final bytes if Length is 1,
		 because always a data of 1 byte is encoded with
		 the 2 final bytes padded. */
		if (Length == 1) {
			Output[2] = '=';
			Output[3] = '=';
		} else {
			unsigned char Three = Input[2];
			Output[2] = EncodingTable[((Two & 0x0f) << 2) | (Three >> 6)];

			/* By Length > 1, the only way for padding the last
			 byte is Length being 2, because if the Length is 3,
			 already have enough data for complete encoding. */
			if (Length == 2) {
				Output[3] = '=';
			} else {
				Output[3] = EncodingTable[Three & 0x3f];
			}
		}

		Output += 4;
		*Output = 0;
		return 0;
	}

	// See Encode3Bytes for more information
	constexpr14 bool Encode(char *Output, const char *Input, size_t Length) {
		unsigned char One = 0, Two = 0, Three = 0;

		/* By padding being only applied to the last 3 bytes,
		 then create a loop checking if the length is more
		 than 3 and decrement it to 3 each loop, without the
		 checking for padding in the loop body, so it's more
		 efficient because it's not necessary a check every
		 loop. */
		for (; Length > 3; Length -= 3) {
			One = Input[0]; Two = Input[1]; Three = Input[2];


			Output[0] = EncodingTable[One >> 2];
			Output[1] = EncodingTable[((One & 0x03) << 4) | (Two >> 4)];
			Output[2] = EncodingTable[((Two & 0x0f) << 2) | (Three >> 6)];
			Output[3] = EncodingTable[Three & 0x3f];

			Input += 3; Output += 4;
		}

		return Encode3Bytes(Output, Input, Length);
	}

	constexpr14 bool Decode4Bytes(char *Output, const char *Input) {
		unsigned char One = DecodingTable[Input[0]], Two = DecodingTable[Input[1]];
		unsigned char Three = DecodingTable[Input[2]], Four = DecodingTable[Input[3]];

		// The same as encoding, but joint the 6 bits into 8.
		Output[0] = (One << 2) | (Two >> 4);

		if (Three) {
			Output[1] = (Two << 4) | (Three >> 2);

			if (Four) {
				Output[2] = (Three << 6) | Four;
				Output += 1;
			}

			Output += 1;
		}

		Output += 1;
		*Output = 0;
		return 0;
	}

	// See Encode and Decode4Bytes for more information
	constexpr14 bool Decode(char *Output, const char *Input, size_t Length) {
		unsigned char One = 0, Two = 0, Three = 0, Four = 0;

		for (; Length > 4; Length -= 4) {
			One = DecodingTable[Input[0]], Two = DecodingTable[Input[1]];
			Three = DecodingTable[Input[2]], Four = DecodingTable[Input[3]];

			Output[0] = (One << 2) | (Two >> 4);
			Output[1] = (Two << 4) | (Three >> 2);
			Output[2] = (Three << 6) | Four;

			Input += 4; Output += 3;
		}

		return Decode4Bytes(Output, Input);
	}
}

#endif
