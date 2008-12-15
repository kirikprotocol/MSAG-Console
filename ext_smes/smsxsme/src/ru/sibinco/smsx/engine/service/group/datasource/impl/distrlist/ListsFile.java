package ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist;

import ru.sibinco.smsx.engine.service.group.datasource.impl.DataFile;

import java.io.File;
import java.io.IOException;

import com.eyeline.utils.IOUtils;

/**
 * User: artem
 * Date: 19.11.2008
 */
class ListsFile extends DataFile {

  private static final int LIST_NAME_LEN = 31;
  private static final int RECORD_LEN = LIST_NAME_LEN + 4/*max elements (submitters/members)*/ + 1/*system*/ + ADDRESS_LEN;

  protected ListsFile(File file) throws IOException {
    super(file, "SMSCDLLST", 0x010000, RECORD_LEN);
  }

  public void list(final Visitor visitor) throws IOException {
    list(new RecordVisitor() {
      public void record(byte[] bytes, long offset) {
        String name = IOUtils.readString(bytes, 0, LIST_NAME_LEN);
        int maxElements = IOUtils.readInt(bytes, LIST_NAME_LEN);
        String owner = null;
        if (bytes[LIST_NAME_LEN + 4] == 0)
          owner = readMsisdn(bytes, LIST_NAME_LEN + 4 + 1);
        visitor.list(name, owner, maxElements, offset);
      }
    });
  }

  public long addList(String name, String owner, int maxElements) throws IOException {
    byte[] bytes = new byte[RECORD_LEN];
    IOUtils.writeString(name, bytes, 0, LIST_NAME_LEN);
    IOUtils.writeInt(maxElements, bytes, LIST_NAME_LEN);
    bytes[LIST_NAME_LEN + 4] = (byte)((owner == null) ? 1 : 0);
    if (owner != null)
      writeMsisdn(owner, bytes, LIST_NAME_LEN + 4 + 1);
    return append(bytes);
  }

  public void removeList(long offset) throws IOException {
    remove(offset);
  }

  public void alterList(long offset, int maxElements) throws IOException {
    byte[] bytes = new byte[4];
    IOUtils.writeInt(maxElements, bytes, 0);
    write(offset + LIST_NAME_LEN, bytes);
  }

  public interface Visitor {
    public void list(String name, String owner, int maxElements, long offset);
  }
}
