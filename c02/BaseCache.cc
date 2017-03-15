#include <iostream>
#include "BaseCache.h"
#include <math.h>
#include <algorithm>

using namespace std;

//WRITE ME
//Default constructor to set everything to '0'
BaseCache::BaseCache() {
  //Not Used  
}

//WRITE ME
//Constructor to initialize cache parameters, create the cache and clears it
BaseCache::BaseCache(uint32_t _cacheSize, uint32_t _associativity, uint32_t _blockSize) {
  
  byteOffsetMask = _blockSize - 1;  
  totalNumberOfBlock = _cacheSize/_blockSize;
  totalNumberOfSets = totalNumberOfBlock/_associativity;
  blockArray.resize(totalNumberOfBlock);
  associativity = _associativity;
  
  lruRunningCounter = 1;
  
  readMissCount   = 0;
  readHitCount  = 0;
  writeHitCount   = 0;
  writeMissCount  = 0;
  
  
  unsigned int i;
  for(i=0;i<blockArray.size();i++)
  {
    blockArray[i].byteStore.resize(_blockSize);
    blockArray[i].validStore = 0; //Invalidate all block for init.
    blockArray[i].tagStore = 0;
    blockArray[i].lruCounter = 1;
  }
  
}

//WRITE ME
//Set cache base parameters
void BaseCache::setCacheSize(uint32_t _cacheSize) {}
void BaseCache::setAssociativity(uint32_t _associativity) {}
void BaseCache::setBlockSize(uint32_t _blockSize) {}

//WRITE ME
//Get cache base parameters
uint32_t BaseCache::getCacheSize() {return 0;}
uint32_t BaseCache::getAssociativity() {return 0;}
uint32_t BaseCache::getBlockSize() {return 0;}

//WRITE ME
//Get cache access statistics
uint32_t BaseCache::getReadHits() { return readHitCount;}
uint32_t BaseCache::getReadMisses() { return readMissCount;}
uint32_t BaseCache::getWriteHits() {return writeHitCount;}
uint32_t BaseCache::getWriteMisses() {return writeMissCount;}

double BaseCache::getReadHitRate() {
  double temp = (readHitCount+readMissCount);
  return (temp ==0)?0:(int)((readHitCount*100)/temp);
}
double BaseCache::getReadMissRate() {
  double temp = (readHitCount+readMissCount);
  return (temp ==0)?0:(int)((readMissCount*100)/temp);
}
double BaseCache::getWriteHitRate() {
  double temp = (writeHitCount+writeMissCount);
  return (temp ==0)?0:(int)((writeHitCount*100)/temp);
}
double BaseCache::getWriteMissRate() {
  double temp = (writeHitCount+writeMissCount);
  return (temp ==0)?0:(int)((writeMissCount*100)/temp);
}
double BaseCache::getOverallHitRate() {
  return (int)(((readHitCount + writeHitCount)*100)/(int)(readHitCount + readMissCount + writeHitCount + writeMissCount));
}
double BaseCache::getOverallMissRate() {
  return (int)(((readMissCount + writeMissCount)*100)/(int)(readHitCount + readMissCount + writeHitCount + writeMissCount));
}

void BaseCache::setReadHits()   {readHitCount++; }
void BaseCache::setReadMisses()   {readMissCount++; }
void BaseCache::setWriteHits()    {writeHitCount++; }
void BaseCache::setWriteMisses()  {writeMissCount++;}

//WRITE ME
//Initialize cache derived parameters
void BaseCache::initDerivedParams() {
  //Not Used  
  //Parameters are derivated on the fly.
}

//WRITE ME
//Reset cache access statistics
void BaseCache::resetStats() {
  //Not Used  
}

//WRITE ME
//Create cache and clear it
void BaseCache::createCache() {
  //Not Used  
}

//WRITE ME
//Reset cache
void BaseCache::clearCache() {
  //Not Used  
}

//WRITE ME
//Read data
//return true if it was a hit, false if it was a miss
//data is only valid if it was a hit, input data pointer
//is not updated upon miss. Make sure to update LRU stack
//bits. You can choose to separate the LRU bits update into
// a separate function that can be used from both read() and write().
bool BaseCache::read(uint32_t addr, uint32_t *data) {
  
  uint32_t getSetIndex = log2(byteOffsetMask+1);
  getSetIndex = (addr >> getSetIndex) & (totalNumberOfSets-1);
  
  uint32_t getTagShifter = (int)std::log2(totalNumberOfSets) + (int)std::log2(totalNumberOfSets);
  uint32_t getTag = addr >> getTagShifter;
    
  getTag = addr >> getTagShifter;
  
  //Goto that set
  uint32_t i;
  for(i=0; i< associativity; i++)
  {
    if ((blockArray[(getSetIndex*associativity) + i].tagStore == getTag) &&
      (blockArray[(getSetIndex*associativity) + i].validStore == 1))
      {
        uint32_t dataReturn;
        std::vector <uint32_t> byteTemp = blockArray[(getSetIndex*associativity) + i].byteStore;
        
        uint32_t byteIndex = addr & byteOffsetMask;
        byteIndex = byteIndex >> 2;
                
        dataReturn = byteTemp[byteIndex*4 + 0] + 
               (byteTemp[byteIndex*4 + 1]*(0x100)) + 
               (byteTemp[byteIndex*4 + 2]*(0x10000)) + 
               (byteTemp[byteIndex*4 + 3]*(0x1000000)) ;
        
        *data = dataReturn;
              
        //update LRU for this block
        blockArray[(getSetIndex*associativity) + i].lruCounter = lruRunningCounter++;
        //return hit
        setReadHits();
        return 1;
      }
  }
  
  // We are here since we did not find any block in set associativity.
  //find LRU block
  uint32_t lruMinTemp = 0xFFFFFFFF;
  for(i=0; i< associativity; i++)
  {
    lruMinTemp = min(lruMinTemp, blockArray[(getSetIndex*associativity) + i].lruCounter);
  }
  for(i=0; i< associativity; i++)
  {
    if ( lruMinTemp == blockArray[(getSetIndex*associativity) + i].lruCounter)
    {
      break;
    }
  }
  
  setReadMisses();
    return 0;
  
}

//WRITE ME
//Write data
//Function returns write hit or miss status. 
bool BaseCache::write(uint32_t addr, uint32_t data) {
  
  uint32_t getSetIndex = log2(byteOffsetMask+1);
  getSetIndex = (addr >> getSetIndex) & (totalNumberOfSets-1);
  
  uint32_t getTagShifter = (int)std::log2(totalNumberOfSets) + (int)std::log2(totalNumberOfSets);
  uint32_t getTag = addr >> getTagShifter;
  
  getTag = addr >> getTagShifter;
    
  //Goto that set
  uint32_t i;
  for(i=0; i< associativity; i++)
  {
    if ((blockArray[(getSetIndex*associativity) + i].tagStore == getTag) &&
      (blockArray[(getSetIndex*associativity) + i].validStore == 1))
      {
        std::vector <uint32_t> byteTemp = blockArray[(getSetIndex*associativity) + i].byteStore;
        
        uint32_t byteIndex = addr & byteOffsetMask;
        byteIndex = byteIndex >> 2;
        
        blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 0] = data & 0xFF;
        blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 1] = (data & 0xFF00) >> 8; 
        blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 2] = (data & 0xFF0000) >> 16; 
        blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 3] = (data & 0xFF000000) >> 24; 
        
        //Update tag in this block
        blockArray[(getSetIndex*associativity) + i].tagStore = getTag;
        //update LRU for this block
        blockArray[(getSetIndex*associativity) + i].lruCounter = lruRunningCounter++;
        //update valid bit
        blockArray[(getSetIndex*associativity) + i].validStore = 1;
        //return hit
        setWriteHits();
        return 1;
      }

  }
  
  // We are here since we did not find any block in set associativity.
  //find LRU block
  uint32_t lruMinTemp = 0xFFFFFFFF;
  for(i=0; i< associativity; i++)
  {
    lruMinTemp = min(lruMinTemp, blockArray[(getSetIndex*associativity) + i].lruCounter);
  }
  for(i=0; i< associativity; i++)
  {
    if ( lruMinTemp == blockArray[(getSetIndex*associativity) + i].lruCounter)
    {
      break;
    }
  }
  //wride data in this block
  std::vector <uint32_t> byteTemp = blockArray[(getSetIndex*associativity) + i].byteStore;
        
  uint32_t byteIndex = addr & byteOffsetMask;
  byteIndex = byteIndex >> 2;
  
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 0] = data & 0xFF;
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 1] = (data & 0xFF00) >> 8; 
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 2] = (data & 0xFF0000) >> 16; 
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 3] = (data & 0xFF000000) >> 24; 
  //update tag in this block
  blockArray[(getSetIndex*associativity) + i].tagStore = getTag;
  //update LRU for this block
  blockArray[(getSetIndex*associativity) + i].lruCounter = lruRunningCounter++;
  //update valid bit
  blockArray[(getSetIndex*associativity) + i].validStore = 1;
  // return Miss  
  setWriteMisses();
    return 0;
}

void BaseCache::readComplete(uint32_t addr, uint32_t data)
{
  uint32_t getSetIndex = log2(byteOffsetMask+1);
  getSetIndex = (addr >> getSetIndex) & (totalNumberOfSets-1);
  
  uint32_t getTagShifter = (int)std::log2(totalNumberOfSets) + (int)std::log2(totalNumberOfSets);
  uint32_t getTag = addr >> getTagShifter;
    
  getTag = addr >> getTagShifter;
  
  uint32_t lruMinTemp = 0xFFFFFFFF;
  unsigned int i;
  for(i=0; i< associativity; i++)
  {
    lruMinTemp = min(lruMinTemp, blockArray[(getSetIndex*associativity) + i].lruCounter);
  }
  for(i=0; i< associativity; i++)
  {
    if ( lruMinTemp == blockArray[(getSetIndex*associativity) + i].lruCounter)
    {
      break;
    }
  }
  
  std::vector <uint32_t> byteTemp = blockArray[(getSetIndex*associativity) + i].byteStore;
  
  uint32_t byteIndex = addr & byteOffsetMask;
  byteIndex = byteIndex >> 2;
  
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 0] = data & 0xFF;
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 1] = (data & 0xFF00) >> 8; 
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 2] = (data & 0xFF0000) >> 16; 
  blockArray[(getSetIndex*associativity) + i].byteStore[byteIndex*4 + 3] = (data & 0xFF000000) >> 24; 
        
  //update valid bit
  blockArray[(getSetIndex*associativity) + i].tagStore = getTag;
  //update LRU for this block
  blockArray[(getSetIndex*associativity) + i].lruCounter = lruRunningCounter++;
  //update valid bit
  blockArray[(getSetIndex*associativity) + i].validStore = 1;
        
  
}

//WRITE ME
//Destructor to free all allocated memeroy.
BaseCache::~BaseCache() {
  blockArray.clear();
}