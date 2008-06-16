/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <signal.h>

#include <logger/Logger.h>

#include "scag/bill/infrastruct/Infrastructure.h"

using namespace scag::bill::infrastruct;
using namespace smsc::logger;

static Logger *logger;

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
    }
}

extern "C" void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int main(int argc, char* argv[])
{
    int resultCode = 0;

    Logger::Init();
    logger = Logger::getInstance("test");

    atexit(atExitHandler);

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);
    sigprocmask(SIG_SETMASK, &set, NULL);
    sigset(SIGTERM, appSignalHandler);
    sigset(SIGINT, appSignalHandler);

    try{
        InfrastructureImpl sm;

        sm.init(".");

//        for(int i = 0; i< 1000; i++)
        {
  //      smsc_log_info(logger, "%d",i);

        smsc_log_info(logger, ">>>>>>Provider test<<<<<<<<");
        smsc_log_info(logger, "serv_id = %d, prov_id=%d", 1, sm.GetProviderID(1));
        smsc_log_info(logger, "serv_id = %d, prov_id=%d", 3, sm.GetProviderID(3));
        smsc_log_info(logger, "serv_id = %d, prov_id=%d", 6, sm.GetProviderID(6));

        smsc_log_info(logger, ">>>>>>Operator test<<<<<<<<");
        smsc_log_info(logger, "abonent_id = +79139061234, op_id=%d", sm.GetOperatorID(Address("+79139061234")));
        smsc_log_info(logger, "abonent_id = +79139079876, op_id=%d", sm.GetOperatorID(Address("+79139079876")));
        smsc_log_info(logger, "abonent_id = +79129079876, op_id=%d", sm.GetOperatorID(Address("+79129079876")));
        smsc_log_info(logger, "abonent_id = +71112223344, op_id=%d", sm.GetOperatorID(Address("+71112223344")));
        smsc_log_info(logger, "abonent_id = +79999037097, op_id=%d", sm.GetOperatorID(Address("+79999037097")));

        smsc_log_info(logger, ">>>>>>Tariff test<<<<<<<<");
        TariffRec* tr;
        tr = sm.GetTariff(1, 1, 1);
        if(tr != NULL)
        {
            auto_ptr<TariffRec> pp(tr);
            smsc_log_info(logger, "operator_id = 1, category_id=1, media_type=1. sn:%s, price:%lf, curr:%s", tr->ServiceNumber.c_str(), tr->Price, tr->Currency.c_str());
        }
        tr = sm.GetTariff(1, 1, 2);
        if(tr != NULL)
        {
            auto_ptr<TariffRec> pp(tr);
            smsc_log_info(logger, "operator_id = 1, category_id=1, media_type=2. sn:%s, price:%lf, curr:%s", tr->ServiceNumber.c_str(), tr->Price, tr->Currency.c_str());
        }

        int o, m, c;
        scanf("%d %d %d", &o, &m, &c);

        sm.ReloadProviderMap();
        sm.ReloadOperatorMap();
        sm.ReloadTariffMatrix();

        smsc_log_info(logger, ">>>>>>Provider test again<<<<<<<<");
        smsc_log_info(logger, "serv_id = %d, prov_id=%d", 1, sm.GetProviderID(1));
        smsc_log_info(logger, "serv_id = %d, prov_id=%d", 3, sm.GetProviderID(3));
        smsc_log_info(logger, "serv_id = %d, prov_id=%d", 6, sm.GetProviderID(6));

        smsc_log_info(logger, ">>>>>>Operator test again<<<<<<<<");
        smsc_log_info(logger, "abonent_id = +79139061234, op_id=%d", sm.GetOperatorID(Address("+79139061234")));
        smsc_log_info(logger, "abonent_id = +79139079876, op_id=%d", sm.GetOperatorID(Address("+79139079876")));
        smsc_log_info(logger, "abonent_id = +79129079876, op_id=%d", sm.GetOperatorID(Address("+79129079876")));
        smsc_log_info(logger, "abonent_id = +71112223344, op_id=%d", sm.GetOperatorID(Address("+71112223344")));
        smsc_log_info(logger, "abonent_id = +79999037097, op_id=%d", sm.GetOperatorID(Address("+79999037097")));

        smsc_log_info(logger, ">>>>>>Tariff test<<<<<<<<");
        tr = sm.GetTariff(1, 1, 1);
        if(tr != NULL)
        {
            auto_ptr<TariffRec> pp(tr);
            smsc_log_info(logger, "operator_id = 1, category_id=1, media_type=1. sn:%s, price:%lf, curr:%s", tr->ServiceNumber.c_str(), tr->Price, tr->Currency.c_str());
        }
        tr = sm.GetTariff(1, 1, 2);
        if(tr != NULL)
        {
            auto_ptr<TariffRec> pp(tr);
            smsc_log_info(logger, "operator_id = 1, category_id=1, media_type=2. sn:%s, price:%lf, curr:%s", tr->ServiceNumber.c_str(), tr->Price, tr->Currency.c_str());
        }
        tr = sm.GetTariff(o, m, c);
        if(tr != NULL)
        {
            auto_ptr<TariffRec> pp(tr);
            smsc_log_info(logger, "operator_id = %d, category_id=%d, media_type=%d. sn:%s, price:%lf, curr:%s", o, c, m, tr->ServiceNumber.c_str(), tr->Price, tr->Currency.c_str());
            printf("operator_id = %d, category_id=%d, media_type=%d. sn:%s, price:%lf, curr:%s", o, c, m, tr->ServiceNumber.c_str(), tr->Price, tr->Currency.c_str());
        }
        else
            printf("No such record. operator_id = %d, category_id=%d, media_type=%d", o, c, m);
        }

    }
    catch (Exception& exc) 
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (std::exception& exc) 
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
