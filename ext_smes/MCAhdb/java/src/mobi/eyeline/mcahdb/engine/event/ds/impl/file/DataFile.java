package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.io.*;
import java.util.*;

import com.eyeline.utils.IOUtils;
import com.eyeline.utils.FileUtils;

/**
 * User: artem
 * Date: 31.07.2008
 */

class DataFile {

  private static final byte EOR = (byte)255;

  private static final int MSISDN_LEN = 15;
  private static final int EVENT_LEN = 1/*type*/ + 8/*date*/ + MSISDN_LEN/*caller*/ + 1/*CalledProfileNotify*/ + 1/*CallerProfileWantNotifyMe*/;

  private final RandomAccessFile f;
  private final RandomAccessFileReader reader;
  private final File dataFileFile;

  public DataFile(File dataFile, boolean readOnly) throws IOException {
    this.f = new RandomAccessFile(dataFile, readOnly ? "r" : "rw");
    this.reader = new RandomAccessFileReader(f);
    this.dataFileFile = dataFile;        
  }

  public boolean checkAndRepair() throws IOException {
    return !FileUtils.truncateFile(f, EOR, EVENT_LEN);
//    long pos = f.length();
//    if (pos == 0)
//      return true;
//
//    f.seek(pos - 1);
//    byte eor = (byte) f.read();
//
//    if (eor != EOR) {
//      FileChannel fc = null;
//      try {
//        fc = f.getChannel();
//        fc.truncate(findLastEOR());
//      } finally {
//        if (fc != null)
//          fc.close();
//      }
//      return false;
//    }
//    return true;
  }

//  private long findLastEOR() throws IOException {
//    long pos = f.length();
//    byte[] bytes = new byte[EVENT_LEN];
//    while (pos - EVENT_LEN > 0) {
//      pos -= EVENT_LEN;
//      f.seek(pos);
//      f.readFully(bytes);
//      for (int i = 0; i< EVENT_LEN; i++) {
//        if (bytes[i] == EOR)
//          return pos + i;
//      }
//    }
//    return 0;
//  }

  public void close() throws IOException {
    f.close();
  }

  public File getFile() {
    return dataFileFile;
  }

  public boolean rename(File toFile) {
    return dataFileFile.renameTo(toFile);
  }

  public void commit() throws IOException {
    f.seek(f.length());
    f.write(EOR);
    reader.clean();
  }

  public long writeEvents(String address, Collection<Event> events) throws DataSourceException, IOException {
    // Sort events
    MCAEventsSet eventsSet = new MCAEventsSet();
    eventsSet.addAll(events);

    byte[] bytes = new byte[MSISDN_LEN + 2/*number of events*/ + eventsSet.size() * EVENT_LEN];
    int pos = 0;

    // Write called
    IOUtils.writeString(address, bytes, pos, MSISDN_LEN);
    pos += MSISDN_LEN;

    // Write events size
    IOUtils.writeShort(eventsSet.size(), bytes, pos);
    pos += 2;

    // Write events
    for (Event e : eventsSet) {
      writeEvent(e, bytes, pos);
      pos += EVENT_LEN;
    }

    long startPos = f.length();
    f.seek(startPos);
    f.write(bytes);
    reader.clean();
    return startPos;
  }

  public Collection<Event> readEvents(String address, long startPos, Date from, Date till) throws IOException, DataSourceException {
    reader.seek(startPos);
    byte[] bytes = new byte[MSISDN_LEN + 2/*number of events*/];
    reader.readFully(bytes);

    String msisdn = IOUtils.readString(bytes, 0, MSISDN_LEN);
    if (!msisdn.equals(address))
      throw new DataSourceException("Index is broken (no list found at " + startPos + ").");

    int eventsSize = IOUtils.readShort(bytes, MSISDN_LEN);
    if (eventsSize == 0)
      return Collections.emptyList();

    bytes = new byte[eventsSize * EVENT_LEN];
    reader.readFully(bytes);

    Collection<Event> result = new ArrayList<Event>(eventsSize);
    int pos = 0;
    for (int i=0; i < eventsSize; i++) {
      Event e = readEvent(bytes, pos);
      e.setCalled(address);
      if (e.getDate().after(from) && e.getDate().before(till))
        result.add(e);
      else
        break;
      pos += EVENT_LEN;
    }
    return result;
  }

  public Map<String, Long> getEventsListsPositions() throws IOException {
    Map<String, Long> result = new HashMap<String, Long>(100);
    byte[] bytes = new byte[MSISDN_LEN + 2];

    try {
      long pos = 0;
      while(true) {
        reader.seek(pos);
        reader.readFully(bytes);
        String msisdn = IOUtils.readString(bytes, 0, MSISDN_LEN);
        int eventsSize = IOUtils.readShort(bytes, MSISDN_LEN);
        result.put(msisdn, pos);
        pos += MSISDN_LEN + 2 + eventsSize * EVENT_LEN;
      }
    } catch (EOFException e) {
    }
    return result;
  }

  private static void writeEvent(Event e, byte[] bytes, int offset) throws DataSourceException {
    int pos = offset;
    // Wtite type
    switch (e.getType()) {
      case MissedCall:          bytes[pos] = 0; break;
      case MissedCallAlert:     bytes[pos] = 1; break;
      case MissedCallAlertFail: bytes[pos] = 2; break;
      default:
        throw new DataSourceException("Unknown event type: " + e.getType());
    }
    pos += 1;

    // Write date
    IOUtils.writeLong(e.getDate().getTime(), bytes, pos);
    pos += 8;

    // Write caller
    IOUtils.writeString(e.getCaller(), bytes, pos, MSISDN_LEN);
    pos += MSISDN_LEN;

    // Write CalledProfileNotify
    bytes[pos] = (byte)(e.isCalledProfileNotify() ? 1 : 0);

    // Write CallerProfileWantNotifyMe
    bytes[pos +1] = (byte)(e.isCallerProfileWantNotifyMe() ? 1 : 0);
  }

  private static Event readEvent(byte[] bytes, int offset) throws DataSourceException {
    Event e = new Event();
    int pos = offset;
    // Read type
    int type = bytes[pos];
    switch (type) {
      case 0 : e.setType(Event.Type.MissedCall); break;
      case 1 : e.setType(Event.Type.MissedCallAlert); break;
      case 2 : e.setType(Event.Type.MissedCallAlertFail); break;
      default:
        throw new DataSourceException("Unknown event type: " + e.getType());
    }
    pos+= 1;

    // Read date
    e.setDate(new Date(IOUtils.readLong(bytes, pos)));
    pos += 8;

    // Read caller
    e.setCaller(IOUtils.readString(bytes, pos, MSISDN_LEN));
    pos += MSISDN_LEN;

    // Read CalledProfileNotify
    e.setCalledProfileNotify(bytes[pos] == 1);

    // Read CallerProfileWantNotifyMe
    e.setCallerProfileWantNotifyMe(bytes[pos + 1] == 1);

    return e;
  }




  private static class MCAEventsSet extends TreeSet<Event> {
    private MCAEventsSet() {
      super(new Comparator<Event>() {
        public int compare(Event o1, Event o2) {
          return (o1.getDate().before(o2.getDate())) ? -1 : 1;
        }
      });
    }
  }
}
