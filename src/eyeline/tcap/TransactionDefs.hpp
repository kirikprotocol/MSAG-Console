/* ************************************************************************** *
 * Transaction Capabilities: basic interfaces and classes.
 * ************************************************************************** */
#ifndef __TCAP_TRANSACTION_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_TRANSACTION_DEFS_HPP

#include "util/vformat.hpp"
#include "util/Factory.hpp"

#include "inman/comon/XRefPtr.hpp"
//#include "inman/inap/ACRegDefs.hpp"

#include "eyeline/tcap/SCCPMsgInd.hpp"
#include "eyeline/asn1/EncodedOID.hpp"

namespace eyelinecom {
namespace tcap {

using smsc::util::format;
using smsc::util::FactoryXArg_T;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::UTZObjectReferee_T;
using smsc::core::synchronization::MTRefPtr_T;
using eyelinecom::asn1::EncodedOID;


typedef EncodedOID AppContextID;
//Reserved AC for ROS operations, which are not binded to any AC (usually old MAP v1 operations)
extern AppContextID _ac_contextlessOperation;
//Reserved AC for unsupported contexts
extern AppContextID _ac_unsupportedAC;

class TCMessageInfo;    //Forward declaration for TCAP message wrapper
class TCIndicationInfo; //Forward declaration for TCProvider indication

//Transaction requests interface (implemented by TCProvider)
class TCProviderTReqIFace { //TODO:
public:
    virtual bool reqT_Begin(TCMessageInfo & tc_msg) = 0;
    virtual bool reqT_Cont(TCMessageInfo & tc_msg) = 0;
    virtual bool reqT_End(TCMessageInfo & tc_msg) = 0;
    virtual bool reqU_Abrt(/*...*/) = 0;
    virtual bool reqU_Cancel(/*...*/) = 0;
    virtual bool reqU_Reject(/*...*/) = 0;
    virtual bool reqT_Reset(uint8_t inv_id) = 0;
};

//Transaction initialization parameters
struct TransactionParms {
    TCProviderTReqIFace * trsIfc;   //Transaction requests interface to use
    TCProviderIFace *     pvdIfc;   //TCProvider to use
    TransactionId trId;             //
    const AppContextID * acId;      //encoded OID of corresponding AC
    uint8_t opcode;                 //first ROS operation code

    TransactionParms(TCProviderTReqIFace * use_trsIfc, TCProviderIFace * use_pvdIfc,
                     const TransactionId & use_id,
                     const AppContextID * ac_id = 0, uint8_t use_opcode = 0)
        : trsIfc(use_trsIfc), pvdIfc(use_pvdIfc), trId(use_id), opcode(use_opcode)
        , acId(ac_id ? ac_id : &_ac_contextlessOperation)
    { }
};

//Base class for transaction FSM (Finite State Machine) classes
class TransactionFSM_AC {
protected:
    TransactionParms    _prm;

public:
    TransactionFSM_AC(const TransactionParms & use_parms)
        : _prm(use_parms)
    { }
    virtual ~TransactionFSM_AC()
    { }

    inline const TransactionId & TrId(void) const { return _prm.trId; }
    inline const AppContextID & ACId(void) const { return *_prm.acId; }

    //-- Interface methods:
    // for synchronous access to transaction
    virtual Mutex & Sync(void) const = 0;
    // Remote point message indications:
    //   dialogue handling:  T_BEGIN, T_CONTINUE, T_END, U_ABORT, T_NOTICE (!!)
    //   component handling: INVOKE, RESULT_L, RESULT_NL, U_ERROR (more ???)
    virtual bool processTMsg(TCMessageInfo & use_msg) = 0; /* throw */
    // Local TC provider indications:
    //   dialogue handling:  P_ABORT
    //   component handling: L_CANCEL, L_REJECT, R_REJECT (more ???)
    virtual bool processTInd(TCIndicationInfo & use_msg) = 0; /* throw */
};

//Definitions for FSM Factory (produces FSM object according to given AC)
typedef smsc::util::FactoryXArg_T<AppContextID, TransactionFSM_AC,
                                                TransactionParms> TrFSMFactory;
typedef TrFSMFactory * (*TrFSMFactoryProducer)(void);

//Definition for FSM Registry (registry of FSM object factories according to AC)
class TrFSMRegistryIFace {
public:
    virtual bool regFactory(TrFSMFactoryProducer fif) = 0;
    virtual const TrFSMFactory * getFactory(const AppContextID & ac_oid) const = 0;
};


//Application Contexts Registry interface
class ROSFactoryProducer;   //TODO: analog of smsc::inman::comp::ROSFactoryProducer
class ROSComponentsFactory; //TODO: analog of smsc::inman::comp::ROSComponentsFactory
class ACRegistryIFace {
    virtual bool regFactory(ROSFactoryProducer fif) = 0;
    virtual const ROSComponentsFactory * getFactory(const EncodedOID & ac_oid) const = 0;
};

//transaction access only refPtr
typedef MTRefPtr_T<TransactionFSM_AC> TransactionRFP;

//TCAP message wrapper (structured message + original SUA buffer)
class TCMessageInfo : public TCAPMessage {
protected:
    TransactionRFP  trFSM;
    SUAMessageInd   suaMsg;

public:
    TCMessageInfo()
        : TCAPMessage(), trFSM(0)
    { }
    ~TCMessageInfo()
    { }

    inline MessageInfo & SUAMsg(void) const { return suaMsg; }
    //returns false if transaction is already defined
    inline bool attachTransaction(const TransactionRFP & use_trFSM)
    {
        if (trFSM && trFSM != use_trFSM)
            return false;
        trFSM = use_trFSM;
        return true;
    }

    // Process remote point message indications
    inline bool updateTransaction(void); /* throw */
    {
        return  trFSM.get() ? trFSM->processTMsg(this) : false;
    }
};

//TCProvider interface (interaction with SUA layer and application dispatcher)
class TCProviderIFace {
public:
    //attempt to demux TCAP message buffer according to ACRegistry data
    virtual bool demuxTMsg(TCMessageInfo & tc_msg) const /* throw Exception */ = 0;
    //attaches existing TCProvider transaction to TCAP message (increases
    //transaction refPtr), if do_alloc == true, the missing transaction is created
    virtual bool attachTransaction(TCMessageInfo & tc_msg, 
                                   bool do_alloc = false) /* throw() */ const = 0;
    //creates idle transaction (searches for FSM registered), returns refPtr to transaction FSM
    virtual TransactionRFP createTransaction(const AppContextID * ac_id = 0, uint8_t use_opcode = 0) /* throw() */ = 0;
};

class TCProvider : public TCProviderIFace, TCProviderTReqIFace {
protected:
    //describes transaction state in terms of TCProvider
    class TransactionState {
    protected:
        //TODO:
    public:
        TransactionState()
        { }
        ~TransactionState()
        { }
    };

    typedef UTZObjectReferee_T<TransactionFSM_AC> TransactionURFR;

    struct TransactionInfo {
        TransactionURFR  fsmRfr;    //FSM object maintaining this transaction
        TransactionState state;     //transaction state in terms of TCProvider
    };

    typedef std::map<TransactionId /**/, TransactionInfo /**/> TransactionsMap;

    class TransactionRegistry {
    protected:
        mutable Mutex   _sync;
        TransactionsMap trMap;

    public:
        TransactionRegistry()
        { }
        ~TransactionRegistry()
        { }

        //Returns FSM implementation for given AC/Operation, if ac_id is
        //unsupported should return NULL.
        //NOTE: if given Operation has no corresponding AC defined (ac_id == 0)
        //it's assumed to be the predefined AC: _ac_contextlessOperation 
        TransactionFSM_AC * allcTrFSM(const TransactionParms & use_parms);
        //searches existing transactions for one with given id.
        TransactionFSM_AC * findTrFSM(const TransactionId & use_id) const;
        //returns number of active references to transaction,
        //zero - means transaction is deleted
        unsigned rlseTrFSM(const TransactionId & use_id);
    };

public:
    TCProvider(/*...*/)
    { }

    //Manages initialization of codecs for components of ROS operations of supported ACs
    ACRegistryIFace * ACRegistry(void) const;
    //Manages initialization of FSMs for operations of supported ACs
    TrFSMRegistryIFace * TrFSMRegistry(void) const;

};

}; //tcap
}; //eyelinecom

#endif /* __TCAP_TRANSACTION_DEFS_HPP */

