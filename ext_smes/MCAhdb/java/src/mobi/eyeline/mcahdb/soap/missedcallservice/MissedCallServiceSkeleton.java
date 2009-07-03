
/**
 * MissedCallServiceSkeleton.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis2 version: 1.4.1  Built on : Aug 13, 2008 (05:03:35 LKT)
 */
    package mobi.eyeline.mcahdb.soap.missedcallservice;

import org.apache.log4j.Category;

import java.text.SimpleDateFormat;
import java.util.*;

import mobi.eyeline.mcahdb.soap.ServiceContext;
import org.tempuri.*;
import mobi.eyeline.mcahdb.engine.event.ds.Event;

/**
 *  MissedCallServiceSkeleton java skeleton for the axisService
 */
public class MissedCallServiceSkeleton{

  private static final Category log = Category.getInstance(MissedCallServiceSkeleton.class);

  private static final int MESSAGE_STATUS_CALL = 0;
  private static final int MESSAGE_STATUS_ALERT = 1;
  private static final int MESSAGE_STATUS_ALERT_FAILED = 2;
        
         
  /**
   * Auto generated method signature
   *
   * @param getMissedCalls
   */
        
  public GetMissedCallsResponse GetMissedCalls(GetMissedCalls getMissedCalls) {

    long start = 0;
    if (log.isDebugEnabled()) {
      log.debug("Get missed call req: addr=" + getMissedCalls.getPhoneNumber());
      start = System.nanoTime();
    }

    GetMissedCallsResponse resp = new GetMissedCallsResponse();
    try {
      String phoneNumber = getMissedCalls.getPhoneNumber().trim();
      if (phoneNumber.charAt(0) != '+')
        phoneNumber = '+' + phoneNumber;
      SimpleDateFormat df = ServiceContext.getInstance().getDf();
      int fetchInterval = ServiceContext.getInstance().getEventsFetchInterval();

      List<Event> events = new ArrayList<Event>(100);
      ServiceContext.getInstance().getEventsFetcher().getEvents(phoneNumber, new Date(System.currentTimeMillis() - 3600000 * fetchInterval), new Date(), events);
      ArrayOfMissedCall missedCalls = new ArrayOfMissedCall();

      final Map<String, MissedCall> calls = new HashMap<String, MissedCall>(events.size()/2);

      for (Event e : events) {

        String caller = e.getCaller();
        if (caller.charAt(0)=='+')
          caller = caller.substring(1);

        MissedCall call = calls.get(caller);
        if (call == null) {
          call = new MissedCall();
          call.setCallDate(df.format(e.getDate()));
          call.setPhoneNumber(caller);
          call.setMessageStatus(MESSAGE_STATUS_CALL);
          call.setMissedCalls(0);
          calls.put(caller, call);
        }

        switch (e.getType()) {
          case MissedCall:
            call.setMissedCalls(call.getMissedCalls() + 1);
            break;
          case MissedCallAlert:
            if (call.getMissedCalls() == 0)
              call.setMissedCalls(1);
            call.setMessageStatus(MESSAGE_STATUS_ALERT);
            call.setProcessDate(df.format(e.getDate()));
            missedCalls.addMissedCall(call);
            calls.remove(caller);
            break;
          case MissedCallAlertFail:
            if (call.getMissedCalls() == 0)
              call.setMissedCalls(1);
            call.setMessageStatus(MESSAGE_STATUS_ALERT_FAILED);
            call.setProcessDate(df.format(e.getDate()));
            missedCalls.addMissedCall(call);
            calls.remove(caller);
            break;
          default:
            log.warn("Unknown event type " + e.getType());
        }
      }

      for (MissedCall c : calls.values())
        missedCalls.addMissedCall(c);

      resp.setGetMissedCallsResult(missedCalls);

    } catch (Throwable e) {
      log.error("Can't get missed calls", e);
      resp.setGetMissedCallsResult(new ArrayOfMissedCall());
    }

    if (log.isDebugEnabled())
      log.debug("Get missed call getMissedCalls for " + getMissedCalls.getPhoneNumber() + " processed in " + (System.nanoTime() - start) + " nanosec.");

    return resp;
  }
     
}
    