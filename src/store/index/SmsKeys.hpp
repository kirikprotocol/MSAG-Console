struct Int64Key{
  uint64_t key;

  Int64Key():key(0){}
  Int64Key(uint64_t key):key(key){}
  Int64Key(const Int64Key& src)
  {
    key=src.key;
  }
  Int64Key& operator=(const Int64Key& src)
  {
    key=src.key;
    return *this;
  }
  uint64_t Get()const{return key;}

  static uint32_t Size(){return 8;}
  void Read(File& f)
  {
    key=f.ReadNetInt64();
  }
  void Write(File& f)const
  {
    f.WriteNetInt64(key);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    res=crc32(res,&key,sizeof(key));
    for(;attempt>0;attempt--)res=crc32(res,&key,sizeof(key));
    return res;
  }
  bool operator==(const Int64Key& cmp)
  {
    return key==cmp.key;
  }
};

template <int N>
class StrKey{
protected:
  char str[N+1];
  uint8_t len;
public:
  StrKey()
  {
    memset(str,0,N+1);
    len=0;
  }
  StrKey(const char* s)
  {
    int l=strlen(s);
    strncpy(str,s,N);
    str[N]=0;
    len=l>N?N:l;
  }
  StrKey(const StrKey& src)
  {
    strcpy(str,src.str);
    len=src.len;
  }
  StrKey& operator=(const StrKey& src)
  {
    strcpy(str,src.str);
    len=src.len;
    return *this;
  }

  bool operator==(const StrKey& cmp)
  {
    return cmp.len==len && !strcmp(cmp.str,str);
  }

  const char* toString(){return str;}
  static uint32_t Size(){return N+1;}
  void Read(File& f)
  {
    f.XRead(len);
    f.Read(str,N);
    str[len]=0;
  }
  void Write(File& f)const
  {
    f.XWrite(len);
    f.Write(str,N);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    res=crc32(res,str,len);
    for(;attempt>0;attempt--)res=crc32(res,str,len);
    return res;
  }
};

struct IdLttKey{
  uint64_t key;
  uint32_t ltt;

  IdLttKey():key(0),ltt(0){}
  IdLttKey(uint64_t key,uint32_t ltt):key(key),ltt(ltt){}
  IdLttKey(const IdLttKey& src)
  {
    key=src.key;
    ltt=src.ltt;
  }
  IdLttKey& operator=(const IdLttKey& src)
  {
    key=src.key;
    ltt=src.ltt;
    return *this;
  }

  static uint32_t Size(){return 12;}
  void Read(File& f)
  {
    key=f.ReadNetInt64();
    ltt=f.ReadNetInt32();
  }
  void Write(File& f)const
  {
    f.WriteNetInt64(key);
    f.WriteNetInt32(ltt);
  }
  static void WriteBadValue(File& f)
  {
    uint64_t a=~0;
    uint32_t b=~0;
    f.WriteInt64(a);
    f.WriteInt32(b);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    res=crc32(res,&key,sizeof(key));
    res=crc32(res,&ltt,sizeof(ltt));
    for(;attempt>0;attempt--)
    {
      res=crc32(res,&key,sizeof(key));
      res=crc32(res,&ltt,sizeof(ltt));
    }
    return res;
  }
  bool operator==(const IdLttKey& cmp)
  {
    return key==cmp.key && ltt==cmp.ltt;
  }
  bool operator<(const IdLttKey& cmp)const
  {
    return key<cmp.key?true:ltt<cmp.ltt;
  }
};
