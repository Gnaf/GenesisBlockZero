0.
Deps on openssl cryptolib to compile

0a. Leave genesisblock.c to default to generate your own Block 0 hash

    Set initial value of uint32_t startNonce = 2083236893 and uint32_t unixtime = 1231006505 in genesisblock.c
    to generate original Bitcoin Block 0 hash

1.
gcc genesisblock.c -o genesisgen -lcrypto

2.
./genesisgen [PubKey] [TimeStamp] [nBits]

2a.
./genesisgen 04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks" 486604799

3.
output=

Coinbase: 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73

PubkeyScript: 4104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac

Merkle Hash: 3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a

Byteswapped: 4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b


Generating block...

Hash: 000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f

Nonce: 2083236893

Unix time: 1231006505
