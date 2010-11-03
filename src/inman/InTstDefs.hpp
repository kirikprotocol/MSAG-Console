/* ************************************************************************** *
 * INMan testing console: commonly used defs and helpers
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TEST_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TEST_DEFS__

#include "core/synchronization/Event.hpp"

#include "inman/AbntContract.hpp"

#include "inman/interaction/ConnSrv.hpp"
#include "inman/interaction/connect.hpp"
#include "inman/interaction/messages.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::logger::Logger;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::core::network::Socket;

using smsc::inman::AbonentContractInfo;

using smsc::inman::interaction::ConnectSrv;
using smsc::inman::interaction::ConnectSupervisorITF;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectAC;
using smsc::inman::interaction::ConnectListenerITF;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializablePacketAC;
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPPacketAC;
using smsc::inman::interaction::INPSerializer;

/* ************************************************************************** *
 * class TSTFacadeAC: 
 * ************************************************************************** */
#define MAX_FACADE_NAME 50
class TSTFacadeAC : public ConnectSupervisorITF, public ConnectListenerITF {
protected:
    Mutex           _sync;
    Connect*        _pipe;
    ConnectSrv *    _connSrv;
    Logger*         logger;
    //logging prefix, f.ex: "TSTFacadeAC[%u]"
    char            _logId[MAX_FACADE_NAME + sizeof("[%u]") + sizeof(unsigned)*3 + 1];

    inline void do_disconnect(void)
    {
        if (_pipe) {
            _connSrv->rlseConnection(_pipe->getId());
            delete _pipe; _pipe = NULL;
        }
    }
    inline bool is_active(void) const
    { return (bool)(_pipe && (_pipe->getId() != (unsigned)(INVALID_SOCKET))); }


public:
    TSTFacadeAC(ConnectSrv * conn_srv, Logger * use_log = NULL)
        : _pipe(NULL), _connSrv(conn_srv)
    {
        logger = use_log ? use_log : Logger::getInstance("smsc.InTST");
    }
    virtual ~TSTFacadeAC()
    {
        do_disconnect();
    }

    inline void Prompt(Logger::LogLevel dlvl, const std::string & str)
    {
        fprintf(stdout, "%s\n", str.c_str());  logger->log(dlvl, "%s: %s", _logId, str.c_str());
    }
    inline void Prompt(Logger::LogLevel dlvl, const char * zstr)
    {
        fprintf(stdout, "%s\n", zstr);  logger->log(dlvl, "%s: %s", _logId, zstr);
    }

    void Disconnect(void)
    {
        MutexGuard grd(_sync);
        do_disconnect();
    }

    bool isActive(void) 
    {
        MutexGuard grd(_sync);
        return is_active();
    }

    unsigned initConnect(const char* use_host, int use_port)
    {
        MutexGuard grd(_sync);
        do_disconnect();
        {
            std::string msg;
            format(msg, "%s: connecting to InManager at %s:%d...\n", _logId, use_host, use_port);
            smsc_log_info(logger, msg.c_str());
            fprintf(stdout, "%s", msg.c_str());
        }
        Socket * socket = _connSrv->setConnection(use_host, use_port);
        if (socket) {
            _pipe = new Connect(socket, INPSerializer::getInstance(), logger);
            _pipe->addListener(this);
            _connSrv->addConnection(_pipe, this);
            {
                std::string msg;
                format(msg, "%s: connect[%u] created\n", _logId, _pipe->getId());
                smsc_log_info(logger, msg.c_str());
                fprintf(stdout, "%s", msg.c_str());
            }
            return _pipe->getId();
        }
        return 0;
    }

    //Customized variant of Connect::sendPck(): it sends specified
    //number of bytes from ObjectBuffer
    int  sendPckPart(INPPacketAC *pck, uint32_t num_bytes = 0)
    {
        ObjectBuffer    buffer(1024);
        buffer.setPos(4);
        pck->serialize(buffer);
        num_bytes = !num_bytes ? (uint32_t)buffer.getPos() - 4 : num_bytes;
        {
            uint32_t len = htonl(num_bytes);
            memcpy(buffer.get(), (const void *)&len, 4);
        }
        MutexGuard grd(_sync);
        if (is_active())
            return _pipe->send(buffer.get(), num_bytes + 4);
        Prompt(Logger::LEVEL_ERROR, "ERR: IN manager doesn't connected!");
        return 0;
    }

    // -- ConnectSupervisorITF
    bool onConnectClosed(ConnectAC * conn)
    {
        MutexGuard grd(_sync);
        _pipe = NULL;
        return true;  //ConnSrv will destroy connect
    }
    // -- ConnectListenerITF
    //virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd) = 0;
    void onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
    {
        MutexGuard grd(_sync);
        Prompt(Logger::LEVEL_ERROR, p_exc->what());
        _pipe = NULL; //ConnSrv will destroy connect
    }
};


/* ************************************************************************** *
 * class AbonentsDB (singleton): abonents registry
 * ************************************************************************** */

typedef smsc::inman::AbonentContractInfo::ContractType AbonentType;
typedef struct {
    AbonentType  abType;
    const char *  addr;
    const char *  imsi;
} AbonentPreset;

struct AbonentInfo : public AbonentContractInfo {
    TonNpiAddress  msIsdn;

    AbonentInfo(const TonNpiAddress * p_adr, ContractType cntr_type = abtUnknown,
                 const char * p_imsi = NULL)
        : AbonentContractInfo(cntr_type, p_imsi)
    { if (p_adr) msIsdn = *p_adr; }

    AbonentInfo(const char * p_adr = NULL, ContractType cntr_type = abtUnknown,
                 const char * p_imsi = NULL)
        : AbonentContractInfo(cntr_type, p_imsi)
    { msIsdn.fromText(p_adr); }

    inline bool Empty(void) const { return (bool)(!msIsdn.length); }

    inline void setContractInfo(const AbonentContractInfo& ctr_inf)
    {
        *(AbonentContractInfo*)this = ctr_inf;
    }
};

class AbonentsDB {
protected:
    typedef std::map<unsigned, AbonentInfo> AbonentsMAP;

    Mutex           _sync;
    AbonentsMAP     registry;
    unsigned        lastAbnId;

    void initDB(unsigned n_abn, const AbonentPreset * p_abn)
    {
        for (unsigned i = 0; i < n_abn; i++) {
            AbonentInfo  abn(p_abn[i].addr, p_abn[i].abType, p_abn[i].imsi);
            if (!abn.Empty())
                registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        }
    }

    AbonentsDB(void) : lastAbnId(0) { }
    ~AbonentsDB() { }

public:
    static AbonentsDB * getInstance(void)
    {
        static AbonentsDB     abnData;
        return &abnData;
    }

    static AbonentsDB * Init(unsigned n_abn, const AbonentPreset * p_abn)
    {
        AbonentsDB * adb = AbonentsDB::getInstance();
        adb->initDB(n_abn, p_abn);
        return adb;
    }

    static void printAbnInfo(FILE * stream, const AbonentInfo & abn, unsigned ab_id)
    {
        fprintf(stream, "abn.%u: %s, isdn <%s>, imsi <%s>, %s\n", ab_id,
                abn.type2Str(), abn.msIsdn.length ? abn.msIsdn.toString().c_str() : " ",
                abn.getImsi() ? abn.getImsi() : "none", abn.tdpSCF.toString().c_str());
    }

    inline unsigned getMaxAbId(void) const { return lastAbnId; }
    inline unsigned nextId(unsigned ab_id) const { return (ab_id >= lastAbnId) ? 1 : ++ab_id; }

    AbonentInfo * getAbnInfo(unsigned ab_id)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::iterator it = registry.find(ab_id);
        return (it != registry.end()) ? &((*it).second) : NULL;
    }

    unsigned searchNextAbn(AbonentType ab_type, unsigned min_id = 0)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.begin();
        if (min_id && (min_id <= registry.size())) {
            it = registry.find(min_id);
        }
        for (; it != registry.end(); it ++) {
            const AbonentInfo & abn = (*it).second;
            if (abn.ab_type == ab_type)
                return (*it).first;
        }
        return 0;
    }

    unsigned searchAbn(const std::string & addr)
    {
        TonNpiAddress   subscr;
        if (!subscr.fromText(addr.c_str()))
            return 0;
        return searchAbn(subscr);
    }

    unsigned searchAbn(const TonNpiAddress & subscr)
    {
        MutexGuard  grd(_sync);
        for (AbonentsMAP::const_iterator it = registry.begin();
                                    it != registry.end(); it ++) {
            const AbonentInfo & abn = (*it).second;
            if (abn.msIsdn == subscr)
                return (*it).first;
        }
        return 0; //unknown
    }


    void printAbnInfo(FILE * stream, unsigned ab_id)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.find(ab_id);
        if (it != registry.end()) {
            const AbonentInfo & abn = (*it).second;
            AbonentsDB::printAbnInfo(stream, abn, ab_id);
        }
    }

    //0, 0 - print ALL
    void printAbonents(FILE * stream, unsigned min_id = 0, unsigned max_id = 0)
    {
        MutexGuard  grd(_sync);
        if (!registry.size())
            return;
        if (!min_id || (min_id > registry.size()))
            min_id = 1;
        if (!max_id || (max_id > registry.size()))
            max_id = (unsigned)registry.size();

        AbonentsMAP::const_iterator it = registry.find(min_id);
        while (min_id <= max_id) {
            const AbonentInfo & abn = (*it).second;
            printAbnInfo(stream, abn, min_id);
            min_id++;
            it++;
        }
    }

    unsigned setAbnInfo(const AbonentInfo & abn)
    {
        unsigned ab_id = searchAbn(abn.msIsdn);
        MutexGuard  grd(_sync);
        if (!ab_id)
            ab_id = ++lastAbnId; 
        registry.insert(AbonentsMAP::value_type(ab_id, abn));
        return ab_id;
    }
    unsigned setAbnInfo(unsigned ab_id, const AbonentInfo & abn)
    {
        MutexGuard  grd(_sync);
        if (ab_id <= lastAbnId) {
            registry.insert(AbonentsMAP::value_type(ab_id, abn));
            return ab_id;
        }
        return 0;
    }

    unsigned addAddress(const TonNpiAddress & addr)
    {
        MutexGuard  grd(_sync);
        AbonentInfo abn(&addr);
        unsigned    ab_id = ++lastAbnId;
        registry.insert(AbonentsMAP::value_type(ab_id, abn));
        return ab_id;
    }

};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_DEFS__ */

/*



*/
