#include <memory>
#include "scag/pvss/common/ScopeType.h"
#include "logger/Logger.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/data/Property.h"

using namespace scag2::pvss;
using scag2::pvss::ScopeType;

int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* log_ = smsc::logger::Logger::getInstance("bck.proc");

    ScopeType scope_ = SCOPE_ABONENT;
    const std::string lines[] = {
            "I 27-05-2009 03:15:02,579 024  pvss.abnt: A key=.1.1.79156062209 property=\"lovemob.inviter.gift.79882600745\" INT: 255 TIME_POLICY: R_ACCESS FINAL_DATE: 2009/07/28 06:31:01 LIFE_TIME: 7776000",
            // std::string line = "I 26-07-2009 00:04:41,923 039  pvss.serv: A key=184 property=\"4205\" STRING: \"http://m1.eyeline.mobi/a79188049413/wap.watago.mobi/sd2/4205/mg/wap/loc/\" TIME_POLICY: ACCESS FINAL_DATE: 2009/07/25 17:34:41 LIFE_TIME: 1800";
            "I 27-05-2009 03:41:53,411 023  pvss.abnt: A key=.1.1.79154541248 property=\"SBT-krasnodar-chargeable-resource\" STRING: \"<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
            "<r><ps><p n=\"sid\" v=\"87\"/><p n=\"category\" v=\"129\"/><p n=\"usr\" v=\"2\"/><p n=\"rid\" v=\"5\"/><p n=\"id\" v=\"1117\"/><p n=\"abonent\" v=\"79154541248\"/></ps><prs><pr n=\"pub\" v=\"guide\"/><pr n=\"qs\" v=\"wppost\"/></prs></r>\" TIME_POLICY: ACCESS FINAL_DATE: 2009/07/25 17:34:41 LIFE_TIME: 1800",
            ""
    };

    std::string prevline;
    for ( const std::string* lineptr = lines; ! lineptr->empty(); ++lineptr ) {
        
        std::string line = *lineptr;
        if ( line.empty() ) break;

        {
            {
                {
                    // processing one line

                    char logLevel, act;
                    unsigned day, month, year, hour, minute, second, msec, thread;
                    int ikey;
                    char cat[20], skey[50];
                    int propstart;
                    
                    bool scanfok = false;
                    do {

                        int sc;
                        if ( scope_ == SCOPE_ABONENT ) {
                            sc = sscanf(line.c_str(), 
                                        "%c %02u-%02u-%04u %02u:%02u:%02u,%03u %u %s %c key=%s property=%n",
                                        &logLevel,&day,&month,&year,&hour,&minute,&second,&msec,&thread,
                                        cat, &act, skey, &propstart );
                        } else {
                            sc = sscanf(line.c_str(), 
                                        "%c %02u-%02u-%04u %02u:%02u:%02u,%03u %u %s %c key=%u property=%n",
                                        &logLevel,&day,&month,&year,&hour,&minute,&second,&msec,&thread,
                                        cat, &act, &ikey, &propstart );
                        }
                        if ( sc < 12 ) {
                            smsc_log_warn(log_,"cannot sscanf(%s): %d",line.c_str(),sc);
                            if ( ! prevline.empty() ) {
                                // trying to combine with prevline
                                prevline.reserve(prevline.size() + line.size() + 2);
                                prevline.push_back('\n');
                                prevline.append(line);
                                line.swap(prevline);
                                prevline.clear();
                                continue;
                            }
                        } else {
                            scanfok = true;
                            if ( ! prevline.empty() ) {
                                smsc_log_warn(log_,"scanf ok while prevline=%s", prevline.c_str());
                                prevline.clear();
                            }
                        }

                        break;
                    } while ( true );
                    if ( ! scanfok ) { continue; }
                        
                    ProfileKey profileKey;
                    switch (scope_) {
                    case (SCOPE_ABONENT) : profileKey.setAbonentKey(skey); break;
                    case (SCOPE_SERVICE) : profileKey.setServiceKey(ikey); break;
                    case (SCOPE_OPERATOR) : profileKey.setOperatorKey(ikey); break;
                    case (SCOPE_PROVIDER) : profileKey.setProviderKey(ikey); break;
                    default: throw smsc::util::Exception("cannot set profile key of unknown scope %u", scope_);
                    }
                        
                    Property property;
                    try {
                        property.fromString( line.c_str() + propstart );
                    } catch (...) {
                        smsc_log_warn(log_,"cannot parse property: %s", line.c_str() + propstart );
                        prevline = line;
                        continue;
                    }

                    smsc_log_info(log_,"key=%s property=%s",
                                  profileKey.toString().c_str(),
                                  property.toString().c_str() );

                    /*
                    SetCommand* cmd = new SetCommand();
                    cmd->setProperty(property);
                    ProfileRequest request( profileKey, cmd );

                    // passing an entry to a pvss dispatcher
                    core::server::Server::SyncLogic* logic = 
                        processor_.dispatcher_->getSyncLogic(processor_.dispatcher_->getIndex(request));
                    if ( !logic ) {
                        smsc_log_warn(log_,"cannot obtain sync logic on key=%s",profileKey.toString().c_str());
                        continue;
                    }
                    delete logic->process(request);

                    if ( (++processed % 100) == 0 ) {
                        const util::msectime_type now = util::currentTimeMillis();
                        const util::msectime_type elapsedTime = now - startTime;
                        const unsigned expected = unsigned(elapsedTime*processor_.propertiesPerSec_/1000);
                        if ( processed > expected ) {
                            // we have to wait
                            int waitTime = int((processed - expected) * 1000ULL / processor_.propertiesPerSec_);
                            if ( waitTime > 10 ) {
                                MutexGuard mg(mon_);
                                mon_.wait(waitTime);
                            }
                        }
                    }
                     */

                    // end of processing
                }
            }
        }

    }
    return 0;
}
