package ru.sibinco.smsx.stats.backend.datasource;

import junit.framework.TestCase;
import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.TestUtils;
import ru.sibinco.smsx.stats.backend.Visitor;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxFileProcessorTest extends TestCase {

  private File artefactsDir;

  private SmsxFileProcessor processor;

  private Date date = new Date();

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");

  public void setUp() throws Exception {
    artefactsDir = TestUtils.createTestDir("smsx");
    createArtefacts();
  }

  public void tearDown() throws Exception {
    if(artefactsDir != null) {
      TestUtils.recursiveDirRemove(artefactsDir);
    }
  }

  public void testProcessAll() throws Exception {

    final Set expected = new HashSet();
    expected.add(new SmsxUsers(0,"МР Сибирь-Новосибирск", 2));
    expected.add(new SmsxUsers(1,"МР Сибирь-Новосибирск", 1));
    expected.add(new SmsxUsers(0,"Unknown", 1));
    Progress p = new Progress();
    processor = new SmsxFileProcessor(artefactsDir, null, null, p, null);
    processor.process(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        assertTrue(expected.remove(o));
        return true;
      }
    });
    assertEquals(expected.size(), 0);
  }

  public void testProcessService() throws Exception {
    Progress p = new Progress();

    final Set expected = new HashSet();
    expected.add(new SmsxUsers(0,"МР Сибирь-Новосибирск",2));
    expected.add(new SmsxUsers(0,"Unknown",1));

    processor = new SmsxFileProcessor(artefactsDir, new Date(System.currentTimeMillis() - 1212212121212l),
        new Date(System.currentTimeMillis() + 12121212l), p, new HashSet(1){{add(new Integer(0));}});
    processor.process(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        assertTrue(expected.remove(o));
        return true;
      }
    });

    assertEquals(expected.size(), 0);
  }

  public void testProcessEmptyService() throws Exception {
    Progress p = new Progress();
    processor = new SmsxFileProcessor(artefactsDir, null, null, p, new HashSet(1){{add(new Integer(2));}});
    final boolean[] ok = new boolean[]{true};
    processor.process(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        ok[0] = false;
        return false;
      }
    });
    assertTrue(ok[0]);
  }


  public void testProcessEmptyDateFrom() throws Exception {
    Progress p = new Progress();
    processor = new SmsxFileProcessor(artefactsDir, new Date(System.currentTimeMillis()+1212121l), null, p, null);
    final boolean[] ok = new boolean[]{true};
    processor.process(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        ok[0] = false;
        return false;
      }
    });
    assertTrue(ok[0]);
  }

  public void testProcessEmptyDateTill() throws Exception {
    Progress p = new Progress();
    processor = new SmsxFileProcessor(artefactsDir, new Date(System.currentTimeMillis()-1212121l), null, p, null);
    final boolean[] ok = new boolean[]{true};
    processor.process(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        ok[0] = false;
        return false;
      }
    });
    assertTrue(ok[0]);
  }


  public void testProggress() throws Exception{
    Progress p = new Progress();
    processor = new SmsxFileProcessor(artefactsDir, null, null, p, null);

    processor.process(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        return true;
      }
    });
    assertEquals(p.getProgress(), 100);
  }

  private void createArtefacts() throws Exception{
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(
          new File(artefactsDir, new StringBuffer().append(sdf.format(date)).append("-smsx-users.csv").toString())
      ),"windows-1251")));
      writer.println("SERVICE_ID,SRC_ADDRESS,REGION");
      writer.println("0,.1.1.79139095312,МР Сибирь-Новосибирск");
      writer.println("1,.1.1.79139095683,МР Сибирь-Новосибирск");
      writer.println("0,.1.1.79139489906,МР Сибирь-Новосибирск");
      writer.println("0,.1.1.79109424802,Unknown");
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }

}
