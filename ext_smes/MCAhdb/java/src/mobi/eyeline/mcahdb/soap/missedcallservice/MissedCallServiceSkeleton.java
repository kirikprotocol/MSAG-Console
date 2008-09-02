package mobi.eyeline.mcahdb.soap.missedcallservice;

import mobi.eyeline.mcahdb.soap.ServiceContext;
import mobi.eyeline.mcahdb.engine.event.ds.Event;

import java.util.*;
import java.text.SimpleDateFormat;

import org.apache.log4j.Category;

/**
 *  MissedCallServiceSkeleton java skeleton for the axisService
 */
public class MissedCallServiceSkeleton {

  private static final Category log = Category.getInstance(MissedCallServiceSkeleton.class);

  private static final int MESSAGE_STATUS_CALL = 0;
  private static final int MESSAGE_STATUS_ALERT = 1;
  private static final int MESSAGE_STATUS_ALERT_FAILED = 2;

  public GetMissedCallsResponse GetMissedCalls(GetMissedCalls req) {
    long start = 0;
    if (log.isDebugEnabled()) {
      log.debug("Get missed call req: addr=" + req.getPhoneNumber());
      start = System.nanoTime();
    }

    GetMissedCallsResponse resp = new GetMissedCallsResponse();

    try {
      SimpleDateFormat df = ServiceContext.getInstance().getDf();
      int fetchInterval = ServiceContext.getInstance().getEventsFetchInterval();

      LinkedList<Event> events = new LinkedList<Event>();
      ServiceContext.getInstance().getEventsFetcher().getEvents(req.getPhoneNumber(), new Date(System.currentTimeMillis() - 3600000 * fetchInterval), new Date(), events);
      ArrayOfMissedCall missedCalls = new ArrayOfMissedCall();

      final Map<String, MissedCall> calls = new HashMap<String, MissedCall>(events.size()/2);

      for (Event e : events) {

        MissedCall call = calls.get(e.getCaller());
        if (call == null) {
          call = new MissedCall();
          call.setCallDate(df.format(e.getDate()));
          call.setPhoneNumber(e.getCaller());
          call.setMessageStatus(MESSAGE_STATUS_CALL);
          call.setMissedCalls(0);
          calls.put(e.getCaller(), call);
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
            calls.remove(e.getCaller());
            break;
          case MissedCallAlertFail:
            if (call.getMissedCalls() == 0)
              call.setMissedCalls(1);
            call.setMessageStatus(MESSAGE_STATUS_ALERT_FAILED);
            call.setProcessDate(df.format(e.getDate()));
            missedCalls.addMissedCall(call);
            calls.remove(e.getCaller());
            break;
          default:
            log.warn("Unknown event type " + e.getType());
        }
      }

      for (MissedCall c : calls.values())
        missedCalls.addMissedCall(c);

      resp.setGetMissedCallsResult(missedCalls);

    } catch (Throwable e) {
      resp.setGetMissedCallsResult(new ArrayOfMissedCall());
    }

    if (log.isDebugEnabled())
      log.debug("Get missed call req for " + req.getPhoneNumber() + " processed in " + (System.nanoTime() - start) + " nanosec.");

    return resp;
  }
}
    