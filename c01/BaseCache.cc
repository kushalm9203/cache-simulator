#include <iostream>
#include "BaseCache.h"
#include <cmath>
using namespace std;

//WRITE ME
//Default constructor to set everything to '0'
BaseCache::BaseCache() {
  cacheSize=0;
  associativity=0;
  blockSize=0;
}

//WRITE ME
//Constructor to initialize cache parameters, create the cache and clears it
BaseCache::BaseCache(uint32_t _cacheSize, uint32_t _associativity, uint32_t _blockSize) {
  cacheSize=_cacheSize;
  associativity=_associativity;
  blockSize=_blockSize;
  initDerivedParams();
  createCache();
  clearCache();
  resetStats();
}

//WRITE ME
//Set cache base parameters
void BaseCache::setCacheSize(uint32_t _cacheSize) { cacheSize=_cacheSize; }
void BaseCache::setAssociativity(uint32_t _associativity) { associativity=_associativity;}
void BaseCache::setBlockSize(uint32_t _blockSize) { blockSize=_blockSize; }

//WRITE ME
//Get cache base parameters
uint32_t BaseCache::getCacheSize() { return cacheSize; }
uint32_t BaseCache::getAssociativity() { return associativity; }
uint32_t BaseCache::getBlockSize() { return blockSize; }

//WRITE ME
//Get cache access statistics
uint32_t BaseCache::getReadHits() { return numReadHits;}
uint32_t BaseCache::getReadMisses() { return numReadMisses; }
uint32_t BaseCache::getWriteHits() { return numWriteHits; }
uint32_t BaseCache::getWriteMisses() { return numWriteMisses;}
double BaseCache::getReadHitRate() { return ((numReadHits*100)/numReads); }
double BaseCache::getReadMissRate() { return ((numReadMisses*100)/numReads); }
double BaseCache::getWriteHitRate() { return ((numWriteHits*100)/numWrites); }
double BaseCache::getWriteMissRate() { return ((numWriteMisses*100)/numWrites); }
double BaseCache::getOverallHitRate() { return (((numReadHits+numWriteHits)*100)/(numReads+numWrites));}
double BaseCache::getOverallMissRate() {  return (((numReadMisses+numWriteMisses)*100)/(numReads+numWrites));}

//WRITE ME
//Initialize cache derived parameters
void BaseCache::initDerivedParams() {
  n_sets=(cacheSize/(associativity*blockSize));
  n_ways=associativity;
  n_indexbits=(log(n_sets)/log(2));
  n_blockoffset=(log(blockSize/4)/log(2));
  n_byteoffset=2;
  n_tagbits=(32-n_indexbits-n_blockoffset-n_byteoffset);
}

//WRITE ME
//Reset cache access statistics
void BaseCache::resetStats() {
    numReads=0;
  numWrites=0;
  numReadHits=0;
  numReadMisses=0;
  numWriteHits=0;
  numWriteMisses=0;
}

//WRITE ME
//Create cache and clear it
void BaseCache::createCache() {
  cacheLines=new cacheLine*[n_sets];
  for(unsigned int r=0;r<n_sets;r++) {
  cacheLines[r]=new cacheLine[n_ways];
 }
  for(unsigned int r=0;r<n_sets;r++) {
    for (unsigned int c=0;c<n_ways;c++) {
      cacheLines[r][c].data=new uint32_t[blockSize/4];
  }
  }
}

//WRITE ME
//Reset cache
void BaseCache::clearCache() {
  for(unsigned int r=0;r<n_sets;r++) {
      for(unsigned int c=0;c<n_ways;c++)  {
    cacheLines[r][c].tag=0;
    cacheLines[r][c].LRUStackBits=0;
    for(unsigned int i=0;i<(blockSize/4);i++) {
      cacheLines[r][c].data[i]=0;
    }
      }
    }
}

uint32_t BaseCache::getbintag(uint32_t addr) {
  uint32_t temp=1;
  uint32_t temp1=0;
  for(int i=31;i>=(32-n_tagbits);i--) {
    temp <<= i;
    temp1 |= temp;
    temp=1;
  }
  temp=addr;
  temp &= temp1;
  temp >>= (n_indexbits+n_blockoffset+n_byteoffset);
  return temp;
}

uint32_t BaseCache::getdecindex(uint32_t addr) {
  uint32_t temp1=0;
  uint32_t temp=1;
  for(int i=31-n_tagbits;i>=32-n_tagbits-n_indexbits;i--) {
    temp <<= i;
    temp1 |= temp;
    temp=1;
 }
  temp=addr;
  temp &= temp1;
  temp >>= n_blockoffset+n_byteoffset;
  // temp=bintodec(temp);
  return temp;
}
/*
uint32_t bintodec(uint32_t index) {
  uint32_t res=0;
  for(int i=0;index>0;++i) {
    if((index%10)==1)
      res+= (1<<i);
    
    index /=10;
  }
  return res;
  } */

uint32_t BaseCache::getdecblock(uint32_t addr) {
  uint32_t temp1=0;
  uint32_t temp=1;
  for(int i=(31-n_tagbits-n_indexbits);i>=2;i--) {
    temp <<= i;
    temp1 |= temp;
    temp=1;
  }
  temp=addr;
  temp &= temp1;
  temp >>= 2;
  //  temp=bintodec(temp);
  return temp;
}

void BaseCache::lruupdate(unsigned int index,unsigned int i) {
  // cacheLines[index][i].LRUStackBits=(n_ways-1);
  for(unsigned int j=0;j<n_ways;j++) {
    if(cacheLines[index][j].LRUStackBits>cacheLines[index][i].LRUStackBits) {
      cacheLines[index][j].LRUStackBits--;
    }
  }
  cacheLines[index][i].LRUStackBits=(n_ways-1);
}

//WRITE ME
//Read data
//return true if it was a hit, false if it was a miss
//data is only valid if it was a hit, input data pointer
//is not updated upon miss. Make sure to update LRU stack
//bits. You can choose to separate the LRU bits update into
// a separate function that can be used from both read() and write().
bool BaseCache::read(uint32_t addr, uint32_t *data) {
  tag=getbintag(addr);
  index=getdecindex(addr);
  blockoffset=getdecblock(addr);
  for(unsigned int i=0;i<n_ways;i++) {
    if(cacheLines[index][i].tag==tag) {
      lruupdate(index,i);
      *data=(cacheLines[index][i].data[blockoffset]);
      numReads++;
      numReadHits++;
      return true;
    }
  }
  
   for(unsigned int i=0;i<n_ways;i++) {
    if(cacheLines[index][i].LRUStackBits==0) {
      cacheLines[index][i].tag = tag;
      lruupdate(index,i);
      *data=cacheLines[index][i].data[blockoffset];
      numReads++;
      numReadMisses++;
    return false;
    }
  }

  for(unsigned int i=0;i<n_ways;i++) {
    if(cacheLines[index][i].data==NULL) {
      cacheLines[index][i].tag = tag;
      *data=(cacheLines[index][i].data[blockoffset]);
      lruupdate(index,i);
      numReads++;
      numReadMisses++;
      return false;
    }
  }
  return false;
 
}
//WRITE ME
//Write data
//Function returns write hit or miss status. 
bool BaseCache::write(uint32_t addr, uint32_t data) {
  tag=getbintag(addr);
  index=getdecindex(addr);
  blockoffset=getdecblock(addr);
  for(unsigned int i=0;i<n_ways;i++) {
    if(cacheLines[index][i].tag==tag) {
      cacheLines[index][i].data[blockoffset]=data;
      lruupdate(index,i);
      numWrites++;
      numWriteHits++;
      return true;
    }
  }
  for(unsigned int i=0;i<n_ways;i++) {
    if(cacheLines[index][i].LRUStackBits==0) {
      (cacheLines[index][i].data)[blockoffset]=data;
      cacheLines[index][i].tag=tag;
       lruupdate(index,i);
       numWrites++;
       numWriteMisses++;
       return false;
    }
   }

  for(unsigned int i=0;i<n_ways;i++) {
    if(cacheLines[index][i].data==NULL) {
      cacheLines[index][i].tag=tag;
      (cacheLines[index][i].data)[blockoffset]=data;
  lruupdate(index,i);
  numWrites++;
  numWriteMisses++;
      return false;
    }
  }
  return false;
}

//WRITE ME
//Destructor to free all allocated memeroy.
BaseCache::~BaseCache() {

}