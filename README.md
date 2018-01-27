0.
Deps on openssl cryptolib to compile

build 64-bit openssl on windows with nmake , copy `libcrypto.lib` to say  `C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib\x64`  
also copy its .h files to `C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include\openssl`
and dlls to `C:\Windows\System32`  

0a. Leave genesisblock.cpp to default to generate your own Block 0 hash

    Set initial value of uint32_t startNonce = 2083236893 and uint32_t unixtime = 1231006505 in genesisblock.cpp 
    to generate original Bitcoin Block 0 hash

1.
build solution 

libux build (for non-pc - req unaligned memory access) : 
```
cd 7I
c++ genesisblock.cpp -o genesisblock -lcrypto
```
2.
genesisblock [PubKey] [TimeStamp] [nBits]

2a.  
windows
```
genesisblock 04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks" 486604799
```
linux
```
./genesisblock 04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks" 486604799
```
3.
![Screen1](/screens/Untitled%201.jpg)
it's from broken version ,ofcourse
