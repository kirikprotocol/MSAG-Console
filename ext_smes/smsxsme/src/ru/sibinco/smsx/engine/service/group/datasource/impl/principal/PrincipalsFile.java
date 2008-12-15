package ru.sibinco.smsx.engine.service.group.datasource.impl.principal;

import com.eyeline.utils.IOUtils;
import ru.sibinco.smsx.engine.service.group.datasource.Principal;
import ru.sibinco.smsx.engine.service.group.datasource.impl.DataFile;

import java.io.EOFException;
import java.io.File;
import java.io.IOException;

/**
 * User: artem
 * Date: 18.11.2008
 */
class PrincipalsFile extends DataFile {

  private static final int RECORD_LEN = ADDRESS_LEN + 4/*max lists*/ + 4/*max elements per list*/;

  public PrincipalsFile(File file) throws IOException {
    super(file, "SMSCDLPRC", 0x010000, RECORD_LEN);
  }

  public void list(final Visitor result) throws IOException {
    list(new RecordVisitor() {
      public void record(byte[] bytes, long offset) {
        Principal p = new Principal(readMsisdn(bytes, 0));
        p.setMaxLists(IOUtils.readInt(bytes, ADDRESS_LEN));
        p.setMaxMembersPerList(IOUtils.readInt(bytes, ADDRESS_LEN + 4));
        result.principal(p, offset);
      }
    });
  }

  public long addPrincipal(Principal p) throws IOException{
    byte[] bytes = new byte[RECORD_LEN];
    writeMsisdn(p.getAddress(), bytes, 0);
    IOUtils.writeInt(p.getMaxLists(), bytes, ADDRESS_LEN);
    IOUtils.writeInt(p.getMaxMembersPerList(), bytes, ADDRESS_LEN + 4);
    return append(bytes);
  }

  public void removePrincipal(long offset) throws IOException {
    remove(offset);
  }

  public int getPrincipalsNumber() {
    return recordsNumber;
  }

  interface Visitor {
    public void principal(Principal p, long offset);
  }
}
