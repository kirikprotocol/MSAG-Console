#ifndef __INST_H__
#define __INST_H__

#include <string>

class Inst 
{
public:
	Inst();
    Inst(const char* fname);
	~Inst();
	bool run(){return run_;}
	
protected:
	std::string filename;
	bool run_;
	bool IsProcDir(char* procdir);
};

#endif
