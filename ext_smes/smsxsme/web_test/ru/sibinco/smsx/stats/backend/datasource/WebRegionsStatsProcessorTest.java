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
public class WebRegionsStatsProcessorTest extends TestCase{

  private File artefactsDir;

  private WebRegionsStatsProcessor processor;

  private Date date = new Date();

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");

  public void setUp() throws Exception {
    artefactsDir = TestUtils.createTestDir("websms-regions");
    createArtefacts();
  }

  public void tearDown() throws Exception {
    if(artefactsDir != null) {
      TestUtils.recursiveDirRemove(artefactsDir);
    }
  }

  public void testProcessAll() throws Exception {
    Progress p = new Progress();

    final Set expected = new HashSet();
    expected.add(new WebRegion("МР Москва-Москва", true, 1,2));
    expected.add(new WebRegion("МР Москва-Москва", false, 0,1));
    expected.add(new WebRegion("Unknown", false, 1,0));

    processor = new WebRegionsStatsProcessor(artefactsDir, new Date(System.currentTimeMillis() - 1212212121212l), new Date(System.currentTimeMillis() + 12121212l), p);
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
    processor = new WebRegionsStatsProcessor( artefactsDir, new Date(System.currentTimeMillis()+1212121l), null, p);
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
    processor = new WebRegionsStatsProcessor(artefactsDir, new Date(System.currentTimeMillis()-1212121l), null, p);
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
    processor = new WebRegionsStatsProcessor(artefactsDir, null, null, p);
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
          new File(artefactsDir, new StringBuffer().append(sdf.format(date)).append("-websms-users.csv").toString())
      ),"windows-1251")));
      writer.println("INDEX,MSC,ADDRESS,REGION");
      writer.println("1,1,.1.1.79169112907,МР Москва-Москва");
      writer.println("2,1,.1.1.79169112908,МР Москва-Москва");
      writer.println("2,1,.1.1.79169112901,МР Москва-Москва");
      writer.println("2,0,.1.1.79169112901,МР Москва-Москва");
      writer.println("1,0,.1.1.79169112901,Unknown");
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }
}
