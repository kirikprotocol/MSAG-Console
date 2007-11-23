#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <inman/uss/ussmessages.hpp>
#include <inman/interaction/serializer.hpp>

#include <util/BinDump.hpp>
#include <inman/common/cvtutil.hpp>
#include <inman/comp/map_uss/MapUSSComps.hpp>

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

struct UssTestService_CFG {
  UssTestService_CFG() : inSSN(0), peerPort(0) {}

  std::string msAddr;
  std::string inAddr;
  uint8_t inSSN;
  std::string peerHost;
  in_port_t peerPort;
};

class UssTestServiceConfig : public UssTestService_CFG
{
public:
  void read(smsc::util::config::Manager& manager)
  {
    if (!manager.findSection("test_ussbalance"))
      throw smsc::util::config::ConfigException("\'test_ussbalance\' section is missed");
    smsc::util::config::ConfigView ussTestBalanceCfg(manager, "test_ussbalance");

    peerHost = ussTestBalanceCfg.getString("peerHost");
    peerPort = ussTestBalanceCfg.getInt("peerPort");

    msAddr = ussTestBalanceCfg.getString("msAddr");
    inAddr = ussTestBalanceCfg.getString("inAddr");
    inSSN = ussTestBalanceCfg.getInt("inSSN");
  }
};

static size_t appendDataToBuf(uint8_t *buf, uint8_t *data, uint32_t len, size_t* offset) {
  memcpy(buf+*offset, data, len); *offset += len;
  return len;
}

static void printVersion()
{
  printf("version: $Revision$\n");
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

int main(int argc, char** argv)
{
  const char* imsi = NULL;
  int processedArgs=0;
  try {
    init_logger();
    const char *  cfgFile = "test_ussbalance.cfg";
    int needDcs=0;
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
      smsc::util::config::Manager::init(cfgFile);
      smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();
      cfg.read(manager);
    } catch (smsc::util::config::ConfigException& exc) {
      fprintf(stderr, "Configuration invalid: %s. Exiting", exc.what());
      return 1;
    }

    struct sockaddr_in peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(cfg.peerPort);

    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
      perror("inet_pton failed");
      return 1;
    }

    ssize_t st = inet_pton(AF_INET, cfg.peerHost.c_str(), &peerAddr.sin_addr);
    if ( st < 0 ) {
      perror("inet_pton failed");
      return 1;
    }

    if ( connect(sockfd,(sockaddr*)&peerAddr,sizeof(peerAddr)) < 0 ) {
      perror("connect failed");
      return 1;
    }

    uint8_t buf[1000];
    uint16_t cmdId = htons(1);
    uint32_t reqId = htonl(10);
    uint8_t dcs = 0x0F;

    uint8_t ussData[] = "*100#";
    uint8_t ussDataLen = strlen((char*)ussData);

    //  uint8_t msAddr[] = "+79133821781";
    uint8_t msAddrLen = cfg.msAddr.size();

    //  uint8_t inAddr[] = "+79139860005";
    uint8_t inAddrLen = cfg.inAddr.size();

    size_t offset=0;
    size_t dataSz = appendDataToBuf(buf, (uint8_t*)&cmdId, sizeof(cmdId), &offset);
    dataSz += appendDataToBuf(buf, (uint8_t*)&reqId, sizeof(reqId), &offset);

    uint8_t flg;
    if (needDcs) flg = 0;
    else flg = 1;
    dataSz += appendDataToBuf(buf, (uint8_t*)&flg, sizeof(flg), &offset);

    if (needDcs) {
      dataSz += appendDataToBuf(buf, (uint8_t*)&dcs, sizeof(dcs), &offset);
      uint8_t ussdStr[MAP_MAX_USSD_StringLength];
      uint8_t ussdStrSz = (uint8_t)smsc::cvtutil::packTextAs7BitPadded((const char*)ussData, ussDataLen, ussdStr);
      dataSz += appendDataToBuf(buf, (uint8_t*)&ussdStrSz, sizeof(ussdStrSz), &offset);
      dataSz += appendDataToBuf(buf, (uint8_t*)ussdStr, ussdStrSz, &offset);
    } else {
      dataSz += appendDataToBuf(buf, (uint8_t*)&ussDataLen, sizeof(ussDataLen), &offset);
      dataSz += appendDataToBuf(buf, (uint8_t*)ussData, ussDataLen, &offset);
    }
    dataSz += appendDataToBuf(buf, (uint8_t*)&msAddrLen, sizeof(msAddrLen), &offset);
    dataSz += appendDataToBuf(buf, (uint8_t*)cfg.msAddr.c_str(), msAddrLen, &offset);
    dataSz += appendDataToBuf(buf, (uint8_t*)&cfg.inSSN, sizeof(cfg.inSSN), &offset);
    dataSz += appendDataToBuf(buf, (uint8_t*)&inAddrLen, sizeof(inAddrLen), &offset);
    dataSz += appendDataToBuf(buf, (uint8_t*)cfg.inAddr.c_str(), inAddrLen, &offset);
    dataSz += appendDataToBuf(buf, (uint8_t*)cfg.inAddr.c_str(), inAddrLen, &offset);
    if ( imsi )
      dataSz += appendDataToBuf(buf, (uint8_t*)imsi, strlen(imsi), &offset);

    printf("Make balance request for ISDN#=%s\n",cfg.msAddr.c_str());
    uint32_t netDataSz = htonl(dataSz);
    if ( writen(sockfd, (uchar_t*)&netDataSz, sizeof(netDataSz)) != sizeof(netDataSz) ) {
      fprintf(stderr,"write: errno=%d\n", errno);
      perror("write failed");
      return 1;
    }

    if ( writen(sockfd, (uchar_t*)buf, dataSz) != dataSz ) {
      fprintf(stderr,"write: errno=%d\n", errno);
      perror("write failed");
      return 1;
    }

    st = readn(sockfd, (uchar_t*)&netDataSz, sizeof(netDataSz));
    if ( st != sizeof(netDataSz) ) {
      if ( !st )
        fprintf(stderr,"got EOF\n");
      else {
        fprintf(stderr,"read: errno=%d\n", errno);
        perror("read failed");
      }
      return 1;
    }
    dataSz = ntohl(netDataSz);

    std::vector<uint8_t> responseBuf(dataSz);

    if ((dataSz=readn(sockfd, &responseBuf[0], dataSz)) < 0) {
      fprintf(stderr,"read(responseBuf): errno=%d\n", errno);
      perror("read failed");
      return 1;
    }

    smsc::inman::interaction::SerializerUSS* serializer = smsc::inman::interaction::SerializerUSS::getInstance();

    smsc::inman::interaction::ObjectBuffer responseObjectBuffer(1024);
    responseObjectBuffer.Write(0, &responseBuf[0], responseBuf.size());

    smsc::inman::interaction::USSPacketAC *ussPacket = static_cast<smsc::inman::interaction::USSPacketAC*>(serializer->deserialize(responseObjectBuffer));
    smsc::inman::interaction::USSResultMessage *message = static_cast<smsc::inman::interaction::USSResultMessage*>(ussPacket->pCmd());
    if ( message->getStatus() == smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_OK ) {
      std::string ussAsString;
      if ( message->getFlg() == 1 ) 
        printf("Got response as Latin1 text: %s\n", message->getLatin1Text());
      else {
        if ( message->getUSSDataAsLatin1Text(ussAsString) )
          printf("Got response as ussd data: %s\n", ussAsString.c_str());
        else {
          smsc::inman::interaction::USSDATA_T resultUssData = message->getUSSData();
          printf("Got response ussd data: %s\n", smsc::util::DumpHex(resultUssData.size(), &resultUssData[0]).c_str());
        }
      }
    } else 
      fprintf(stderr, "Got error\n");
  } catch (std::exception& ex) {
    fprintf(stderr, "Catch exception=[%s]\n",ex.what());
    return 1;
  }
  return 0;
}
