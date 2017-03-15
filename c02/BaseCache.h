#ifndef __BASE_CACHE_H__
#define __BASE_CACHE_H__

#include <iostream>
#include <vector>

#define ADDR_BITS 32

// We dont use this structure
typedef struct _cacheLine {
    uint32_t tag;
    uint32_t *data; // data points to data stored in 
    		    // cache line. cache line size is
    		    // multiple of 32bits.
    uint32_t LRUStackBits;
    		    // These bits keep track of usage position
    		    // of cache line in LRU stack. MRU cache
    		    // line is represented by the top of the 
    		    // stack, while LRU is the bottom of the
    		    // stack. 'LRUStackBits=0' represents the
    		    // LRU cache line. 
} cacheLine;

// Block structure
typedef struct blockStruct_t {
	uint32_t tagStore;
	uint32_t validStore;
	uint32_t lruCounter;
	std::vector <uint32_t> byteStore;
}blockUnit;

class BaseCache{
    private:
    	//cache base parameters
	uint32_t cacheSize; //in Bytes
	uint32_t associativity;
	uint32_t blockSize;  //in Bytes
	cacheLine **cacheLines;

	//cache access statistics
	uint32_t numReads;
	uint32_t numWrites;
	uint32_t numReadHits;
	uint32_t numReadMisses;
	uint32_t numWriteHits;
	uint32_t numWriteMisses;
	
	// Internal bits
	uint32_t byteOffsetMask;
	uint32_t setIndexMask;
	uint32_t tagMask;
	uint32_t totalNumberOfBlock;
	uint32_t lruRunningCounter;
	uint32_t totalNumberOfSets;
	
	uint32_t readMissCount;
	uint32_t readHitCount;
	uint32_t writeHitCount;
	uint32_t writeMissCount;
	
	
	std::vector<blockUnit> blockArray;

    public:
	//Default constructor to set everything to '0'
	BaseCache();        
	//Constructor to initialize cache parameters and to create the cache 
	BaseCache(uint32_t _cacheSize, uint32_t _associativity, uint32_t _blockSize);
	//Set cache base parameters
	void setCacheSize(uint32_t _cacheSize); 
	void setAssociativity(uint32_t _associativity);
	void setBlockSize(uint32_t _blockSize);

	//Get cache base parameters
	uint32_t getCacheSize(); 
	uint32_t getAssociativity(); 
	uint32_t getBlockSize(); 
	
	//Get cache access statistics
	uint32_t getReadHits(); 
	uint32_t getReadMisses(); 
	uint32_t getWriteHits(); 
	uint32_t getWriteMisses(); 
	double getReadHitRate();
	double getReadMissRate();
	double getWriteHitRate();
	double getWriteMissRate();
	double getOverallHitRate();
	double getOverallMissRate();
	
	void setReadHits();
	void setReadMisses();
	void setWriteHits();
	void setWriteMisses();

	//Initialize cache derived parameters
	void initDerivedParams();
	//Reset cache access statistics
	void resetStats();
	//Create cache and clear it
	void createCache();
	//Reset cache
	void clearCache();
	//Read data
	//return true if it was a hit, false if it was a miss
	//data is only valid if it was a hit, input data pointer
	//is not updated upon miss.
	bool read(uint32_t addr, uint32_t *data);
	//Write data
	//Function returns write hit or miss status. 
	bool write(uint32_t addr, uint32_t data);

	void readComplete(uint32_t addr, uint32_t data);
	/********ADD ANY ADDITIONAL METHODS IF REQUIRED*********/
	
	//Destructor to free all allocated memeroy.
	~BaseCache();
};
#endif