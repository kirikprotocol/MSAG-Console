#ident "$Id$"
#ifndef __SMSC_INMAN_COMMON_FACTORYT_HPP__
#define __SMSC_INMAN_COMMON_FACTORYT_HPP__

#include <map>

namespace smsc {
namespace inman {
namespace common {

template <typename KeyTypeNm, class ProductType>
class FactoryT {
public:
    typedef typename KeyTypeNm KeyType;

    struct Producer
    {
        virtual ProductType* create() const = 0;
    };

    template< class T >
    struct ProducerT : public Producer
    {
        virtual ProductType* create() const { return new T; }
    };

    typedef std::map<KeyType, Producer*> ProductsMap;

protected:
    ProductsMap producers;

public:
    //usefull for determining wether the product is registered or not
    Producer * getProducer(KeyType key) const
    {
        ProductsMap::const_iterator it = producers.find(key);
        if (it == producers.end())
            return NULL;
        return (*it).second;
    }

    ProductType* create(KeyType key) const
    {
        ProductsMap::const_iterator it = producers.find(key);
        if (it == producers.end())
            return NULL;
        return (*it).second->create();
    }

    void registerProduct(KeyType key, Producer* producer)
    {
        producers.insert( ProductsMap::value_type( key, producer ) );
    }
};


} //common
} //inman
} //smsc

#endif

