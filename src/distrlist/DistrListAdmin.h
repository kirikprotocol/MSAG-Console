
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
        string  address;
        int     maxLst, maxEl;

        Principal(string address="", int maxLst=0, int maxEl=0)
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
    
    /*struct Member
    {
    };*/


    /*
    struct DistrList
    {
    };*/

    
    class DistrListAdmin
    {
    protected:
        
        DistrListAdmin() {};

    public:

        virtual ~DistrListAdmin() {};

        virtual Array<Address> members(string dlName, const Address& submitter)
            throw(SQLException, IllegalSubmitterException) = 0;
        
        virtual void addPrincipal(const Principal& prc) 
            throw(SQLException, PrincipalAlreadyExistsException) = 0;
        
        virtual void addMember(string dlName, const Address& member) 
            throw(SQLException, ListNotExistsException, MemberAlreadyExistsException) = 0;
        virtual void deleteMember(string dlName, const Address& member) 
            throw(SQLException, ListNotExistsException, MemberNotExistsException) = 0;
        virtual void deleteMembers(string dlName) 
            throw(SQLException, ListNotExistsException) = 0;

        
        /*
        virtual void grantPosting(string dlName) = 0;
        virtual void revokePosting(string dlName) = 0;

        virtual void addDistrList(const DistrList& list) = 0;
        virtual void deleteDistrList(string dlName) = 0;
        virtual Array<DistrList> distributionList() = 0;

        virtual void addSysDistrList(const DistrList& list) = 0;
        virtual void deleteSysDistrList(string dlName) = 0;
        virtual Array<DistrList> sysDistrList() = 0;

        virtual bool checkPermission(string dlName, string address) = 0;*/
    };
    
}}

#endif //SMSC_DISTRIBUTION_LIST_ADMIN

