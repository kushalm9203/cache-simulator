#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "BaseCache.h"
using namespace std;

void BaseCachemainwrite(uint32_t address, uint32_t data, fstream& fp) {
  string inp_line, parsed_line;
  uint32_t temp_add;
  while(getline(fp, inp_line)) {
    stringstream inp_string(inp_line);
    if(getline(inp_string,parsed_line,'\t')) {
      temp_add=stoul(parsed_line,NULL,16);
      if(temp_add==address) {
	fp<<data<<'\n';
	break;
      }
    }
  }
}

unsigned int BaseCachemainread(uint32_t address,uint32_t *data, fstream& fp,int numreads) {
  numreads++;
  string inp_line, parsed_line;
  uint32_t temp_add,temp_data;
  while(getline(fp,inp_line)) {
    stringstream inp_string(inp_line);
    if(getline(inp_string,parsed_line,'\t')) {
      temp_add=stoul(parsed_line,NULL,16);
      if(temp_add==address) {
	getline(inp_string,parsed_line,'\n');
	temp_data=stoul(parsed_line);
	*data=temp_data;
	break;
      }
    }
  }
  return numreads;
}

int main(int argc, char **argv) {
    string inp_line, parsed_line;
    string command;
    int min,max;
    unsigned int numreads;
    numreads=0;
    unsigned int address, data;
    uint32_t L1_cacheSize, L1_associativity, L1_blockSize, L2_cacheSize, L2_associativity, L2_blockSize;
    L1_cacheSize = atoi(argv[1]);
    L1_associativity = atoi(argv[2]);
    L1_blockSize = atoi(argv[3]);
    L2_cacheSize = atoi(argv[4]);
    L2_associativity = atoi(argv[5]);
    L2_blockSize = atoi(argv[6]);
    ifstream fp_inp(argv[7]);
    fstream fp("memfoot.dat");
    if(!fp.is_open()) {
      cout<<"Error";
      return EXIT_FAILURE;
    }
    BaseCache BaseCacheL1(L1_cacheSize, L1_associativity, L1_blockSize);
    BaseCache BaseCacheL2(L2_cacheSize, L2_associativity, L2_blockSize);
  
    if(fp_inp.is_open()) {
        while(getline(fp_inp, inp_line)) {
            stringstream inp_string(inp_line);
    	    if(getline(inp_string,parsed_line, ' ')) 
		command = parsed_line;
	    if(getline(inp_string,parsed_line, ' ')) 
		address = stoul(parsed_line, NULL, 16);
	    if (!(command.compare("w"))) {
	    	if(getline(inp_string,parsed_line, ' ')) {
		    data = stoul(parsed_line, NULL, 16);
		}
	    }
	   
	    //Issue read/write command
	    if (!(command.compare("w"))) { 
	        if(BaseCacheL1.write(address, data)) {
		  BaseCacheL2.write(address, data);
		  BaseCachemainwrite(address,data,fp);
	       	}
	    else {
	      if(BaseCacheL2.write(address, data)) {
	      	BaseCachemainwrite(address,data,fp);
	      }
	      else {
	       	BaseCachemainwrite(address,data,fp);
		 }
	    }
	    }
	    if (!(command.compare("r"))) { 
	        if(BaseCacheL1.read(address, &data)) {
	        }
		else if(BaseCacheL2.read(address, &data)){
		    BaseCacheL1.write(address, data);
		    BaseCacheL1.dec();
		  }
       		  else {
		   numreads=BaseCachemainread(address, &data,fp,numreads);
		    BaseCacheL2.write(address, data);
		    BaseCacheL1.write(address, data);
		    BaseCacheL1.dec();
		    BaseCacheL2.dec();
		  }
		}
	}
    
		fp_inp.close();
    }
    cout <<endl;
    cout << "L1 Read Hits (HitRate): "<<BaseCacheL1.getReadHits()<<" ("<<BaseCacheL1.getReadHitRate()<<"%)"<<endl;
    cout << "L1 Read Misses (MissRate): "<<BaseCacheL1.getReadMisses() <<" ("<<BaseCacheL1.getReadMissRate()<<"%)"<<endl;
    cout << "L1 Write Hits (HitRate): "<<BaseCacheL1.getWriteHits()<<" ("<<BaseCacheL1.getWriteHitRate()<<"%)"<<endl;
    cout << "L1 Write Misses (MissRate): "<<BaseCacheL1.getWriteMisses() <<" ("<<BaseCacheL1.getWriteMissRate()<<"%)"<<endl;
    cout << "L1 Overall Hit Rate: "<<BaseCacheL1.getOverallHitRate() <<"%" << endl;
    cout << "L1 Overall Miss Rate: "<<BaseCacheL1.getOverallMissRate()<<"%"<<endl;
 cout <<endl;
    cout << "L2 Read Hits (HitRate): "<<BaseCacheL2.getReadHits()<<" ("<<BaseCacheL2.getReadHitRate()<<"%)"<<endl;
    cout << "L2 Read Misses (MissRate): "<<BaseCacheL2.getReadMisses() <<" ("<<BaseCacheL2.getReadMissRate()<<"%)"<<endl;
    cout << "L2 Write Hits (HitRate): "<<BaseCacheL2.getWriteHits()<<" ("<<BaseCacheL2.getWriteHitRate()<<"%)"<<endl;
    cout << "L2 Write Misses (MissRate): "<<BaseCacheL2.getWriteMisses() <<" ("<<BaseCacheL2.getWriteMissRate()<<"%)"<<endl;
    cout << "L2 Overall Hit Rate: "<<BaseCacheL2.getOverallHitRate() <<"%" << endl;
    cout << "L2 Overall Miss Rate: "<<BaseCacheL2.getOverallMissRate()<<"%"<<endl;
     cout <<endl;
     if(BaseCacheL1.getReadHits()>0) {
	 min=1;
     }
      else if(BaseCacheL2.getReadHits()>0) {
       min =16;
     }
     else
       min =86;
     
     if(numreads>0) {
       max=86;
	 }
    else if(BaseCacheL2.getReadHits()>0) {
       max =16;
     }
     else
       max=1;
     int temp = (70*(BaseCacheL2.getReadMissRate()/100));
     int amat = (1 + ((BaseCacheL1.getReadMissRate()/100)*(15+temp)));
     cout << "Average memory access time (AMAT) (Reads): "<<amat<<"ns"<<endl;
    cout << "Minimum access time: "<<min<<"ns"<<endl;
    cout << "Maximum access time: "<<max<<"ns"<<endl;

    
  return 1;
}