#ifndef _SCAG_UTIL_HISTO_HISTOLOGGER_H
#define _SCAG_UTIL_HISTO_HISTOLOGGER_H

#include <sstream>
#include "Histo.h"
#include "scag/util/Print.h"

namespace scag2 {
namespace util {
namespace histo {

class HistoLogger
{
public:
    HistoLogger( util::Print& print, unsigned maxValue = 60 ) : print_(print), maxValue_(maxValue), floatPrecision_(2) {}
    void setFloatPrecision( unsigned f ) { floatPrecision_ = f; }

    template < typename T > void printHisto( const util::histo::Histo1d< T >& h, const char* fmt, ... ) {
        if ( !print_.enabled() ) return;

        // prepare histo dump
        std::string dump;
        dump.reserve( (maxValue_+10)*h.numberOfBins() + 60);

        // writing title
        char buf[80];
        va_list arglist;
        va_start(arglist,fmt);
        vsnprintf(buf,sizeof(buf),fmt,arglist);
        va_end(arglist);

        // get the scale for histogram
        unsigned scale = 1;
        for ( unsigned i = 0; i < h.numberOfBins(); ++i ) {
            register unsigned v = h.getBinValue(i);
            if (v/scale > maxValue_) {
                scale = (v-1)/maxValue_ + 1;
            }
        }
        std::ostringstream os;
        os.setf(std::ios::fixed,std::ios::floatfield);
        os.precision(floatPrecision_);
        os << buf << " tot=" << h.getTotal() << " min/avg/max=" <<
            h.getMinValue() << "/" << h.getAverage() << "/" <<
            h.getMaxValue() << " scale=" << scale;
        dump.append(os.str());
        for ( unsigned i = 0; i < h.numberOfBins(); ++i ) {
            dump.push_back('\n');
            if ( i == 0 ) {
                os.rdbuf()->str("");
                os << h.getBinLowerBound(1);
                fill( dump, 8, os.str().c_str(), '<');
            } else if ( i+1 < h.numberOfBins() ) {
                os.rdbuf()->str(""); // reset the stream
                os << h.getBinLowerBound(i);
                fill( dump, 8, os.str().c_str() );
            } else {
                os.rdbuf()->str("");
                os << h.getBinLowerBount(i);
                fill( dump, 8, os.str().c_str(), '>');
            }
            dump.push_back(' ');
            fill(dump,h.getBinValue(i)/scale,"",'*');
        }
        print_.print("%s",dump.c_str());
    }

private:
    void fill(std::string& dump, unsigned len, const char* val, char pad = ' ')
    {
        unsigned valLen = unsigned(strlen(val));
        if ( valLen > len ) {
            dump.append(val,len);
        } else {
            dump.append(std::string(len-valLen,pad));
            dump.append(val);
        }
    }

private:
    util::Print& print_;
    unsigned     maxValue_;
    unsigned     floatPrecision_;
};

}
}
}

#endif
