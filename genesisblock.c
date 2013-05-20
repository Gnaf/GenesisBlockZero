
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

//Copied from Bitcoin source
const uint64_t COIN = 100000000;
const uint64_t CENT = 1000000;

uint32_t OP_CHECKSIG = 172; // This is expressed as 0xAC
bool generateBlock = false;
uint32_t startNonce = 0;
uint32_t unixtime = 0;

typedef struct {
	/* Hash of Tx */
	uint8_t merkleHash[32];
	
	/* Tx serialization before hashing */
	uint8_t *serializedData;
	
	/* Tx version */
	uint32_t version;
	
	/* Input */
	uint8_t numInputs; // Program assumes one input
	uint8_t prevOutput[32];
	uint32_t prevoutIndex;
	uint8_t *scriptSig;
	uint32_t sequence;
	
	/* Output */
	uint8_t numOutputs; // Program assumes one output
	uint64_t outValue;
	uint8_t *pubkeyScript;
	
	/* Final */
	uint32_t locktime;
} Transaction;

// Got this off the internet. Am not sure if it can fail in some circumstances
void byteswap(uint8_t *buf, int length)
{
	int i;
	uint8_t temp;

	for(i = 0; i < length / 2; i++)
	{
		temp = buf[i];
		buf[i] = buf[length - i - 1];
		buf[length - i - 1] = temp;
	}   
}

// Following two functions are borrowed from cgminer.
char *bin2hex(const unsigned char *p, size_t len)
{
	char *s = malloc((len * 2) + 1);
	unsigned int i;

	if (!s)
		return NULL;

	for (i = 0; i < len; i++)
		sprintf(s + (i * 2), "%02x", (unsigned int) p[i]);

	return s;
}

size_t hex2bin(unsigned char *p, const char *hexstr, size_t len)
{
	int ret = 0;
	size_t retlen = len;

	while (*hexstr && len) {
		char hex_byte[4];
		unsigned int v;

		if (!hexstr[1]) {
			return ret;
		}

		memset(hex_byte, 0, 4);
		hex_byte[0] = hexstr[0];
		hex_byte[1] = hexstr[1];

		if (sscanf(hex_byte, "%x", &v) != 1) {
			return ret;
		}

		*p = (unsigned char) v;

		p++;
		hexstr += 2;
		len--;
	}

	if (len == 0 && *hexstr == 0)
		ret = retlen;
		
	return ret;
}

Transaction *InitTransaction()
{
	Transaction *transaction;
	
	transaction = calloc(1, sizeof(*transaction));
	if(!transaction)
	{
		return NULL;
	}
	
	// Set some initial data that will remain constant throughout the program
	transaction->version = 1;
	transaction->numInputs = 1;
	transaction->numOutputs = 1;
	transaction->locktime = 0;
	transaction->prevoutIndex = 0xFFFFFFFF;
	transaction->sequence = 0xFFFFFFFF;
	transaction->outValue = 50*COIN;
	
	// We initialize the previous output to 0 as there is none
	memset(transaction->prevOutput, 0, 32);
	
	return transaction;
}

int main(int argc, char *argv[])
{
	Transaction *transaction;
	unsigned char hash1[32], hash2[32];
	char timestamp[255], pubkey[132];
	uint32_t timestamp_len = 0, scriptSig_len = 0, pubkey_len = 0, pubkeyScript_len = 0;
	uint32_t nBits = 0;
	
	if((argc-1) < 3)
	{
		fprintf(stderr, "Usage: genesisgen [options] <pubkey> \"<timestamp>\" <nBits>\n");
		return 0;		
	}
	
	pubkey_len = strlen(argv[1]) / 2; // One byte is represented as two hex characters, thus we divide by two to get real length.
	timestamp_len = strlen(argv[2]);
	
	if(pubkey_len != 65)
	{
		fprintf(stderr, "Invalid public key length! %s\n", argv[1]);
		return 0;
	}
	
	if(timestamp_len > 254 || timestamp_len <= 0)
	{
		fprintf(stderr, "Size of timestamp is 0 or exceeds maximum length of 254 characters!\n");
		return 0;
	}	

	transaction = InitTransaction();
	if(!transaction)
	{
		fprintf(stderr, "Could not allocate memory! Exiting...\n");
		return 0;	
	}
	
	strncpy(pubkey, argv[1], sizeof(pubkey));
	strncpy(timestamp, argv[2], sizeof(timestamp));
	sscanf(argv[3], "%lu", (long unsigned int *)&nBits);
	
	pubkey_len = strlen(pubkey) >> 1;
	scriptSig_len = timestamp_len;
	
	// Encode pubkey to binary and prepend pubkey size, then append the OP_CHECKSIG byte
	transaction->pubkeyScript = malloc((pubkey_len+2)*sizeof(uint8_t));
	pubkeyScript_len = hex2bin(transaction->pubkeyScript+1, pubkey, pubkey_len); // No error checking, yeah.
	transaction->pubkeyScript[0] = 0x41; // A public key is 32 bytes X coordinate, 32 bytes Y coordinate and one byte 0x04, so 65 bytes i.e 0x41 in Hex.
	pubkeyScript_len+=1;
	transaction->pubkeyScript[pubkeyScript_len++] = OP_CHECKSIG;
	
	// Encode timestamp to binary
	transaction->scriptSig = malloc(scriptSig_len*sizeof(uint8_t));
	uint32_t scriptSig_pos = 0;
	
	
	// This is basically how I believe the size of the nBits is calculated
	if(nBits <= 255)
	{
		transaction->scriptSig[scriptSig_pos++] = 0x01;
		transaction->scriptSig[scriptSig_pos++] = (uint8_t)nBits;
	}
	else if(nBits <= 65535)
	{
		transaction->scriptSig[scriptSig_pos++] = 0x02;
		memcpy(transaction->scriptSig+scriptSig_pos, &nBits, 2);
		scriptSig_pos+=2;
	}	
	else if(nBits <= 16777215)
	{
		transaction->scriptSig[scriptSig_pos++] = 0x03;
		memcpy(transaction->scriptSig+scriptSig_pos, &nBits, 3);
		scriptSig_pos+=3;
	}
	else //else if(nBits <= 4294967296LL)
	{
		transaction->scriptSig[scriptSig_pos++] = 0x04;
		memcpy(transaction->scriptSig+scriptSig_pos, &nBits, 4);
		scriptSig_pos+=4;
	}
	
	// Important! In the Bitcoin code there is a statement 'CBigNum(4)' 
	// i've been wondering for a while what it is but
	// seeing as alt-coins keep it the same, we'll do it here as well
	// It should essentially mean PUSH 1 byte on the stack which in this case is 0x04 or just 4
	transaction->scriptSig[scriptSig_pos++] = 0x01;
	transaction->scriptSig[scriptSig_pos++] = 0x04;
	
	transaction->scriptSig[scriptSig_pos++] = (uint8_t)scriptSig_len;
	
	scriptSig_len += scriptSig_pos;
	transaction->scriptSig = realloc(transaction->scriptSig, scriptSig_len*sizeof(uint8_t));
	memcpy(transaction->scriptSig+scriptSig_pos, (const unsigned char *)timestamp, timestamp_len);
	
	// Here we are asuming some values will have the same size
	uint32_t serializedLen = 
	4    // tx version
	+1   // number of inputs
	+32  // hash of previous output
	+4   // previous output's index
	+1   // 1 byte for the size of scriptSig
	+scriptSig_len
	+4   // size of sequence
	+1   // number of outputs
	+8   // 8 bytes for coin value
	+1   // 1 byte to represent size of the pubkey Script
	+pubkeyScript_len
	+4;   // 4 bytes for lock time
	
	// Now let's serialize the data
	uint32_t serializedData_pos = 0;
	transaction->serializedData = malloc(serializedLen*sizeof(uint8_t));
	memcpy(transaction->serializedData+serializedData_pos, &transaction->version, 4);
	serializedData_pos += 4;
	memcpy(transaction->serializedData+serializedData_pos, &transaction->numInputs, 1);
	serializedData_pos += 1;
	memcpy(transaction->serializedData+serializedData_pos, transaction->prevOutput, 32);
	serializedData_pos += 32;
	memcpy(transaction->serializedData+serializedData_pos, &transaction->prevoutIndex, 4);
	serializedData_pos += 4;
	memcpy(transaction->serializedData+serializedData_pos, &scriptSig_len, 1);
	serializedData_pos += 1;
	memcpy(transaction->serializedData+serializedData_pos, transaction->scriptSig, scriptSig_len);
	serializedData_pos += scriptSig_len;
	memcpy(transaction->serializedData+serializedData_pos, &transaction->sequence, 4);
	serializedData_pos += 4;
	memcpy(transaction->serializedData+serializedData_pos, &transaction->numOutputs, 1);
	serializedData_pos += 1;
	memcpy(transaction->serializedData+serializedData_pos, &transaction->outValue, 8);
	serializedData_pos += 8;
	memcpy(transaction->serializedData+serializedData_pos, &pubkeyScript_len, 1);
	serializedData_pos += 1;
	memcpy(transaction->serializedData+serializedData_pos, transaction->pubkeyScript, pubkeyScript_len);
	serializedData_pos += pubkeyScript_len;
	memcpy(transaction->serializedData+serializedData_pos, &transaction->locktime, 4);
	serializedData_pos += 4;	
	
	// Now that the data is serialized
	// we hash it with SHA256 and then hash that result to get merkle hash
	SHA256(transaction->serializedData, serializedLen, hash1);
	SHA256(hash1, 32, hash2);
	
	// This copy isn't necessary imo, but here for clarity
	memcpy(transaction->merkleHash, hash2, 32);
	
	char *merkleHash = bin2hex(transaction->merkleHash, 32);
	byteswap(transaction->merkleHash, 32); 
	char *merkleHashSwapped = bin2hex(transaction->merkleHash, 32);
	char *txScriptSig = bin2hex(transaction->scriptSig, scriptSig_len);
	char *pubScriptSig = bin2hex(transaction->pubkeyScript, pubkeyScript_len);
	printf("\nCoinbase: %s\n\nPubkeyScript: %s\n\nMerkle Hash: %s\nByteswapped: %s\n",txScriptSig, pubScriptSig, merkleHash, merkleHashSwapped);
	
	//if(generateBlock)
	{
		printf("Generating block...\n");
		if(!unixtime)
		{
			unixtime = time(NULL);
		}
		
		unsigned char block_header[80], block_hash1[32], block_hash2[32];
		uint32_t blockversion = 1;
		memcpy(block_header, &blockversion, 4);
		memset(block_header+4, 0, 32);
		byteswap(transaction->merkleHash, 32); // We swapped it before, so do it again now.
		memcpy(block_header+36, transaction->merkleHash, 32);
		memcpy(block_header+68, &unixtime, 4);
		memcpy(block_header+72, &nBits, 4);
		memcpy(block_header+76, &startNonce, 4);
		
		uint32_t *pNonce = (uint32_t *)(block_header + 76);
		uint32_t *pUnixtime = (uint32_t *)(block_header + 68);
		unsigned int counter, start = time(NULL);
		while(1)
		{
			SHA256(block_header, 80, block_hash1);
			SHA256(block_hash1, 32, block_hash2);
			
			unsigned int check = *((uint32_t *)(block_hash2 + 28)); // The hash is in little-endian, so we check the last 4 bytes.
			if(check == 0) // \x00\x00\x00\x00
			{
				byteswap(block_hash2, 32);
				char *blockHash = bin2hex(block_hash2, 32);
				printf("\nBlock found!\nHash: %s\nNonce: %u\nUnix time: %u", blockHash, startNonce, unixtime);
				free(blockHash);
				break;
			}
			
			startNonce++;
			counter+=1;
			if(time(NULL)-start >= 1)
			{
				printf("\r%d Hashes/s, Nonce %u\r", counter, startNonce);
				counter = 0;
				start = time(NULL);
			}
			*pNonce = startNonce;
			if(startNonce > 4294967294LL)
			{
				//printf("\nBlock found!\nHash: %s\nNonce: %u\nUnix time: %u", blockHash, startNonce, unixtime);
				unixtime++;
				*pUnixtime = unixtime;
				startNonce = 0;
			}
		}
	}
	
		
	// Lots of cleanup
	free(merkleHash);
	free(merkleHashSwapped);
	free(txScriptSig);
	free(pubScriptSig);
	free(transaction->serializedData);
	free(transaction->scriptSig);
	free(transaction->pubkeyScript);
	free(transaction);

	return 0;
}
