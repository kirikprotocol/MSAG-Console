package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import mobi.eyeline.mcahdb.engine.event.ds.EventsDataSource;
import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.event.ds.impl.file.Store;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.event.ds.impl.file.StoresCache;
import mobi.eyeline.mcahdb.engine.event.EventStore;

import java.util.*;
import java.io.File;
import java.io.IOException;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class EventsDataSourceImpl implements EventsDataSource {

  private StoresCache cache;

  public EventsDataSourceImpl(EventStore.Config config) throws DataSourceException {
    cache = new StoresCache(new File(config.getEventsStoreDir()), config.getEventsStoreRWTimeout(), config.getEventsStoreROTimeout());
  }

  public void getEvents(String address, Date from, Date till, Collection<Event> result) throws DataSourceException {
    Collection<Store> files = cache.listFiles(from, till);

    for (Store f : files) {
      try {
        f.open(true);
        f.getEvents(address, from, till, result);
      } catch (IOException e) {
        throw new DataSourceException(e);
      } finally {
        try {
          f.close();
        } catch (IOException e) {
        }
      }
    }
  }

  public void addEvents(Collection<Event> events) throws DataSourceException {
    Set<Store> openedFiles = new HashSet<Store>(2);

    try {
      for (Event e : events) {
        Store s = cache.getFile(e.getDate());
        if (!openedFiles.contains(s)) {
          openedFiles.add(s);
          s.open(false);
        }
        s.addEvent(e);
      }

      // todo transaction problems
      for (Store s : openedFiles) {
        try {
          s.commit();
        } catch (IOException e) {
        }
      }

    } catch (Exception e) {      
      for (Store s : openedFiles) {
        try {
          s.rollback();
        } catch (IOException e1) {
        }
      }
      throw new DataSourceException(e);
    } finally {
      for (Store s : openedFiles) {
        try {
          s.close();
        } catch (IOException e) {
        }
      }
    }
  }

  public void close() {
    cache.shutdown();
  }
}
