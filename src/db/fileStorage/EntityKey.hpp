#ifndef __DBENTITYSTORAGE_ENTITY_KEY_HPP__
# define __DBENTITYSTORAGE_ENTITY_KEY_HPP__ 1

# include <string>

/*
** Interface for key abstraction. Keys can compare.
**
*/
template <typename EntityKeyTraits_t>
class EntityKey {
public:
  virtual bool operator==(const typename EntityKeyTraits_t::key_type&) const = 0;
  virtual bool operator<(const typename EntityKeyTraits_t::key_type&) const = 0;
  virtual std::string toString() const = 0;
};

/*
** It's the base class for Key Traits abstraction.
*/
template <class keyT>
struct key_traits {
  typedef keyT key_type;
};

#endif
