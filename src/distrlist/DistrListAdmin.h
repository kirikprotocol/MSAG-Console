
#ifndef SMSC_DISTRIBUTION_LIST_ADMIN
#define SMSC_DISTRIBUTION_LIST_ADMIN

#include <string>

#include "sms/sms.h"
//#include "db/DataSource.h"
#include "core/buffers/Array.hpp"

#include "DistrListExceptions.h"
#include "core/buffers/File.hpp"

namespace smsc { namespace distrlist
{
    using std::string;
    using smsc::core::buffers::Array;

    //using namespace smsc::db;
    using namespace smsc::sms;

    struct Principal
    {
        Address address;
        int     maxLst, maxEl;

        Principal()
            : maxLst(0), maxEl(0) {};
        Principal(Address address, int maxLst, int maxEl)
            : address(address), maxLst(maxLst), maxEl(maxEl) {};

    };

    struct DistrList
    {
        char    name[31];
        bool    system;
        int     maxEl;
        Address owner;

        DistrList(const char* argName="", int maxEl=0)
            : system(true),maxEl(maxEl)
            {
              setName(argName);
            };
        DistrList(const Address& owner, const char* argName="", int maxEl=0)
            : system(false), maxEl(maxEl), owner(owner)
        {
          setName(argName);
        };
        DistrList(const DistrList& dl)
            : system(dl.system),maxEl(dl.maxEl), owner(dl.owner)
        {
          strcpy(name,dl.name);
        };

        void setName(const char* argName)
        {
          if(argName)
          {
            memcpy(name,argName,std::min(strlen(argName)+1,sizeof(name)));
            name[sizeof(name)-1]=0;
          }else
          {
            memset(name,0,sizeof(name));
          }
        }

    };

    class DistrListAdmin
    {
    protected:

        DistrListAdmin() {};

    public:

        virtual ~DistrListAdmin() {};

        virtual void addDistrList(string dlName,
             bool system,const Address& dlOwner,int maxEl,
               smsc::core::buffers::File::offset_type offset1=0,
               smsc::core::buffers::File::offset_type offset2=0)
            throw(smsc::core::buffers::FileException, ListAlreadyExistsException,
                  PrincipalNotExistsException, ListCountExceededException) = 0;
        virtual void deleteDistrList(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException) = 0;
        virtual DistrList getDistrList(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException) = 0;
        virtual Array<DistrList> list(const Address& dlOwner)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException) = 0;
        virtual Array<DistrList> list()
            throw(smsc::core::buffers::FileException) = 0;
        virtual void copyDistrList(const std::string& dlName,const std::string& newDlName)=0;
        virtual void renameDistrList(const std::string& dlName,const std::string& newDlName)=0;


        virtual void changeDistrList(const string& dlName,int maxElements)
          throw(smsc::core::buffers::FileException,ListNotExistsException) = 0;

        virtual void getSubmitters(const string& dlName,Array<Address>& sbm)
          throw(smsc::core::buffers::FileException,ListNotExistsException)=0;

        virtual void addPrincipal(const Principal& prc,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, PrincipalAlreadyExistsException) = 0;
        virtual void deletePrincipal(const Address& address)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException, PrincipalInUseException) = 0;
        virtual void changePrincipal(const Principal& prc)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException, IllegalPrincipalException) = 0;
        virtual Principal getPrincipal(const Address& address)
            throw(smsc::core::buffers::FileException, PrincipalNotExistsException) = 0;

        virtual Array<Principal> getPrincipals()
            throw(smsc::core::buffers::FileException)=0;

        virtual void addMember(string dlName, const Address& member,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  MemberAlreadyExistsException, MemberCountExceededException) = 0;
        virtual void deleteMember(string dlName, const Address& member)
            throw(smsc::core::buffers::FileException, ListNotExistsException, MemberNotExistsException) = 0;
        virtual void deleteMembers(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException) = 0;
        virtual Array<Address> members(string dlName, const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException,IllegalSubmitterException) = 0;

        virtual Array<Address> members(string dlName)
            throw(smsc::core::buffers::FileException, ListNotExistsException) = 0;

        virtual void grantPosting(string dlName, const Address& submitter,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  PrincipalNotExistsException, SubmitterAlreadyExistsException) = 0;

        virtual void grantPosting(const string& dlName, const Address& owner,const Address& submitter,smsc::core::buffers::File::offset_type offset=0)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  PrincipalNotExistsException, SubmitterAlreadyExistsException) = 0;


        virtual void revokePosting(string dlName, const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  SubmitterNotExistsException, IllegalSubmitterException) = 0;

        virtual void revokePosting(string dlName, const Address& owner,const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException,
                  SubmitterNotExistsException, IllegalSubmitterException) = 0;

        virtual bool checkPermission(string dlName, const Address& submitter)
            throw(smsc::core::buffers::FileException, ListNotExistsException) = 0;


    };

}}

#endif //SMSC_DISTRIBUTION_LIST_ADMIN
