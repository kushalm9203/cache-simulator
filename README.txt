c01 contains implementation of basic cache simulator. LRU policy for cache line replacement has been implemented.
Command to run the cache simulator (for 512B cache, associativity 2 and
block size of 16bytes) is :
	./cacheSim 512 2 16 input1.txt

c02 contains implementation of cache simmulator with a memory hierarchy (L1, L2 and a main memory) - extension of c01.
Eviction policy for read misses and writes (write-through) and No-Write Allocate policy for write misses were implemented.
Command to run the simulator for default parameters should be:
	./cacheSim 65536 4 32 131072 16 32 input1.txt
Above command has L1 cache parameters (size, associativity, block size) followed
by L2 cache parameters.