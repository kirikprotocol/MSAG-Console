package mobi.eyeline.mcahdb.engine;

import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.journal.ds.JournalEvent;

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

  private static void writeMissedCall(JournalEvent e, BufferedWriter writer) throws IOException {
    writer.write("A," + df.format(e.getDate()) + ',' + e.getCaller() + ',' + e.getCalled() + ',' + (e.isCalledProfileNotify() ? 1 : 0) + ',' + (e.isCallerProfileWantNotifyMe() ? 1 : 0) + '\n');
  }

  private static void writeMissedCallAlert(JournalEvent e, BufferedWriter writer) throws IOException {
    writer.write("D," + df.format(e.getDate()) + ',' + e.getCaller() + ',' + e.getCalled() + ',' + (e.isCalledProfileNotify() ? 1 : 0) + ',' + (e.isCallerProfileWantNotifyMe() ? 1 : 0) + '\n');
  }

  private static void writeMissedCallAlertFail(JournalEvent e, BufferedWriter writer) throws IOException {
    writer.write("F," + df.format(e.getDate()) + ',' + e.getCaller() + ',' + e.getCalled() + '\n');
  }

  private static void writeJournal(Collection<JournalEvent> events, File storeFile) throws IOException {
    BufferedWriter os = null;
    try {
      os = new BufferedWriter(new FileWriter(storeFile));

      for (JournalEvent e : events) {
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

  public static Collection<JournalEvent> generateJournalEvents(int count, int step) {
    return generateJournalEvents("+7913902", "+7913903", 0, count, step);
  }

  public static Collection<JournalEvent> generateJournalEvents(String prefix1, String prefix2, int start, int count, int step) {
    // Prepare events
    Calendar cal = Calendar.getInstance();
    cal.setTime(new Date());

    Collection<JournalEvent> events = new ArrayList<JournalEvent>(count);
    for (int i=start; i<start + count; i++) {
      JournalEvent e = new JournalEvent();
      e.setCaller(prefix1 + i);
      e.setCalled(prefix2 + i);
      e.setDate(cal.getTime());
      e.setCalledProfileNotify(true);
      e.setCallerProfileWantNotifyMe(true);
      int x = (int)(Math.random() * 100);
      if (x % 3 == 1)
        e.setType(JournalEvent.Type.MissedCall);
      else if (x % 3 == 0)
        e.setType(JournalEvent.Type.MissedCallAlert);
      else
        e.setType(JournalEvent.Type.MissedCallAlertFail);

      events.add(e);

      cal.setTimeInMillis(cal.getTimeInMillis() + step);
    }
    return events;
  }

  public static Collection<Event> generateEvents(int count, int step) {
    // Prepare events
    Calendar cal = Calendar.getInstance();
    cal.setTime(new Date());

    Collection<Event> events = new ArrayList<Event>(count);
    for (int i=1000; i<1000 + count; i++) {
      Event e = new Event();
      e.setCaller("+7913902" + i);
      e.setCalled("+7913903" + i);
      e.setDate(cal.getTime());
      int x = (int)(Math.random() * 100);
      if (x % 3 == 1)
        e.setType(Event.Type.MissedCall);
      else if (x % 3 == 0)
        e.setType(Event.Type.MissedCallAlert);
      else
        e.setType(Event.Type.MissedCallAlertFail);

      events.add(e);

      cal.setTimeInMillis(cal.getTimeInMillis() + step);
    }
    return events;
  }

  public static String generateJournal(String storeDir, String prefix1, String prefix2, int start, int count, int step) throws IOException {
    final String journal = "journal." + startdf.format(new Date()) + ".csv";

    // Create journal
    writeJournal(generateJournalEvents(prefix1, prefix2, start, count, step), new File(storeDir, journal));

    return journal;
  }
}
