package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.event.ds.Event;

import java.util.*;
import java.io.IOException;
import java.io.File;

import com.eyeline.utils.tree.radix.*;
import com.eyeline.utils.IOUtils;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class StoreImpl implements Store {

  private static final Category log = Category.getInstance(StoreImpl.class);

  private static final int INDEX_READ_BUFFER_SIZE = 8192 * 8;

  private UnmodifiableRTree<Collection<Long>> roIndex;
  private RTree<Collection<Long>> rwIndex;
  private File file;
  private DataFile dataFile;
  private File indexFile;

  private Map<String, Collection<Event>> eventsBuffer = new HashMap<String, Collection<Event>>(100);
  private boolean readOnly = false;
  private boolean opened = false;

  public StoreImpl(File dataFile) {
    this.file = dataFile;
  }

  private static StringsRTree createIndex(DataFile dataFile) throws IOException {
    StringsRTree<Collection<Long>> index = new StringsRTree<Collection<Long>>();
    for (Map.Entry<String, Long> e :  dataFile.getEventsListsPositions().entrySet())
      put(index, e.getKey(), e.getValue());
    return index;
  }

  boolean isReadOnly() {
    return readOnly;
  }

  public boolean exists() {
    return file.exists();
  }

  private void openFileForRead() throws IOException, DataSourceException {
    rwIndex = null;

    this.dataFile = new DataFile(file, true);

    if (!dataFile.checkAndRepair()) {
      log.warn("Data file " + dataFile.getFile().getAbsolutePath() + " was crushed and now repaired.");
    } else if (!indexFile.exists()) {
      log.warn("Index file " + indexFile.getAbsolutePath() + " does not exists.");
    } else if (indexFile.exists() && dataFile.checkAndRepair()) {// If index exists and date file ok, load it
      try {
        roIndex = new FileBasedStringsRTree(indexFile.getAbsolutePath(), LongsCollectionSerializer.getInstance(), INDEX_READ_BUFFER_SIZE);
        return;
      } catch (TreeException e) {
        log.warn("Index file " + indexFile.getAbsolutePath() + " is crushed. It will be removed and new index will be created.");
        if (!indexFile.delete())
          throw new DataSourceException("Can't remove index file " + indexFile.getAbsolutePath());
      }
    }

    // Index does not exists, data file cruched or index file cruched    
    roIndex = FileBasedStringsRTree.createRTree(createIndex(this.dataFile), indexFile.getAbsolutePath(), LongsCollectionSerializer.getInstance(), INDEX_READ_BUFFER_SIZE);
  }

  private void openFileForWrite() throws DataSourceException, IOException {
    this.dataFile = new DataFile(file, false);

    if (indexFile.exists()) {// If index exists, remove it
      log.debug("Index file " + indexFile.getAbsolutePath() + " exists but data file opens it edit mode. Index file will be removed.");
      if (!indexFile.delete())
        throw new DataSourceException("Can't remove index file " + indexFile.getAbsolutePath());
    }

    rwIndex = createIndex(this.dataFile);
    roIndex = rwIndex;
  }

  public void open(boolean readOnly) throws DataSourceException {
    try {
      if (this.readOnly && !readOnly)
        close();

      if (!opened) {
        indexFile = new File(dataFile.getFile().getAbsolutePath() + ".ind");
        if (readOnly)
          openFileForRead();
        else
          openFileForWrite();
        opened = true;
      }

      this.readOnly = readOnly;

    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  public void close() throws IOException {
    if (opened) {
      if (readOnly)
        ((FileBasedStringsRTree)roIndex).close();
      else { // store index
        FileBasedStringsRTree index = FileBasedStringsRTree.createRTree(rwIndex, indexFile.getAbsolutePath(), LongsCollectionSerializer.getInstance());
        index.close();
      }
      dataFile.close();
      dataFile = null;
      opened = false;
    }
  }

  public void commit() throws DataSourceException, IOException {
    if (readOnly)
      throw new DataSourceException("File opened in read only mode");

    for (Map.Entry<String, Collection<Event>> e : eventsBuffer.entrySet()) {
      long pos = dataFile.writeEvents(e.getKey(), e.getValue());
      put(rwIndex, e.getKey(), pos);
    }

    dataFile.commit();

    eventsBuffer.clear();
  }

  public void rollback() throws DataSourceException, IOException {
    eventsBuffer.clear();
  }

  public void getEvents(String address, Date from, Date till, Collection<Event> result) throws DataSourceException, IOException {
    Collection<Long> indexes = roIndex.get(address);
    if (indexes == null)
      return;

    for (long index : indexes)
      dataFile.readEvents(address, index, from, till, result);
  }

  public void addEvent(Event event) throws DataSourceException, IOException {
    if (readOnly)
      throw new DataSourceException("File opened in read only mode");

    Collection<Event> list = eventsBuffer.get(event.getCalled());
    if (list == null) {
      list = new LinkedList<Event>();
      eventsBuffer.put(event.getCalled(), list);
    }
    list.add(event);
  }  

  private static void put(RTree<Collection<Long>> ind, String address, long value) {
    Collection<Long> val = ind.get(address);
    if (val == null) {
      val = new LinkedList<Long>();
      ind.put(address, val);
    }
    val.add(value);
  }

  public int hashCode() {
    return dataFile.getFile().getName().hashCode();
  }

  public boolean equals(Object o) {
    if (o instanceof StoreImpl)
      return ((StoreImpl)o).dataFile.getFile().equals(dataFile.getFile());
    return false;
  }



  protected static class LongsCollectionSerializer implements Serializer<Collection<Long>> {

    private static final LongsCollectionSerializer instance = new LongsCollectionSerializer();

    public static LongsCollectionSerializer getInstance() {
      return instance;
    }

    public void serialize(Collection<Long> longs, byte[] bytes, int i) {
      int pos = i;
      IOUtils.writeShort(longs.size(), bytes, pos);
      pos += 2;
      for (long l : longs) {
        IOUtils.writeLong(l, bytes, pos);
        pos += 8;
      }
    }

    public Collection<Long> deserialize(byte[] bytes, int i) {
      int pos = i;
      int size = IOUtils.readShort(bytes, pos);
      if (size == 0)
        return Collections.emptyList();

      Collection<Long> result = new ArrayList<Long>(size);
      pos += 2;
      for (int j = 0; j<size; j++) {
        result.add(IOUtils.readLong(bytes, pos));
        pos += 8;
      }
      return result;
    }

    public int getSerializationSize(Collection<Long> longs) {
      return 2 + longs.size() * 8;
    }
  }
}
