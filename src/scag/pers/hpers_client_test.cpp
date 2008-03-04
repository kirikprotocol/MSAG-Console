#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include "version.inc"

#include "PersClient.h"
#include "scag/config/ConfigManager.h"

using namespace std;
using namespace smsc::logger;
using namespace scag::pers::client;
using namespace scag::pers;
using namespace scag::config;
using namespace smsc::util::config;

const int ITER_CNT = 10;
const int BATCH_COUNT = 6;

extern "C" static void atExitHandler(void)
{
    Logger::Shutdown();
}

const char* PREFIX = "891389";

const int REQ_CNT = 20;

int testAbonentProfile(PersClient& pc, int pf_number) {
  Logger* logger = Logger::getInstance("hpersc");
  char s[20], tv[20], tvs[30], tvb[30], tvd[30];		

  sprintf(s, "%s%05d", PREFIX, pf_number);
  sprintf(tv, "test_val%05d", pf_number);			
  sprintf(tvs, "test_val_string%05d", pf_number);						
  sprintf(tvb, "test_val_bool%05d", pf_number);						
  sprintf(tvd, "test_val_date%05d", pf_number);		

  int success_req = 0;
  try {
    Property prop;
    SerialBuffer sb;
    int int_val = 500;

    try {
      prop.setInt(tv, int_val, FIXED, -1, 20);
      pc.SetProperty(PT_ABONENT, s, prop);
      ++success_req;
    } catch(const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: set property %s persclientexception: %s",
                      s, tv, e.what());
      int_val = 0;
    }

    bool bool_val = false;
    try {
      prop.setBool(tvb, bool_val, R_ACCESS, -1, 25);
      pc.SetProperty(PT_ABONENT, s, prop);
      ++success_req;
    } catch(const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: set property %s persclientexception: %s",
                      s, tvb, e.what());
      bool_val = true;
    }

    std::string str_val = "test_string";
    try {
      prop.setString(tvs, str_val.c_str(), W_ACCESS, -1, 25);
      pc.SetProperty(PT_ABONENT, s, prop);
      ++success_req;
    } catch(const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: set property %s persclientexception: %s",
                      s, tvs, e.what());
    }

    try {
      prop.setDate(tvd, 111111, INFINIT, -1, 25);
      pc.SetProperty(PT_ABONENT, s, prop);
      ++success_req;
    } catch(const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: set property %s persclientexception: %s",
                      s, tvd, e.what());
    }

    try {
      pc.GetProperty(PT_ABONENT, s, tv, prop);
      if (int_val && prop.getIntValue() != int_val) {
        smsc_log_error(logger, ">>ABONENT %s: get property %s return wrong value", s, tv);
      }
      ++success_req;
      smsc_log_debug(logger,  ">>ABONENT %s: get int %s", s, prop.toString().c_str());
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: get property %s: persclientexception: %s",
                     s, tv, e.what());
    }

    try {
      pc.GetProperty(PT_ABONENT, s, tvb, prop);
      if (!bool_val && prop.getBoolValue() != bool_val) {
        smsc_log_error(logger, ">>ABONENT %s: get property %s return wrong value", s, tvb);
      }
      smsc_log_debug(logger,  ">>ABONENT %s: get bool %s", s, prop.toString().c_str());
      ++success_req;
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: get property %s: persclientexception: %s",
                     s, tvb, e.what());
    }

    try {
      pc.GetProperty(PT_ABONENT, s, tvs, prop);
      if (prop.getStringValue().compare(str_val) != 0) {
        smsc_log_error(logger, ">>ABONENT %s: get property %s return wrong value", s, tvs);
      }
      smsc_log_debug(logger,  ">>ABONENT %s: get string %s", s, prop.toString().c_str());
      ++success_req;
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s get property %s: persclientexception: %s",
                     s, tvs, e.what());
    }
    try {
      pc.GetProperty(PT_ABONENT, s, tvd, prop);
      smsc_log_debug(logger,  ">>ABONENT %s: get string1 %s", s, prop.toString().c_str());
      ++success_req;
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s get property name '%s': persclientexception: %s",
                     s, tvd, e.what());
    }

    int old_val = 0;
    try {
      pc.GetProperty(PT_ABONENT, s, tv, prop);
      smsc_log_debug(logger,  ">>ABONENT %s: get int(before inc) %s", s, prop.toString().c_str());
      old_val = prop.getIntValue();
      ++success_req;
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: get property %s: persclientexception: %s",
                     s, tv, e.what());
    }
    try {
      int inc_val = 100;
      prop.setInt(tv, inc_val, FIXED, -1, 20);
      int inc_res = pc.IncProperty(PT_ABONENT, s, prop);
      if (inc_res != inc_val + old_val) {
        smsc_log_error(logger, ">>ABONENT %s: inc property %s return wrong value", s, tv);
      }
      ++success_req;
      smsc_log_debug(logger,  ">>ABONENT %s: inc result = %d", s, inc_res);
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: inc property %s: persclientexception: %s",
                     s, tv, e.what());
    }
    try {
      pc.GetProperty(PT_ABONENT, s, tv, prop);
      smsc_log_debug(logger,  ">>ABONENT %s: get int(after inc/before inc-mod) %s", s, prop.toString().c_str());
      old_val = prop.getIntValue();
      ++success_req;
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: get property %s: persclientexception: %s",
                     s, tv, e.what());
      old_val = 0;
    }

    try {
      int inc_mod_val = 205;
      int mod_val = 10;
      prop.setInt(tv, inc_mod_val, FIXED, -1, 20);
      int inc_mod_res = pc.IncModProperty(PT_ABONENT, s, prop, mod_val);
      if (inc_mod_res != (old_val + inc_mod_val) % mod_val) {
        smsc_log_error(logger, ">>ABONENT %s: inc-mod property %s return wrong value", s, tv);
      }
      ++success_req;
      smsc_log_debug(logger,  ">>ABONENT %s: inc-mod result = %d", s, inc_mod_res);
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: inc-mod property %s: persclientexception: %s",
                     s, tv, e.what());
    }
    try {
      pc.GetProperty(PT_ABONENT, s, tv, prop);
      smsc_log_debug(logger,  ">>ABONENT %s: get int(after inc-mod) %s", s, prop.toString().c_str());
      ++success_req;
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: get property %s: persclientexception: %s",
                     s, tv, e.what());
    }

    try {
      pc.DelProperty(PT_ABONENT, s, tv);
      smsc_log_debug(logger,  ">>ABONENT %s: delete property %s", s, tv);
      ++success_req;
    } catch (const PersClientException &e) {
      if (e.getType() == CANT_CONNECT) throw;
      smsc_log_error(logger, ">>ABONENT %s: del property %s: persclientexception: %s",
                     s, tv, e.what());
    }

    prop.setInt(tv, 234567, FIXED, -1, 20);
    pc.SetProperty(PT_SERVICE, pf_number + 1, prop);
    ++success_req;

    prop.setBool(tvb, false, R_ACCESS, -1, 25);
    pc.SetProperty(PT_PROVIDER, pf_number + 1, prop);
    ++success_req;

    prop.setString(tvs, "test_string", W_ACCESS, -1, 25);
    pc.SetProperty(PT_OPERATOR, pf_number + 1, prop);
    ++success_req;

    pc.GetProperty(PT_SERVICE, pf_number + 1, tv, prop);
    smsc_log_debug(logger,  ">>SERVICE %d: get int %s", pf_number + 1, prop.toString().c_str());
    ++success_req;

    pc.GetProperty(PT_PROVIDER, pf_number + 1, tvb, prop);
    smsc_log_debug(logger,  ">>PROVIDER %d: get bool %s", pf_number + 1, prop.toString().c_str());
    ++success_req;

    pc.GetProperty(PT_OPERATOR, pf_number + 1, tvs, prop);
    smsc_log_debug(logger,  ">>OPERATOR %d: get string %s", pf_number + 1, prop.toString().c_str());
    ++success_req;

  } catch(const PersClientException &e) {
    if (e.getType() == CANT_CONNECT) throw;
    smsc_log_error(logger, "testAbonentProfile: persclientexception: %s", e.what());
  } 
  smsc_log_warn(logger,  ">>ABONENT %s: success requests %d, error requests %d, total requests=%d",
                  s, success_req, (REQ_CNT - success_req), REQ_CNT);
  return success_req;
}

int testBatch(PersClient& pc, int pf_number1, int pf_number2, int pf_number3, int pf_number4) {
  Logger* logger = Logger::getInstance("hpersc");

  char s4[20], s1[20], s2[20], s3[20], tv[20], tvs[30], tvb[30], tvd[30];		

  sprintf(s1, "%s%05d", PREFIX, pf_number1);
  sprintf(s2, "%s%05d", PREFIX, pf_number2);
  sprintf(s3, "%s%05d", PREFIX, pf_number3);
  sprintf(s4, "%s%05d", PREFIX, pf_number4);
  sprintf(tv, "test_val_batch%05d", pf_number1);			
  sprintf(tvs, "test_val_string_batch%05d", pf_number2);						
  sprintf(tvb, "test_val_bool_batch%05d", pf_number3);						
  //sprintf(tvd, "test_val_date%05d", pf_number4);	


  Property prop;
  SerialBuffer sb;

  pc.PrepareBatch(sb);

  int int_val = 234567;

  prop.setInt(tv, int_val, FIXED, -1, 20);
  pc.SetPropertyPrepare(PT_ABONENT, s1, prop, sb);
  pc.GetPropertyPrepare(PT_ABONENT, s1, tv, sb);

  bool bool_val = false;
  prop.setBool(tvb, bool_val, R_ACCESS, -1, 25);
  pc.SetPropertyPrepare(PT_ABONENT, s2, prop, sb);
  pc.GetPropertyPrepare(PT_ABONENT, s2, tvb, sb);

  std::string str_val = "test_string_batch";
  prop.setString(tvs, str_val.c_str(), W_ACCESS, -1, 25);
  pc.SetPropertyPrepare(PT_ABONENT, s3, prop, sb);
  pc.GetPropertyPrepare(PT_ABONENT, s3, tvs, sb);

  pc.FinishPrepareBatch(BATCH_COUNT, sb);
  pc.RunBatch(sb);

  int success_req = 0;
  try {
    pc.SetPropertyResult(sb);
    ++success_req;
  } catch(const PersClientException &e) {
    if (e.getType() == CANT_CONNECT) throw;
    smsc_log_error(logger, "testBatch: BATCH >>ABONENT %s: set property %s persclientexception: %s",
                    s1, tv, e.what());
    int_val = 0;
  }
  try {
    pc.GetPropertyResult(prop, sb);
    if (int_val && prop.getIntValue() != int_val) {
      smsc_log_error(logger, "BATCH >>ABONENT %s: GetProperty return wrong value");
    }
    ++success_req;
    smsc_log_debug(logger,  "BATCH >>ABONENT %s: get int %s", s1, prop.toString().c_str());
  } catch(const PersClientException &e) {
    if (e.getType() == CANT_CONNECT) throw;
    smsc_log_error(logger, "testBatch: BATCH >>ABONENT %s: get property %s persclientexception: %s",
                    s1, tv, e.what());
  }

  try {
    pc.SetPropertyResult(sb);
    ++success_req;
  } catch(const PersClientException &e) {
    if (e.getType() == CANT_CONNECT) throw;
    smsc_log_error(logger, "testBatch: BATCH >>ABONENT %s: set property %s persclientexception: %s",
                    s2, tvb, e.what());
    bool_val = true;
  }
  try {
    pc.GetPropertyResult(prop, sb);
    if (!bool_val && prop.getBoolValue() != bool_val) {
      smsc_log_error(logger, "BATCH >>ABONENT %s: GetProperty return wrong value");
    }
    smsc_log_debug(logger,  "BATCH >>ABONENT %s: get bool %s", s2, prop.toString().c_str());
    ++success_req;
  } catch(const PersClientException &e) {
    if (e.getType() == CANT_CONNECT) throw;
    smsc_log_error(logger, "testBatch: BATCH >>ABONENT %s: get property %s persclientexception: %s",
                    s2, tvb, e.what());
  }

  try {
    pc.SetPropertyResult(sb);
    ++success_req;
  } catch(const PersClientException &e) {
    if (e.getType() == CANT_CONNECT) throw;
    smsc_log_error(logger, "testBatch: BATCH >>ABONENT %s: set property %s persclientexception: %s",
                    s3, tvs, e.what());
  }
  try {
    pc.GetPropertyResult(prop, sb);
    if (prop.getStringValue().compare(str_val) != 0) {
      smsc_log_error(logger, "BATCH >>ABONENT %s: GetProperty return wrong value");
    }
    smsc_log_debug(logger,  "BATCH >>ABONENT %s: get string %s", s3, prop.toString().c_str());
    ++success_req;
  } catch(const PersClientException &e) {
    if (e.getType() == CANT_CONNECT) throw;
    smsc_log_error(logger, "testBatch: BATCH >>ABONENT %s: get property %s persclientexception: %s",
                    s3, tvs, e.what());
  }
  smsc_log_warn(logger,  "BATCH: success requests %d, error requests %d, total requests=%d",
                  success_req, (BATCH_COUNT - success_req), BATCH_COUNT);
  return success_req;
}

int main(int argc, char* argv[])
{
    int resultCode = 0;

    Logger::Init();
    Logger* logger = Logger::getInstance("hpersc");

    atexit(atExitHandler);

    try {

        ConfigManager::Init();

        PersClientConfig &client_config = ConfigManager::Instance().getPersClientConfig();

        PersClient::Init(client_config.host.c_str(), client_config.port, client_config.timeout,
                          client_config.pingTimeout);
        PersClient& pc = PersClient::Instance();

        Property prop;
        SerialBuffer sb;
        time_t t = time(NULL);
        int addr_min = client_config.minAddr;
        int addr_max = client_config.maxAddr;
        int addr_max_common = client_config.commonAddr;

        if (addr_min > addr_max) {
          smsc_log_warn(logger, "start address will be %d end address will be %d", addr_max, addr_min);
          int tmp = addr_min;
          addr_min = addr_max;
          addr_max  = tmp;
        }

        int ok_req = 0;
        int err_req = 0;
        for(int i = 0; i < ITER_CNT; i++)
        {
          int batch_ok_req = testBatch(pc, 1, addr_min, addr_max_common - 1, addr_max - 1);
          ok_req += batch_ok_req;
          err_req += (BATCH_COUNT - batch_ok_req);

          for (int j = 0; j < addr_max_common; ++j) {
            int _ok_req = testAbonentProfile(pc, j);
            ok_req += _ok_req;
            err_req += (REQ_CNT - _ok_req);
          }
          for (int j = addr_min; j < addr_max; ++j) {
            int _ok_req = testAbonentProfile(pc, j);
            ok_req += _ok_req;
            err_req += (REQ_CNT - _ok_req);
          }
          smsc_log_warn(logger,  "end %d iteration", i);
        }
        t = time(NULL) - t;
        int total_req = REQ_CNT * (addr_max_common + addr_max - addr_min) * ITER_CNT + BATCH_COUNT * ITER_CNT;
        if(t) {
          smsc_log_error(logger,  "timings: %d sec, %d req/s", t,
                          total_req/t); //24
        }
        smsc_log_error(logger,  "success requests %d, error requests %d, total requests %d",
                        ok_req, err_req, total_req);
    }
    catch (PersClientException& exc) 
    {
        smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) 
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }

    return resultCode;
}



