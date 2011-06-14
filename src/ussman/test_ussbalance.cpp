//#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
//#endif /* MOD_IDENT_ON */

#include <stdio.h>
#include <string.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <list>

#include "util/config/XCFManager.hpp"
#include "util/config/XCFView.hpp"
using smsc::util::config::CStrSet;
using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;
using smsc::util::config::XConfigView;

#include "ussman/interaction/MsgUSSProcess.hpp"
using smsc::inman::interaction::PacketBuffer_T;
using smsc::ussman::comp::USSOperationData;
using smsc::ussman::comp::USSDataString;

#include "util/BinDump.hpp"
#include "inman/common/cvtutil.hpp"


using smsc::util::TonNpiAddress;

static smsc::logger::Logger* logger;

static ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)
{
  size_t	nleft;
  ssize_t	nread;
  char	*ptr;

  ptr = (char*)vptr;
  nleft = n;
  while (nleft > 0) {
    if ( (nread = read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR)
        nread = 0;		/* and call read() again */
      else
        return(-1);
    } else if (nread == 0)
      break;				/* EOF */

    nleft -= nread;
    ptr   += nread;
  }
  return(n - nleft);		/* return >= 0 */
}

static ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
  size_t		nleft;
  ssize_t		nwritten;
  const char	*ptr;

  ptr = (char*)vptr;
  nleft = n;
  while (nleft > 0) {
    if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
      if (errno == EINTR)
        nwritten = 0;		/* and call write() again */
      else
        return(-1);			/* error */
    }

    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(n);
}

struct SubscriberSSCfg {
  TonNpiAddress _msAddr;  //subcriber ISDN address
  uint8_t       _inSSN;   //SS provider SSN
  TonNpiAddress _inAddr;  //SS provider ISDN address

  SubscriberSSCfg() : _inSSN(0)
  { }
};


struct UssTestService_CFG {
  std::string   _peerHost;  //USSMan host
  in_port_t     _peerPort;  //USSMan port

  std::list<SubscriberSSCfg>  _abSS;

  UssTestService_CFG() : _peerPort(0) 
  { }
};

class UssTestServiceConfig : public UssTestService_CFG {
protected:
  void readSSParams(const XConfigView & cfg_sec, SubscriberSSCfg & ssp_cfg) /*throw(std::exception)*/
  {
    const char * cStr = 0;
    try { cStr = cfg_sec.getString("msAddr");
    } catch (const std::exception & exc) { }
    if (!cStr || !ssp_cfg._msAddr.fromText(cStr))
      throw ConfigException("'msAddr' is invalid or missing");

    cStr = 0;
    try { cStr = cfg_sec.getString("inAddr");
    } catch (const std::exception & exc) { }
    if (!cStr || !ssp_cfg._inAddr.fromText(cStr))
      throw ConfigException("'inAddr' is invalid or missing");

    uint32_t iTmp = 0x100;
    try { iTmp = (uint32_t)cfg_sec.getInt("inSSN");
    } catch (const std::exception & exc) { }
    if (!iTmp || (iTmp > 0xFF))
      throw ConfigException("'inSSN' is invalid or missing");
    ssp_cfg._inSSN = (uint8_t)iTmp;
  }


  void readSSSection(const XConfigView & cfg_sec, std::list<SubscriberSSCfg> & sscfg_list) /*throw(std::exception)*/
  {
    std::auto_ptr<CStrSet>  nmSubs(cfg_sec.getShortSectionNames());
    for (CStrSet::const_iterator it = nmSubs->begin(); it != nmSubs->end(); ++it) {
      SubscriberSSCfg   ssCfg;
      XConfigView       ssSec;

      cfg_sec.getSubConfig(ssSec, it->c_str());
      printf("Reading '%s' parameters ..\n", it->c_str());
      readSSParams(ssSec, ssCfg);
      sscfg_list.push_back(ssCfg);
    }
  }

public:
  void read(const Config & use_xcfg) /*throw(std::exception)*/
  {
    if (!use_xcfg.findSection("test_ussbalance"))
      throw ConfigException("\'test_ussbalance\' section is missed");

    XConfigView tstCfg(use_xcfg, "test_ussbalance");

    const char * cStr = 0;
    try { cStr = tstCfg.getString("peerHost");
    } catch (const std::exception & exc) { }
    if (!cStr || !*cStr)
      throw ConfigException("'peerHost' is invalid or missing");
    _peerHost = cStr;

    uint32_t iTmp = 0;
    try { iTmp = (uint32_t)tstCfg.getInt("peerPort");
    } catch (const std::exception & exc) { }
    if (!iTmp)
      throw ConfigException("'peerPort' is invalid or missing");
    _peerPort = iTmp;

    if (!tstCfg.findSubSection("AbonentsDb"))
      throw ConfigException("\'AbonentsDb\' subsection is missed");

    XConfigView ssSec;
    tstCfg.getSubConfig(ssSec, "AbonentsDb");
    printf("Reading 'AbonentsDb' subsection ..\n");
    readSSSection(ssSec, _abSS);
    if (_abSS.empty())
      throw ConfigException("no valid abonent SS provider defined");
  }
};


static void printVersion()
{
  printf("%s", &ident[0]);
}

static void printUsage(const char* progName)
{
  printf("Usage: %s [-h] [-v] [-I IMSIvalue] [cfg_file [needDcs]]\n", progName);
}

#include <logger/Logger.h>

static void init_logger()
{
  smsc::logger::Logger::Init();
  logger = smsc::logger::Logger::getInstance("smsc.uss_tst");
}


int doSSRequest(int sock_fd, uint32_t & dlg_id, const SubscriberSSCfg & ssp_cfg)
{
  ++dlg_id;
  printf("\nComposing USS Request[%u] for ISDN#=%s ..", dlg_id, ssp_cfg._msAddr.toString().c_str());

  smsc::ussman::interaction::SPckUSSRequest oMsg;
  PacketBuffer_T<512> sendBuf;

  try {
    oMsg.setDlgId(dlg_id);
    oMsg._Cmd._msIsdn = ssp_cfg._msAddr;
    oMsg._Cmd._ussData.setText("*100#");
    oMsg._Cmd._inAddr = ssp_cfg._inAddr;
    oMsg._Cmd._inSSN = ssp_cfg._inSSN;
//    if (imsi)
//      oMsg._Cmd._imsi = imsi;

    oMsg.serialize(sendBuf);
    sendBuf.setPos(0);

  } catch (const std::exception & exc) {
    fprintf(stderr, "packet serialization exception: %s", exc.what());
    return (-3);
  }
  uint32_t dataSz = sendBuf.getDataSize();

  union {
    uint8_t   _buf[sizeof(uint32_t)];
    uint32_t  _uval;
  } netDataSz;

  netDataSz._uval = htonl(dataSz);
  ssize_t n = writen(sock_fd, (uint8_t*)&netDataSz._buf, 4);
  if ( (n < 0) || ((uint32_t)n != sizeof(netDataSz)) ) {
    fprintf(stderr,"write: errno=%d\n", errno);
    perror("write failed");
    return (-4);
  }

  n = writen(sock_fd, (uint8_t*)sendBuf.get(), dataSz);
  if ( (n < 0) || ((uint32_t)n != dataSz) ) {
    fprintf(stderr,"write: errno=%d\n", errno);
    perror("write failed");
    return (-4);
  }

  size_t st = readn(sock_fd, (uint8_t*)&netDataSz._buf, 4);
  if (st != 4) {
    if (!st)
      fprintf(stderr,"got EOF\n");
    else {
      fprintf(stderr,"read: errno=%d\n", errno);
      perror("read failed");
    }
    exit(-5);
  }
  dataSz = ntohl(netDataSz._uval);

  PacketBuffer_T<512> recvBuf;
  recvBuf.extend((uint32_t)dataSz);

  if ((dataSz = (uint32_t)readn(sock_fd, recvBuf.get(), dataSz)) < 0) {
    fprintf(stderr,"read(recvBuf): errno=%d\n", errno);
    perror("read failed");
    return 1;
  }
  recvBuf.setDataSize(dataSz);
  recvBuf.setPos(0);

  smsc::ussman::interaction::SPckUSSResult iMsg;
  try {
    iMsg.deserialize(recvBuf, smsc::inman::interaction::SerializablePacketIface::dsmComplete);
  } catch (const std::exception & exc) {
    fprintf(stderr, "corrupted response received: %s", exc.what());
    return -3;
  }

  char tBuf[smsc::ussman::interaction::USSResultMessage::_ussRes_strSZ];
  iMsg._Cmd.log2str(tBuf, (unsigned)sizeof(tBuf));
  printf("Received USS Result[%u]:\n  %s\n", dlg_id, &tBuf[0]);
  return 0;
}

int main(int argc, char** argv)
{
  const char * imsi = NULL;
  int processedArgs = 0;
  int needDcs = 0;
  const char *  cfgFile = "test_ussbalance.cfg";  

  init_logger();

  if (argc > 1) {
    if ( !strcmp(argv[1], "-v") ) {
      printVersion(); return 0;
    } else if ( !strcmp(argv[1], "-h") ) {
      printUsage(argv[0]); return 0;
    } else {
      if ( !strcmp(argv[1], "-I") ) {
        imsi = argv[2];
        processedArgs = 2;
      }
      if ( argc  == processedArgs + 3 ) {
        needDcs = atoi (argv[processedArgs++ + 1]);
      }
      cfgFile = argv[processedArgs + 1];
    }
  }

  UssTestServiceConfig cfg;
  try {
    std::auto_ptr<Config> config(XCFManager::getInstance().getConfig(cfgFile)); //throws
    cfg.read(*config.get());
  } catch (const smsc::util::config::ConfigException& exc) {
    fprintf(stderr, "Configuration invalid: %s. Exiting", exc.what());
    return -1;
  }

  struct sockaddr_in peerAddr;
  memset(&peerAddr, 0, sizeof(peerAddr));
  peerAddr.sin_family = AF_INET;
  peerAddr.sin_port = htons(cfg._peerPort);

  int sockfd;
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("inet_pton failed");
    exit(-2);
  }

  ssize_t st = inet_pton(AF_INET, cfg._peerHost.c_str(), &peerAddr.sin_addr);
  if ( st < 0 ) {
    perror("inet_pton failed");
    exit(-2);
  }

  if ( connect(sockfd,(sockaddr*)&peerAddr,(int)sizeof(peerAddr)) < 0 ) {
    perror("connect failed");
    exit(-2);
  }

  int res = 0;
  uint32_t dlgId = 0;
  for (std::list<SubscriberSSCfg>::const_iterator
        it = cfg._abSS.begin(); !res && (it != cfg._abSS.end()); ++it) {
    doSSRequest(sockfd, dlgId, *it);
  }
  exit(res);
}
