package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;

import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.TestUtils;
import com.eyeline.utils.tree.radix.FileBasedStringsRTree;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class StoreImplTest {

  @Test
  public void testAdd() throws DataSourceException, IOException {
    StoreImpl store = new StoreImpl(new File("store/events/2008010101.dat"));
    store.open(false);
    Collection<Event> events = TestUtils.generateEvents(2, 1);
    for (Event e : events)
      store.addEvent(e);
    store.commit();
    store.close();
  }

  @Test
  public void testRead() throws DataSourceException, IOException {
    StoreImpl store = new StoreImpl(new File("store/events/2008010101.dat"));
    store.open(true);
    Collection<Event> events = new LinkedList<Event>();
    store.getEvents("+79139031000", new Date(System.currentTimeMillis() - 3600000), new Date(), events);
    for (Event e : events) {
      System.out.println(e.getType() + " " + e.getCaller() + " " + e.getCalled());
    }

    store.close();
  }

//  @Test
  public void printIndex() {
    FileBasedStringsRTree tree = new FileBasedStringsRTree("store/events/2008010101.dat.ind", StoreImpl.LongsCollectionSerializer.getInstance());
    tree.printTree();
  }
}
