package ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist;

import ru.sibinco.smsx.engine.service.group.datasource.impl.DataFile;

import java.io.File;
import java.io.IOException;
import java.io.EOFException;

import com.eyeline.utils.IOUtils;

/**
 * User: artem
 * Date: 19.11.2008
 */
class MembersFile extends DataFile {

  private static final int LIST_NAME_LEN = 32;
  private static final int RECORD_LEN = LIST_NAME_LEN + ADDRESS_LEN;

  public MembersFile(File file) throws IOException {
    super(file, "SMSCDLMEM", 0x010000, RECORD_LEN);
  }

  public void list(final Visitor visitor) throws IOException {
    list(new RecordVisitor() {
      public void record(byte[] bytes, long offset) {
        String listName = IOUtils.readString(bytes, 0, LIST_NAME_LEN);
        String address = readMsisdn(bytes, LIST_NAME_LEN);
        visitor.member(listName, address, offset);
      }
    });
  }

  public long addMember(String listName, String address) throws IOException {
    byte bytes[] = new byte[RECORD_LEN];
    IOUtils.writeString(listName, bytes, 0, LIST_NAME_LEN);
    writeMsisdn(address, bytes, LIST_NAME_LEN);
    return append(bytes);
  }

  public void removeMember(long offset) throws IOException {
    remove(offset);
  }

  public interface Visitor {
    public void member(String listName, String address, long offset);
  }
}
