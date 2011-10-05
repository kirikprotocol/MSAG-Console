
#ifndef SMSC_DISTRIBUTION_LIST_MANAGER
#define SMSC_DISTRIBUTION_LIST_MANAGER

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>

//#include <db/DataSource.h>

#include "DistrListAdmin.h"

#include "core/buffers/File.hpp"
#include "core/buffers/FixedRecordFile.hpp"
#include "sms/sms_util.h"
#include <set>
#include <vector>


namespace smsc { namespace distrlist
{
    //using namespace smsc::db;

    using smsc::logger::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    using namespace core::synchronization;

    struct PrincipalRecord:Principal{
        int lstCount;
        smsc::core::buffers::File::offset_type offset;

        PrincipalRecord():Principal(),lstCount(0)
        {
        }

        PrincipalRecord(Address address, int maxLst, int maxEl):Principal(address, maxLst,maxEl),lstCount(0)
          {}

        void Read(smsc::core::buffers::File& f)
        {
          smsc::sms::ReadAddress(f,address);
          maxLst=f.ReadNetInt32();
          maxEl=f.ReadNetInt32();
        }
        void Write(smsc::core::buffers::File& f)const
        {
          smsc::sms::WriteAddress(f,address);
          f.WriteNetInt32(maxLst);
          f.WriteNetInt32(maxEl);
        }
        static uint32_t Size()
        {
          return smsc::sms::AddressSize()+4+4;
        }
    };

    struct MemberRecord{
      char list[32];
      Address addr;
      smsc::core::buffers::File::offset_type offset;

      MemberRecord()
      {
        offset=0;
      }
      MemberRecord(const DistrList& argDl,const Address& argAddr):addr(argAddr)
      {
        strcpy(list,argDl.name);
        offset=0;
      }

      /*
      This is ugly hack.
      This structures are stored in an std::set.
      Iterator of std::set disallow modification of item.
      */
      void setList(const char* newLstName)const
      {
        char* ptr=const_cast<char*>(list);
        strncpy(ptr,newLstName,31);
        ptr[31]=0;
      }

      bool operator<(const MemberRecord& rhs)const
      {
        return addr<rhs.addr;
      }

      void Read(smsc::core::buffers::File& f)
      {
        f.ReadFixedString(list);
        ReadAddress(f,addr);
      }
      void Write(smsc::core::buffers::File& f)const
      {
        f.WriteFixedString(list);
        WriteAddress(f,addr);
      }
      static uint32_t Size()
      {
        MemberRecord* ptr=0;
        return (uint32_t)sizeof(ptr->list)+AddressSize();
      }
    };
    typedef MemberRecord SubmitterRecord;

    struct DistrListRecord:DistrList{
      smsc::core::buffers::File::offset_type offset;
      typedef std::set<MemberRecord> MembersContainer;
      MembersContainer members;
      typedef std::set<SubmitterRecord> SubmittersContainer;
      SubmittersContainer submitters;

      DistrListRecord()
      {
        offset=0;
      }

      DistrListRecord(const char* dlName,int maxel):
        DistrList(dlName,maxel)
        {
          offset=0;
        }

      DistrListRecord(const Address& addr,const char* dlName,int maxel):
        DistrList(addr,dlName,maxel)
        {
          offset=0;
        }

      void Read(smsc::core::buffers::File& f)
      {
        f.ReadFixedString(name);
        maxEl=f.ReadNetInt32();
        system=f.ReadByte()!=0;
        smsc::sms::ReadAddress(f,owner);
      }
      void Write(smsc::core::buffers::File& f)const
      {
        f.WriteFixedString(name);
        f.WriteNetInt32(maxEl);
        f.WriteByte(system?1:0);
        smsc::sms::WriteAddress(f,owner);
      }

      static uint32_t Size()
      {
        DistrList* ptr=0;
        return (uint32_t)sizeof(ptr->name)+4+1+smsc::sms::AddressSize();
      }
    };

    class DistrListManager : public DistrListAdmin
    {
    protected:

        smsc::logger::Logger *logger;

        smsc::core::buffers::Hash<DistrListRecord*> lists;
        smsc::core::buffers::Hash<PrincipalRecord> principals;

        smsc::core::buffers::FixedRecordFile<DistrListRecord> lstFile;
        smsc::core::buffers::FixedRecordFile<PrincipalRecord> prcFile;
        smsc::core::buffers::FixedRecordFile<MemberRecord>    memFile;
        smsc::core::buffers::FixedRecordFile<SubmitterRecord> sbmFile;

        std::string dir;

        smsc::core::synchronization::Mutex mtx;

    public:

        DistrListManager(Manager& config)
            throw(ConfigException,smsc::core::buffers::FileException);
        virtual ~DistrListManager();

        void init();

        virtual void addDistrList(string dlName, bool system,
            const Address& dlOwner,int maxel,
              smsc::core::buffers::File::offset_type offset1=0,
              smsc::core::buffers::File::offset_type offset2=0)
            throw(smsc::core::buffers::FileException, ListAlreadyExistsException,
                  PrincipalNotExistsException, ListCountExceededException);
        virtual void deleteDistrList(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException);
        virtual DistrList getDistrList(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException);
        virtual Array<DistrList> list(const Address& dlOwner)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException);
        virtual Array<DistrList> list()
            throw(smsc::core::buffers::FileException);
        virtual void copyDistrList(const std::string& dlName,const std::string& newDlName);
        virtual void renameDistrList(const std::string& dlName,const std::string& newDlName);



        virtual void changeDistrList(const string& dlName,int maxElements)
          throw(smsc::core::buffers::FileException,ListNotExistsException);

        virtual void getSubmitters(const string& dlName,Array<Address>& sbm)
          throw(smsc::core::buffers::FileException,ListNotExistsException);


        virtual void addPrincipal(const Principal& prc,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, PrincipalAlreadyExistsException);
        virtual void deletePrincipal(const Address& address)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException, PrincipalInUseException);
        virtual void changePrincipal(const Principal& prc)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException, IllegalPrincipalException);
        virtual Principal getPrincipal(const Address& address)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException);
        virtual Array<Principal> getPrincipals()
            throw(smsc::core::buffers::FileException);


        virtual void addMember(string dlName, const Address& member,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  MemberAlreadyExistsException, MemberCountExceededException);
        virtual void deleteMember(string dlName, const Address& member)
            throw(smsc::core::buffers::FileException, ListNotExistsException, MemberNotExistsException);
        virtual void deleteMembers(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException);
        virtual Array<Address> members(string dlName, const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException,IllegalSubmitterException);
        virtual Array<Address> members(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException);

        virtual void grantPosting(string dlName, const Address& submitter,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  PrincipalNotExistsException, SubmitterAlreadyExistsException);
        virtual void grantPosting(const string& dlName, const Address& owner,const Address& submitter,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  PrincipalNotExistsException, SubmitterAlreadyExistsException);
        virtual void revokePosting(string dlName, const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  SubmitterNotExistsException, IllegalSubmitterException);
        virtual void revokePosting(string dlName, const Address& owner,const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  SubmitterNotExistsException, IllegalSubmitterException);
        virtual bool checkPermission(string dlName, const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException);

    };

}}

#endif //SMSC_DISTRIBUTION_LIST_MANAGER
