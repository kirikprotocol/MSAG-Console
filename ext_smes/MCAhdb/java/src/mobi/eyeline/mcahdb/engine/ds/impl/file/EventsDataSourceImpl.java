package mobi.eyeline.mcahdb.engine.ds.impl.file;

import mobi.eyeline.mcahdb.engine.ds.EventsDataSource;
import mobi.eyeline.mcahdb.engine.ds.Event;
import mobi.eyeline.mcahdb.engine.ds.DataSourceException;
import mobi.eyeline.mcahdb.engine.ds.impl.file.store.StoresCache;
import mobi.eyeline.mcahdb.engine.ds.impl.file.store.Store;
import mobi.eyeline.mcahdb.GlobalConfig;

import java.util.*;
import java.io.File;
import java.io.IOException;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class EventsDataSourceImpl implements EventsDataSource {

  private StoresCache cache;

  public EventsDataSourceImpl(Config config) {
    cache = new StoresCache(new File(config.getEventsStoreDir()));
  }


  public Collection<Event> getEvents(String address, Date from, Date till) throws DataSourceException {
    Collection<Store> files = cache.getFilesForRead(from, till);
    Collection<Event> result= new LinkedList<Event>();

    for (Store f : files) {
      try {
        f.open();
        result.addAll(f.getEvents(address, from, till));
      } catch (IOException e) {
        throw new DataSourceException(e);
      } finally {
        try {
          f.close();
        } catch (IOException e) {
        }
      }
    }

    return result;
  }

  public void addEvents(Collection<Event> events) throws DataSourceException {
    Set<Store> openedFiles = new HashSet<Store>(2);

    try {
      for (Event e : events) {
        Store s = cache.getFileForWrite(e.getDate());
        if (!openedFiles.contains(s)) {
          s.open();
          openedFiles.add(s);
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


  public interface Config {
    public String getEventsStoreDir();
  }
}
