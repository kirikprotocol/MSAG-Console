package ru.novosoft.smsc.admin.snmp;

import org.junit.After;
import org.junit.Before;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class TrapDataSourceTestStub {

  protected File snmpDir;
  protected MemoryFileSystem fs = new MemoryFileSystem();


  @Before
  public void before() throws IOException, AdminException {
    snmpDir = fs.mkdirs("snmp");
    fs.createNewFile(new File(snmpDir, "20101109_220719.ucs.csv"), TrapDataSourceTestStub.class.getResourceAsStream("20101109_220719.ucs.csv"));
  }

}
