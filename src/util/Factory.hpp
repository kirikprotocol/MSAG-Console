/* ************************************************************************** *
 * Generic factories templates.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_FACTORYT_HPP__
#ident "@(#)$Id$"
#define __SMSC_UTIL_FACTORYT_HPP__

#include <map>

namespace smsc {
namespace util {

template <typename KeyTypeNm, class ProductType>
class FactoryT {
public:
    typedef KeyTypeNm KeyType;

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
        std::pair<typename ProductsMap::iterator, bool> res =
            producers.insert(typename ProductsMap::value_type(key, producer));
        return res.second;
    }
    //
    ProductType* create(KeyType key) const
    {
        typename ProductsMap::const_iterator it = producers.find(key);
        return (it == producers.end()) ? NULL : it->second->create();
    }
    //usefull for determining wether the product is registered or not
    Producer * getProducer(KeyType key) const
    {
        typename ProductsMap::const_iterator it = producers.find(key);
        return (it == producers.end()) ? NULL : it->second;
    }

    //destroys all producers
    void eraseAll(void)
    {
        for (typename ProductsMap::iterator it = producers.begin(); it != producers.end(); ++it)
            delete it->second;
        producers.clear();
    }
};


//Generic factory that allows to set an optional argument
//for product creation.
//NOTE: _TArg(_ArgTArg * use_arg = NULL) must be defined
template <typename _KeyTArg, class _ProductAC_TArg, class _ArgTArg>
class FactoryXArg_T {
public:
    typedef _KeyTArg KeyType;

    struct ProducerITF
    {
        virtual _ProductAC_TArg * create(_ArgTArg use_arg) const = 0;
    };

    template< class _TArg /* : public _ProductAC_TArg*/>
    struct ProducerT : public ProducerITF
    {
        virtual _ProductAC_TArg * create(_ArgTArg use_arg) const
        { return new _TArg(use_arg); }
    };

protected:
    typedef std::map<KeyType, ProducerITF*> ProductsMap;
    ProductsMap producers;

public:
    FactoryXArg_T()
    { }
    ~FactoryXArg_T()
    { }

    bool registerProduct(KeyType key, ProducerITF * producer)
    {
        std::pair<typename ProductsMap::iterator, bool> res =
            producers.insert(typename ProductsMap::value_type(key, producer));
        return res.second;
    }

    _ProductAC_TArg * create(KeyType key, _ArgTArg use_arg) const
    {
        typename ProductsMap::const_iterator it = producers.find(key);
        return (it == producers.end()) ? NULL : it->second->create(use_arg);
    }

    //usefull for determining wether the product is registered or not
    ProducerITF * getProducer(KeyType key) const
    {
        typename ProductsMap::const_iterator it = producers.find(key);
        return (it == producers.end()) ? NULL : it->second;
    }

    //destroys all producers
    void eraseAll(void)
    {
        for (typename ProductsMap::iterator it = producers.begin(); it != producers.end(); ++it)
            delete it->second;
        producers.clear();
    }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_FACTORYT_HPP__ */

