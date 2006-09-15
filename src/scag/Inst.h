#ifndef __INST_H__
#define __INST_H__

#include <string>

class Inst 
{
public:
    Inst() : run_(true) { check("/tmp/scag.pid"); }
    Inst(const char* fname) : run_(true) { check(fname); }
    
    ~Inst() { if(run_) remove(filename.c_str()); }
    bool run(){return run_;}
    
protected:
    std::string filename;
    bool run_;
    bool IsProcDir(char* procdir);
    void check(const char* fname);
};

#endif
