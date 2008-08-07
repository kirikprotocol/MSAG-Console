package mobi.eyeline.mcahdb.engine.ds.impl.file;

import mobi.eyeline.mcahdb.engine.ds.Event;
import mobi.eyeline.mcahdb.engine.ds.impl.EventImpl;

import java.text.SimpleDateFormat;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.File;
import java.io.FileWriter;
import java.util.Collection;
import java.util.Calendar;
import java.util.Date;
import java.util.ArrayList;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class TestUtils {

  private static final SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd HH:mm:ss.SSS");
    private static final SimpleDateFormat startdf = new SimpleDateFormat("yyyyMMddHHmmss");

  private TestUtils() {
  }

  private static void writeMissedCall(Event e, BufferedWriter writer) throws IOException {
    writer.write("A," + df.format(e.getDate()) + ',' + e.getCaller() + ',' + e.getCalled() + ',' + (e.isCalledProfileNotify() ? 1 : 0) + ',' + (e.isCallerProfileWantNotifyMe() ? 1 : 0) + '\n');
  }

  private static void writeMissedCallAlert(Event e, BufferedWriter writer) throws IOException {
    writer.write("D," + df.format(e.getDate()) + ',' + e.getCaller() + ',' + e.getCalled() + ',' + (e.isCalledProfileNotify() ? 1 : 0) + ',' + (e.isCallerProfileWantNotifyMe() ? 1 : 0) + '\n');
  }

  private static void writeMissedCallAlertFail(Event e, BufferedWriter writer) throws IOException {
    writer.write("F," + df.format(e.getDate()) + ',' + e.getCaller() + ',' + e.getCalled() + '\n');
  }

  private static void writeJournal(Collection<Event> events, File storeFile) throws IOException {
    BufferedWriter os = null;
    try {
      os = new BufferedWriter(new FileWriter(storeFile));

      for (Event e : events) {
        switch (e.getType()) {
          case MissedCall: writeMissedCall(e, os); break;
          case MissedCallAlert: writeMissedCallAlert(e, os); break;
          case MissedCallAlertFail: writeMissedCallAlertFail(e, os); break;
          default:
        }
      }

    } finally {
      if (os != null)
        try {
          os.close();
        } catch (IOException e) {
        }
    }
  }

  public static Collection<Event> generateEvents(int count, int step) {
    // Prepare events
    Calendar cal = Calendar.getInstance();
    cal.setTime(new Date());

    Collection<Event> events = new ArrayList<Event>(count);
    for (int i=1000; i<1000 + count; i++) {
      EventImpl e = new EventImpl();
      e.setCaller("+7913902" + i);
      e.setCalled("+7913903" + i);
      e.setDate(cal.getTime());
      if (i % 3 == 1)
        e.setType(Event.Type.MissedCall);
      else if (i % 3 == 0)
        e.setType(Event.Type.MissedCallAlert);
      else
        e.setType(Event.Type.MissedCallAlertFail);

      events.add(e);

      cal.setTimeInMillis(cal.getTimeInMillis() + step);
    }
    return events;
  }

  public static String generateJournal(int count, int step) throws IOException {
    final String journal = "journal." + startdf.format(new Date()) + ".csv";

    // Create journal
    writeJournal(generateEvents(count, step), new File("store/journals/" + journal));

    return journal;
  }
}
