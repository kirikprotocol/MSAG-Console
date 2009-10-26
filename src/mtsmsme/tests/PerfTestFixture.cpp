#include "mtsmsme/tests/PerfTestFixture.hpp"
#include "mtsmsme/processor/SccpSender.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "logger/Appender.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PerfTestFixture);

using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::processor::TCO;
using smsc::logger::Appender;
void PerfTestFixture::setUp()
{
  Logger::Init();
  logger = Logger::getInstance("perftest");
}
void PerfTestFixture::tearDown()
{
  Logger::Shutdown();
}

class NullSccpSender: public SccpSender {
  public:
    void send(uint8_t cdlen, uint8_t *cd, uint8_t cllen, uint8_t *cl,
        uint16_t ulen, uint8_t *udp){}
};
class NullAppender : public Appender {
public:
  NullAppender(): Appender("-") {};
  virtual ~NullAppender() {};
  virtual void log(timeval tv,
                   const char logLevelName,
                   const char * const category,
                   const char * const message) throw() {};
};
void PerfTestFixture::perftest()
{
  try
  {
    smsc_log_debug(logger, "======== PerfTestFixture::perftest ========\n");
    NullSccpSender nullsender;
    NullAppender nullappender;
    TCO mtsms(10000);
    mtsms.setSccpSender(&nullsender);
    {
      //shut up debug loggers for drystone
      Logger* extlogger;
      extlogger = 0;
      extlogger = Logger::getInstance("mt.sme.tco");
      extlogger->setAppender(&nullappender);
      extlogger = 0;
      extlogger = Logger::getInstance("mt.sme.sri4sm");
      extlogger->setAppender(&nullappender);
    }


    uint8_t cd[] ={
    0x12, 0x06, 0x00, 0x12, 0x04, 0x97, 0x31, 0x89,
    0x06, 0x00, 0x26, 0x38, 0x95};
    uint8_t cl[] = {
    0x12, 0x08, 0x00, 0x12, 0x04, 0x97, 0x31, 0x89,
    0x06, 0x00, 0x36, 0x00};
    uint8_t ui[] = {
    0x62, 0x49, 0x48, 0x04, 0xBA, 0xBE, 0x05, 0x89,
    0x6B, 0x1E, 0x28, 0x1C, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xA0, 0x11, 0x60,
    0x0F, 0x80, 0x02, 0x07, 0x80, 0xA1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x14, 0x02,
    0x6C, 0x21, 0xA1, 0x1F, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x2D, 0x30, 0x17, 0x80, 0x09, 0x91, 0x97,
    0x31, 0x89, 0x06, 0x00, 0x26, 0x38, 0x95, 0x81,
    0x01, 0xFF, 0x82, 0x07, 0x91, 0x97, 0x31, 0x89,
    0x06, 0x00, 0x36};

    hrtime_t start, end;
    int i, iters = 10000;

    start = gethrtime();
    for (i = 0; i < iters; i++)
    {
      mtsms.NUNITDATA(sizeof(cd),cd,sizeof(cl),cl,sizeof(ui),ui);
    }
    end = gethrtime();

    smsc_log_info(logger,
        "TCAP Engine speed = %f pkt/sec\n", iters * 1000000000.0 / (end - start));

  } catch (std::exception& ex)
  {
    smsc_log_error(logger, " catched unexpected exception [%s]", ex.what());
  }
}

