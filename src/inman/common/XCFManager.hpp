/* ************************************************************************** *
 * XML Configuration Files Manager: lightweight implementation.
 * ************************************************************************** */
#ifndef __UTIL_XCFG_MANAGER_HPP__
#ident "@(#)$Id$"
#define __UTIL_XCFG_MANAGER_HPP__

#include <sys/stat.h>

#include <list>

#include "util/config/Config.h"

#include "util/xml/init.h"
#include "util/xml/DOMTreeReader.h"

namespace smsc {
namespace util {
namespace config {

using smsc::util::xml::DOMTreeReader;
using smsc::util::xml::ParseException;

class XCFManager {
private:
    typedef std::list<std::string> XCFPath;
    XCFPath path;

protected:
    XCFManager()
    { 
        smsc::util::xml::initXerces();
        path.push_back(std::string("./"));
        path.push_back(std::string("./conf/"));
        path.push_back(std::string("../conf/"));
    }
    ~XCFManager()
    { 
        smsc::util::xml::TerminateXerces();
    }

public:
    static XCFManager & getInstance()
    {
        static XCFManager instance;
        return instance;
    }

    void addPath(const char * use_path)
    {
        std::string fn(use_path);
        std::string::size_type bpos = fn.find_last_of('/');
        if (bpos == fn.npos || (bpos != fn.size()))
            fn += '/';
        path.push_back(fn);
    }

    //returns actual file path, relating to working directory
    bool findFile(const char * nm_file, std::string * rel_nm = NULL)
    {
        std::string fn;
        for (XCFPath::const_iterator pit = path.begin();
                                    pit != path.end(); ++pit) {
            struct stat s;
            fn = *pit; fn += nm_file;
            
            if (!stat(fn.c_str(), &s) && !S_ISDIR(s.st_mode)) {
                if (rel_nm)
                    *rel_nm = fn;
                return true;
            }
        }
        return false;
    }

    //Searches for XML configuration file, perfoms its parsing.
    //Returns allocated Config() on success.
    Config * getConfig(const char * xcfg_file, std::string * real_nm = NULL)
        throw(ConfigException)
    {
        std::string relFn;
        std::string *pFn = real_nm ? real_nm : &relFn;
        if (!findFile(xcfg_file, pFn))
            throw ConfigException("%s file not found", pFn->c_str());

        DOMTreeReader reader;
        DOMElement *elem = NULL;
        try {
          DOMDocument * document = reader.read(pFn->c_str());
          if (document)
              elem = document->getDocumentElement();
        } catch (const ParseException & exc) {
            throw ConfigException("%s parsing failed: %s", pFn->c_str(), exc.what());
        }
        if (!elem)
            throw ConfigException("%s parsing failed: no element", pFn->c_str());
        return new Config(*elem); //throws
    }

};

} //config
} //util
} //smsc
#endif /* __UTIL_XCFG_MANAGER_HPP__ */

