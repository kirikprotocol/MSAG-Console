#ifndef __INST_H__
#define __INST_H__

#include <string>

namespace scag {
namespace util {

/// A checker for existence of another instance of running process.
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

} // namespace util
} // namespace scag

namespace scag2 {
namespace util {
using scag::util::Inst;
} // namespace util
} // namespace scag2

#endif
