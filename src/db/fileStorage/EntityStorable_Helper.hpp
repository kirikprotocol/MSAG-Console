#ifndef __DBENTITYSTORAGE_ENTITYSTORABLE_HELPER_HPP__
# define __DBENTITYSTORAGE_ENTITYSTORABLE_HELPER_HPP__

# include <inttypes.h>

class EntityStorable_Helper {
public:
  EntityStorable_Helper(uint8_t* serializeBuf, uint16_t serializeBufSz);
  EntityStorable_Helper(const EntityStorable_Helper& rhs);
  EntityStorable_Helper& operator=(const EntityStorable_Helper&);

  virtual ~EntityStorable_Helper();

  uint32_t calcCrc(uint32_t crc=0) const;

  uint16_t getSize() const;
protected:
  uint8_t* _serialize_buf;
  uint16_t _serialize_buf_size;
private:
  void make_copy(const EntityStorable_Helper& rhs);
};

#endif
