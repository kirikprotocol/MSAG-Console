package mobi.eyeline.mcahdb.engine;

import mobi.eyeline.mcahdb.engine.ds.*;
import mobi.eyeline.mcahdb.GlobalConfig;

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
  private final EventsDataSource eventsDS;
  private final ScheduledExecutorService executor;


  public JournalsProcessor(JournalDataSource ds, EventsDataSource eventsDS, Config config) {
    this.ds = ds;
    this.eventsDS = eventsDS;
    this.executor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "Journals-Processor-Task");
      }
    });
    executor.scheduleWithFixedDelay(new ProcessJournalTask(), 0, config.getJournalsCheckInterval(), TimeUnit.MILLISECONDS);
  }

  public void shutdown() {
    executor.shutdownNow();
  }

  private class ProcessJournalTask implements Runnable {

    private void processJournal(Journal j)  {
      long start = 0;
      try {
        if (log.isDebugEnabled()) {
          log.debug("Process journal " + j.getName());
          start = System.nanoTime();
        }

        j.open();

        Event e;
        Collection<Event> events = new LinkedList<Event>();
        while ((e = j.nextEvent()) != null)
          events.add(e);

        eventsDS.addEvents(events);

        if (log.isDebugEnabled())
          log.debug("Journal " + j.getName() + " was sucessfully processed in " + (System.nanoTime() - start) + " nanosec.");

      } catch (DataSourceException e) {
        log.error("Journal " + j.getName() + " process failed.", e);
      } finally {
        j.close();
      }
    }

    public void run() {
      try {
        if (log.isDebugEnabled())
          log.debug("Journal loading started");

        JournalsSortedSet journals = new JournalsSortedSet();
        journals.addAll(ds.getJournals());

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
  }
}
