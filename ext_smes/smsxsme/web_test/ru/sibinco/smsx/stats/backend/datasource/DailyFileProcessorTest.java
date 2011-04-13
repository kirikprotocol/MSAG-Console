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
public class DailyFileProcessorTest extends TestCase {

  private File artefactsDir;

  private DailyFileProcessor processor;

  private Date date = new Date();

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");

  public void setUp() throws Exception {
    artefactsDir = TestUtils.createTestDir("websms-daily");
    createArtefacts();
  }

  public void tearDown() throws Exception {
    if(artefactsDir != null) {
      TestUtils.recursiveDirRemove(artefactsDir);
    }
  }

  public void testProcessAll() throws Exception {
    String d = sdf.format(date);
    final Set expected = new HashSet();

    expected.add(new WebDaily(d, "МР Дальний Восток-Якутск", false, 760));
    expected.add(new WebDaily(d, "МР Дальний Восток-Якутск", true, 120));
    Progress p = new Progress();

    processor = new DailyFileProcessor(artefactsDir, new Date(System.currentTimeMillis() - 1212212121212l),
        new Date(System.currentTimeMillis() + 12121212l), p);
    processor.process(new Visitor() {
      public boolean visit(Object o) throws StatisticsException {
        assertTrue(expected.remove(o));
        return true;
      }
    });
    assertEquals(expected.size(), 0);
  }

  public void testProcessEmptyDateFrom() throws Exception {
    Progress p = new Progress();
    processor = new DailyFileProcessor(artefactsDir, new Date(System.currentTimeMillis()+1212121l), null, p);
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
    processor = new DailyFileProcessor(artefactsDir, new Date(System.currentTimeMillis()-1212121l), null, p);
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
    processor = new DailyFileProcessor(artefactsDir, null, null, p);
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
          new File(artefactsDir, new StringBuffer().append(sdf.format(date)).append("-traffic.csv").toString())
      ),"windows-1251")));
      writer.println("SERVICE_ID,MSC,REGION,SRC_SME_ID");
      writer.println("0,0,МР Дальний Восток-Якутск,websms,220");
      writer.println("0,0,МР Дальний Восток-Якутск,websyssms,220");
      writer.println("1,0,МР Дальний Восток-Якутск,websms,320");
      writer.println("0,1,МР Дальний Восток-Якутск,websms,120");
      writer.println("0,1,МР Дальний Восток-Якутск,dasasd,120");
      writer.println("0,1,МР Дальний Восток-Якутск,tretet,120");
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }
}
