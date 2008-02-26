#pragma ident "$Id$"
/* ************************************************************************** *
 * Generic factories templates.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_FACTORYT_HPP__
#define __SMSC_UTIL_FACTORYT_HPP__

#include <map>

namespace smsc {
namespace util {

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
    bool registerProduct(KeyType key, Producer* producer)
    {
        std::pair<ProductsMap::iterator, bool> res =
            producers.insert(ProductsMap::value_type(key, producer));
        return res.second;
    }
    //
    ProductType* create(KeyType key) const
    {
        ProductsMap::const_iterator it = producers.find(key);
        return (it == producers.end()) ? NULL : it->second->create();
    }
    //usefull for determining wether the product is registered or not
    Producer * getProducer(KeyType key) const
    {
        ProductsMap::const_iterator it = producers.find(key);
        return (it == producers.end()) ? NULL : it->second;
    }

    //destroys all producers
    void eraseAll(void)
    {
        for (ProductsMap::iterator it = producers.begin(); it != producers.end(); ++it)
            delete it->second;
        producers.clear();
    }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_FACTORYT_HPP__ */

