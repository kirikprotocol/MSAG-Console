package ru.novosoft.smsc.admin.snmp;

import org.junit.After;
import org.junit.Before;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class TrapDataSourceTestStub {

  protected File snmpDir;


  @Before
  public void before() throws IOException {
    snmpDir = TestUtils.createRandomDir(".snmpDir");
    TestUtils.exportResource(TrapDataSourceTestStub.class.getResourceAsStream("20101109_220719.ucs.csv"),
        new File(snmpDir, "20101109_220719.ucs.csv"), false);
    assertEquals(snmpDir.list().length, 1);
  }

  @After
  public void after() {
    if(snmpDir != null) {
      TestUtils.recursiveDeleteFolder(snmpDir);
    }
  }


}
