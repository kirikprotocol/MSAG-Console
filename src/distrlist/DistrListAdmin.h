
#ifndef SMSC_DISTRIBUTION_LIST_ADMIN
#define SMSC_DISTRIBUTION_LIST_ADMIN

#include <string>

#include <sms/sms.h>
#include <db/DataSource.h>
#include <core/buffers/Array.hpp>

#include "DistrListExceptions.h"

namespace smsc { namespace distrlist
{
    using std::string;
    using smsc::core::buffers::Array;

    using namespace smsc::db;
    using namespace smsc::sms;
    
    struct Principal
    {
        Address address;
        int     maxLst, maxEl;

        Principal() 
            : maxLst(0), maxEl(0) {};
        Principal(Address address, int maxLst, int maxEl)
            : address(address), maxLst(maxLst), maxEl(maxEl) {};
        Principal(const Principal& prc)
            : address(prc.address), maxLst(prc.maxLst), maxEl(prc.maxEl) {};
        virtual ~Principal() {};
        
        Principal& operator=(const Principal& prc) {
            address = prc.address;
            maxLst = prc.maxLst; maxEl = prc.maxEl;
            return (*this);
        }
    };
    
    struct DistrList
    {
        string  name;
        int     maxEl;
        bool    system;
        Address owner;

        DistrList(string name="", int maxEl=0)
            : name(name), maxEl(maxEl), system(true) {};
        DistrList(const Address& owner, string name="", int maxEl=0)
            : name(name), maxEl(maxEl), system(false), owner(owner) {};
        DistrList(const DistrList& dl)
            : name(dl.name), maxEl(dl.maxEl), system(dl.system), owner(dl.owner) {};
        virtual ~DistrList() {};

        DistrList& operator=(const DistrList& dl) {
            name = dl.name; maxEl = dl.maxEl;
            system = dl.system; owner = dl.owner;
            return (*this);
        }

    };
    
    class DistrListAdmin
    {
    protected:
        
        DistrListAdmin() {};

    public:

        virtual ~DistrListAdmin() {};
        
        virtual void addDistrList(string dlName, const Address& dlOwner) 
            throw(SQLException, ListAlreadyExistsException, 
                  PrincipalNotExistsException, ListCountExceededException) = 0;
        virtual void deleteDistrList(string dlName)
            throw(SQLException, ListNotExistsException) = 0;
        virtual DistrList getDistrList(string dlName)
            throw(SQLException, ListNotExistsException) = 0;
        virtual Array<DistrList> list()
            throw(SQLException) = 0;

        virtual void addPrincipal(const Principal& prc) 
            throw(SQLException, PrincipalAlreadyExistsException) = 0;
        virtual void deletePrincipal(const Address& address) 
            throw(SQLException, PrincipalNotExistsException, PrincipalInUseException) = 0;
        virtual void changePrincipal(const Principal& prc) 
            throw(SQLException, PrincipalNotExistsException, IllegalPrincipalException) = 0;
        virtual Principal getPrincipal(const Address& address) 
            throw(SQLException, PrincipalNotExistsException) = 0;
        
        virtual void addMember(string dlName, const Address& member) 
            throw(SQLException, ListNotExistsException, 
                  MemberAlreadyExistsException, MemberCountExceededException) = 0;
        virtual void deleteMember(string dlName, const Address& member) 
            throw(SQLException, ListNotExistsException, MemberNotExistsException) = 0;
        virtual void deleteMembers(string dlName) 
            throw(SQLException, ListNotExistsException) = 0;
        virtual Array<Address> members(string dlName, const Address& submitter)
            throw(SQLException, IllegalSubmitterException) = 0;
        
        virtual void grantPosting(string dlName, const Address& submitter) 
            throw(SQLException, ListNotExistsException, 
                  PrincipalNotExistsException, SubmitterAlreadyExistsException) = 0;
        virtual void revokePosting(string dlName, const Address& submitter)
            throw(SQLException, ListNotExistsException,
                  SubmitterNotExistsException, IllegalSubmitterException) = 0;
        virtual bool checkPermission(string dlName, const Address& submitter)
            throw(SQLException, ListNotExistsException) = 0;
    };
    
}}

#endif //SMSC_DISTRIBUTION_LIST_ADMIN

