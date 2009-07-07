package mobi.eyeline.mcahdb.engine.journal;

import mobi.eyeline.mcahdb.engine.event.EventStore;
import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.InitException;
import mobi.eyeline.mcahdb.engine.scheduler.Scheduler;
import mobi.eyeline.mcahdb.engine.journal.ds.Journal;
import mobi.eyeline.mcahdb.engine.journal.ds.JournalDataSource;
import mobi.eyeline.mcahdb.engine.journal.ds.JournalEvent;
import mobi.eyeline.mcahdb.engine.journal.ds.impl.file.JournalDataSourceImpl;

import java.util.TreeSet;
import java.util.Comparator;
import java.util.Collection;
import java.util.LinkedList;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class JournalsProcessor {

  private static final Category log = Category.getInstance(JournalsProcessor.class);

  private final JournalDataSource ds;
  private final EventStore eventsStore;
  private final ScheduledExecutorService executor;
  private final Scheduler scheduler;

  public JournalsProcessor(EventStore eventsStore, Scheduler scheduler, Config config) throws InitException {
    try {
      this.ds = new JournalDataSourceImpl(config);
    } catch (DataSourceException e) {
      throw new InitException(e);
    }
    this.eventsStore = eventsStore;
    this.scheduler = scheduler;

    this.executor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "Journals-Processor-Task");
      }
    });
    executor.scheduleAtFixedRate(new Runnable() {
      public void run() {
        loadJournals();
      }
    }, 0, config.getJournalsCheckInterval(), TimeUnit.MILLISECONDS);
  }

  public void shutdown() {
    executor.shutdownNow();
  }

  private void processJournal(Journal j)  {
    long start = 0;
    try {
      if (log.isDebugEnabled()) {
        log.debug("Process journal " + j.getName());
        start = System.nanoTime();
      }

      j.open();

      JournalEvent e;
      Collection<Event> events = new LinkedList<Event>();
      while ((e = j.nextEvent()) != null) {
        switch (e.getType()) {
          case MissedCall:
            if (e.isCalledProfileNotify() && e.isCallerProfileWantNotifyMe())
              scheduler.missedCall(e.getCaller(), e.getCalled(), e.getDate());
            events.add(createEvent(e));
            break;
          case MissedCallAlert:
            if (e.isCalledProfileNotify() && e.isCallerProfileWantNotifyMe())
              scheduler.missedCallAlert(e.getCaller(), e.getCalled());
            events.add(createEvent(e));
            break;
          case MissedCallAlertFail:
            events.add(createEvent(e));
            break;
          case MissedCallRemove:
            events.add(createEvent(e));
            scheduler.missedCallError(e.getCaller(), e.getCalled());
            break;
          case ProfileChaged:
            if (!e.isCallerProfileWantNotifyMe())
              scheduler.profileChanged(e.getCalled());
        }
      }
      eventsStore.addEvents(events);
      if (log.isDebugEnabled())
        log.debug("Journal " + j.getName() + " was sucessfully processed in " + (System.nanoTime() - start) + " nanosec.");

    } catch (DataSourceException e) {
      log.error("Journal " + j.getName() + " process failed.", e);
    } finally {
      j.close();
    }
  }

  private static Event createEvent(JournalEvent e) {
    Event event = new Event();
    switch (e.getType()) {
      case MissedCall: event.setType(Event.Type.MissedCall); break;
      case MissedCallAlert: event.setType(Event.Type.MissedCallAlert); break;
      case MissedCallRemove: event.setType(Event.Type.MissedCallRemove); break;
      default: event.setType(Event.Type.MissedCallAlertFail);
    }
    event.setCaller(e.getCaller());
    event.setCalled(e.getCalled());
    event.setDate(e.getDate());
    event.setCalledProfileNotify(e.isCalledProfileNotify());
    event.setCallerProfileWantNotifyMe(e.isCallerProfileWantNotifyMe());
    return event;
  }

  private void loadJournals() {
    try {
      if (log.isDebugEnabled())
        log.debug("Journal loading started");

      JournalsSortedSet journals = new JournalsSortedSet();
      ds.getJournals(journals);

      for (Journal j : journals) {
        processJournal(j);
        ds.removeJournal(j.getName());
      }

      if (log.isDebugEnabled())
        log.debug("Journal loading finished.");

    } catch (Throwable e) {
      log.error(e,e);
    }
  }

  private static class JournalsSortedSet extends TreeSet<Journal> {
    private JournalsSortedSet() {
      super(new Comparator<Journal>() {
        public int compare(Journal o1, Journal o2) {
          return o1.getStartDate().before(o2.getStartDate()) ? -1 : 1;
        }
      });
    }

    @Override
    public Object clone() {
      throw new UnsupportedOperationException();
    }
  }

  public interface Config {
    public int getJournalsCheckInterval();
    public String getJournalsStoreDir();
    public String getJournalsArchivesDir();
  }
}
