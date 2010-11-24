#include "informer/dcp/client/InformerClient.hpp"
#include "util/Timer.hpp"

bool interrupted=false;

extern "C" void sigint(int sig)
{
  interrupted=true;
}

int main( int argc, char** argv )
{
  sigset(SIGINT,sigint);
  int port = 9073;
  if (argc>1) {
    port = atoi(argv[1]);
  }

  using namespace eyeline::informer::dcp::client;
  using namespace eyeline::informer::dcp;
  smsc::logger::Logger::Init();
  InformerClient ic;
  try{
    ic.Init("localhost",port);
    while(!ic.isConnected())
    {
      printf("not connected\n");
      sleep(1);
    }
    ic.userAuth("bukind","pwd");
    printf("auth ok\n");
    messages::DeliveryInfo di;
    di.setName("test1 по русски");
    di.setPriority(1);
    di.setTransactionMode(false);
    di.setStartDate("15.11.2010 10:00:00");
    //di.setEndDate("15.12.2010 10:00:00");
    di.setActivePeriodStart("02:00:00");
    di.setActivePeriodEnd("20:00:00");
    di.getActiveWeekDaysRef().push_back("Mon");
    di.getActiveWeekDaysRef().push_back("Tue");
    di.getActiveWeekDaysRef().push_back("Wed");
    di.getActiveWeekDaysRef().push_back("Thu");
    di.getActiveWeekDaysRef().push_back("Fri");
    di.setFlash(false);
    di.setUseDataSm(false);
    di.setDeliveryMode(messages::DeliveryMode::SMS);
    di.setOwner("bukind");
    di.setRetryOnFail("false");
    di.setReplaceMessage(false);
    di.setSvcType("TEST");
    di.setUserData("test1");
    di.setSourceAddress("123321");
    di.setFinalDlvRecords(true);
    di.setFinalMsgRecords(true);
    int did=ic.createDelivery(di);
    printf("delivery created, did=%d\n",did);
    try{
      ic.modifyDelivery(did,di);
    }catch(std::exception& e)
    {
      printf("ic.modifyDelivery exception:%s\n",e.what());
    }

//    TIMETHIS("getdlvinfo",1000)
//    {
//      messages::DeliveryInfo di3=ic.getDeliveryInfo(did);
//    }

    messages::DeliveryInfo di2=ic.getDeliveryInfo(did);
    printf("deliveryInfo=%s\n",di2.toString().c_str());
    messages::GetDeliveryStateResp ds=ic.getDeliveryState(did);
    printf("deliveryState=%s\n",ds.toString().c_str());
    ic.changeDeliveryState(did,ds.getState());
    messages::GetDeliveriesList gdlReq;
    messages::DeliveriesFilter filter;
    gdlReq.setFilter(filter);
    gdlReq.getResultFieldsRef().push_back(messages::DeliveryFields::ActivityPeriod);
    gdlReq.getResultFieldsRef().push_back(messages::DeliveryFields::EndDate);
    gdlReq.getResultFieldsRef().push_back(messages::DeliveryFields::Name);
    gdlReq.getResultFieldsRef().push_back(messages::DeliveryFields::StartDate);
    gdlReq.getResultFieldsRef().push_back(messages::DeliveryFields::Status);
    gdlReq.getResultFieldsRef().push_back(messages::DeliveryFields::UserData);
    gdlReq.getResultFieldsRef().push_back(messages::DeliveryFields::UserId);
    int gdlReqId=ic.getDeliveriesList(gdlReq);
    printf("gdlReqId=%d\n",gdlReqId);
    std::vector<messages::DeliveryListInfo> gdlResult;
    bool mms=true;
    while(mms)
    {
      mms=ic.getDeliveriesListNext(gdlReqId,100,gdlResult);
      for(std::vector<messages::DeliveryListInfo>::iterator it=gdlResult.begin(),end=gdlResult.end();it!=end;++it)
      {
        printf("gdlResult=%s\n",it->toString().c_str());
      }
      gdlResult.clear();
    }
    messages::CountDeliveries cdReq;
    cdReq.setFilter(filter);
    printf("deliveries count=%d\n",ic.countDeliveries(cdReq));
    std::vector<messages::DeliveryHistoryItem> gdhResult;
    ic.getDeliveryHistory(did,gdhResult);
    for(std::vector<messages::DeliveryHistoryItem>::iterator it=gdhResult.begin(),end=gdhResult.end();it!=end;++it)
    {
      printf("history=%s\n",it->toString().c_str());
    }
    std::vector<int64_t> allMsgids;
    time_t tstart=time(0);
    for(int i=0;i<10;i++)
    {
      std::vector<messages::DeliveryMessage> msgs;
      messages::DeliveryMessage msg1;
      for(int j=0;j<1000;j++)
      {
        char abnt[32];
        sprintf(abnt,"+79130%02d%04d",i,j);
        msg1.setAbonent(abnt);
        msg1.setMsgType(messages::MessageType::TextMessage);
        char txt[32];
        sprintf(txt,"this is message text:%dx%d",i,j);
        msg1.setText(txt);
        char ud[32];
        sprintf(ud,"ud%d:%d",i,j);
        msg1.setUserData(ud);
        msgs.push_back(msg1);
      }
      std::vector<int64_t> msgids;
      ic.addDeliveryMessages(did,msgs,msgids);
      allMsgids.insert(allMsgids.end(),msgids.begin(),msgids.end());
    }
    time_t tend=time(0);
    printf("upload time=%llu\n",tend-tstart);
    //for(std::vector<int64_t>::iterator it=msgids.begin(),end=msgids.end();it!=end;++it)
    //{
    //printf("msgid=%lld\n",*it);
    //}
    messages::DeliveryGlossary dg=ic.getDeliveryGlossary(did);
    printf("glossary=%s\n",dg.toString().c_str());
    ic.modifyDeliveryGlossary(did,dg);

    messages::RequestMessagesState rmsReq;
    tm nowTime;
    time_t now=time(0);
    gmtime_r(&now,&nowTime);
    char dateBuf1[32];
    char dateBuf2[32];
    sprintf(dateBuf1,"%02d.%02d.%02d 02:00:00",nowTime.tm_mday,nowTime.tm_mon+1,nowTime.tm_year+1900);
    sprintf(dateBuf2,"%02d.%02d.%02d 22:00:00",nowTime.tm_mday,nowTime.tm_mon+1,nowTime.tm_year+1900);
    rmsReq.setStartDate(dateBuf1);
    rmsReq.setEndDate(dateBuf2);
    rmsReq.getStatesRef().push_back(messages::DeliveryMessageState::New);
    //rmsReq.getMsisdnFilterRef().push_back("+79130000001");
    //rmsReq.getMsisdnFilterRef().push_back("+79130000002");
    std::vector<messages::ReqField> flds;
    flds.push_back(messages::ReqField::Abonent);
    flds.push_back(messages::ReqField::Date);
    flds.push_back(messages::ReqField::ErrorCode);
    flds.push_back(messages::ReqField::State);
    flds.push_back(messages::ReqField::Text);
    flds.push_back(messages::ReqField::UserData);
    rmsReq.setFields(flds);
    rmsReq.setDeliveryId(did);

    int rmsReqId=ic.requestMessagesState(rmsReq);
    std::vector<messages::MessageInfo> rmsResult;
    mms=true;
    while(mms)
    {
      mms=ic.getNextMessagesPack(rmsReqId,100,rmsResult);
      for(std::vector<messages::MessageInfo>::iterator it=rmsResult.begin(),end=rmsResult.end();it!=end;++it)
      {
        printf("rmsResult=%s\n",it->toString().c_str());
      }
      rmsResult.empty();
    }
    messages::CountMessages cm;
    cm.setStartDate(dateBuf1);
    cm.setEndDate(dateBuf2);
    cm.setDeliveryId(did);
    cm.getStatesRef().push_back(messages::DeliveryMessageState::Delivered);
    cm.getStatesRef().push_back(messages::DeliveryMessageState::Failed);
    printf("messages count=%d\n",ic.countMessages(cm));

    allMsgids.erase(allMsgids.begin()+allMsgids.size()/2,allMsgids.end());
    ic.dropDeliveryMessages(did,allMsgids);

    //ic.dropDelivery(did);

    messages::DeliveryState nds;
    nds.setStatus(messages::DeliveryStatus::Active);
    printf("changing delivery state to %s\n",nds.toString().c_str());
    ic.changeDeliveryState(did,nds);
    printf("waiting\n");
    while(!interrupted)
    {
      sleep(1);
      messages::GetDeliveryStateResp ds2=ic.getDeliveryState(did);
      printf("new delivery state:%s\n",ds2.toString().c_str());
      printf("messages count=%d\n",ic.countMessages(cm));
    }


  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
  ic.Stop();
  return 0;
}
