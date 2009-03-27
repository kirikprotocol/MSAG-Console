#ifndef _SCAG_UTIL_HISTO_HISTO_H
#define _SCAG_UTIL_HISTO_HISTO_H

#include <cassert>
#include <algorithm>

namespace scag2 {
namespace util {
namespace histo {

namespace details {
/// NOTE: binround adds an underflow bin also
template < typename T > struct binround
{
    // NOTE: integral type T is assumed
    inline static int round( T val ) { return int(val+1); }
    inline static T floatRound( double val ) { return val+0.5; }
};
template <> struct binround< float >
{
    inline static int round( float val ) { return int(val+1.5); }
    inline static float floatRound( double val ) { return val; }
};
template <> struct binround< double >
{
    inline static int round( double val ) { return int(val+1.5); }
    inline static double floatRound( double val ) { return val; }
};
}

// template < typename T > Histo;

// simple histogram of type T
class HistoData
{
    // friend template < typename T > class Histo;
public:
    HistoData( unsigned nbins ) :
    size_(nbins), bins_(new unsigned[size_]) {
        reset();
    }

    /*
    HistoData< T >* stripData() {
        HistoData< T >* ret = new HistoData< T >(*this);
        std::swap(ret->bins_,bins_);
        return ret;
    }
     */

    inline ~HistoData() {
        delete[] bins_;
    }

    /// the total number of bins (including underflow and overflow)
    inline unsigned size() const { return size_; }

    // the minimum value
    // inline T getMin() const { return tmin_; }
    // inline T getBinWidth() const { return dt_; }

    /// NOTE: no checks for speed
    inline unsigned getBinValue( unsigned n ) const { return bins_[n]; }
    inline unsigned getTotal() const { return count_; }
    inline void reset() { memset(bins_,0,sizeof(unsigned)*size_); count_ = 0; }
    
    /// fill bin.
    /// NOTE: checks are not done for performance
    inline void fillBin( unsigned n, unsigned inc = 1 ) {
        bins_[n] += inc;
        count_ += inc;
    }

    /// get average.
    /// NOTE: underflow and overflow are not accounted.
    /// NOTE: returned value is bin number [0..size-2].
    /// NOTE: rv [0..1) corresponds the 1st bin.
    double getAverage() const {
        unsigned count = 0;
        unsigned long long avg = 0;
        for ( unsigned i = 1; i < size_-1; ++i ) {
            count += bins_[i];
            avg += i*bins_[i];
        }
        if ( count == 0 ) return 0;
        return double(avg)/count;
    }

private:
    HistoData() : size_(0), bins_(0) {}
    HistoData( const HistoData& h ) : size_(h.size_), bins_(new unsigned[size_]) {
        reset();
    }
    HistoData& operator = ( const HistoData& h );

private:
    const unsigned size_; // the size of histogram (including the underflow and the overflow bins)
    unsigned*      bins_;
    unsigned       count_;
};


template < typename T > class Histo1d
{
public:
    Histo1d( unsigned nbins, T tmin, T tmax ) : nbins_(nbins), data_(nbins+2), tmin_(tmin), dt_((tmax-tmin)/nbins), first_(true), bounds_(0) {
        if ( dt_ <= 0 ) dt_ = 1;
    }
    Histo1d( double scale, unsigned nbins, T tmin = 1 ) : nbins_(nbins), data_(nbins+2), tmin_(tmin), dt_(0), first_(true) {
        if ( scale <= 1. ) scale = 2.;
        // find out the boundaries between bins
        bounds_ = new T[nbins_+1];
        double pos = tmin_;
        for ( unsigned i = 0; i <= nbins_; ++i ) {
            bounds_[i] = details::binround<T>::floatRound(pos);
            pos *= scale;
        }
    }
    
    ~Histo1d() {
        if (bounds_) delete [] bounds_;
    }

    void fill( T val, unsigned inc = 1 ) {
        register unsigned n = bin(val);
        data_.fillBin( n, inc );
        if ( first_ ) {
            first_ = false;
            minValue_ = val;
            maxValue_ = val;
        } else if ( val < minValue_ ) {
            minValue_ = val;
        } else if ( val > maxValue_ ) {
            maxValue_ = val;
        }
    }
    
    /// NOTE: incl. underflow and overflow
    inline unsigned numberOfBins() const { return data_.size(); }
    inline unsigned getBinValue( unsigned n ) const {
        assert(n < data_.size());
        return data_.getBinValue(n);
    }
    inline unsigned getTotal() const { return data_.getTotal(); }
    inline void reset() { data_.reset(); first_ = true; }

    /// get the boundary beetween the underflow and 1st bins
    inline T getMin() const { return tmin_; }
    inline T getBinWidth() const { return dt_; }

    // NOTE: only for bins 1..N-1
    inline T getBinLowerBound( unsigned n ) const { 
        assert( n >= 1 && n < data_.size() );
        if ( bounds_ ) {
            // logarithmic
            return bounds_[n-1];
        } else {
            // linear
            return (n-1)*dt_ + tmin_;
        }
    }

    /// get the minimum value seen so far
    inline T getMinValue() const { return minValue_; }
    /// get the minimum value seen so far
    inline T getMaxValue() const { return maxValue_; }
    /// get the average.
    /// NOTE: underflow and overflow are not accounted
    T getAverage() const {
        if ( bounds_ ) {
            // logarithmic
            unsigned count = 0;
            double avg = 0;
            for ( unsigned i = 1; i <= nbins_; ++i ) {
                unsigned b = getBinValue(i);
                if ( b ) {
                    count += b;
                    avg += b * (getBinLowerBound(i) + getBinLowerBound(i+1))/2;
                }
            }
            if ( count == 0 ) return 0;
            return details::binround<T>::floatRound(avg/count);
        } else {
            // linear
            return T(data_.getAverage()*dt_+tmin_);
        }
    }
    
private:
    unsigned bin( T val )
    {
        if ( bounds_ ) {
            // logarithmic
            if ( val < bounds_[0] ) {
                // __trace2__("val: %llu, n:0", uint64_t(val));
                return 0;
            }
            if ( val >= bounds_[nbins_] ) {
                // __trace2__("val: %llu, n:%u", uint64_t(val), nbins_+1);
                return nbins_+1;
            }
            T* pos = std::upper_bound( &bounds_[0], &bounds_[nbins_+1], val );
            const unsigned n = unsigned(pos - bounds_);
            // __trace2__("bounds are: [%llu,%llu], val: %llu, n: %u", uint64_t(bounds_[0]), uint64_t(bounds_[nbins_]), uint64_t(val), n);
            assert( n > 0 && n <= nbins_ );
            return n;
        } else {
            // linear
            int n = details::binround< T >::round((val-tmin_)/dt_);
            if ( n < 0 ) n = 0;
            else if ( n >= data_.size() ) n = data_.size()-1;
            return n;
        }
    }

private:
    unsigned       nbins_;
    HistoData      data_;
    T*             bounds_;  // boundaries b/w bins
    T              tmin_;
    T              dt_;
    T              minValue_;
    T              maxValue_;
    bool           first_;
};

}
}
}

#endif /* !_SCAG_UTIL_HISTO_HISTO_H */
