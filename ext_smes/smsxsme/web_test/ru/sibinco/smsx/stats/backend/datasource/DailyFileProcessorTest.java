package ru.sibinco.smsx.stats.backend.datasource;

import junit.framework.TestCase;
import ru.sibinco.smsx.stats.backend.TestUtils;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;

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
    ProgressImpl p = new ProgressImpl();

    processor = new DailyFileProcessor(artefactsDir, new Date(System.currentTimeMillis() - 1212212121212l),
        new Date(System.currentTimeMillis() + 12121212l), p);

    Iterator i = processor.process(new ShutdownIndicator()).iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }

    assertEquals(expected.size(), 0);
  }

  public void testProcessEmptyDateFrom() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new DailyFileProcessor(artefactsDir, new Date(System.currentTimeMillis()+1212121l), null, p);
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }

  public void testProcessEmptyDateTill() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new DailyFileProcessor(artefactsDir, new Date(System.currentTimeMillis()-1212121l), null, p);
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }


  public void testProggress() throws Exception{
    ProgressImpl p = new ProgressImpl();
    processor = new DailyFileProcessor(artefactsDir, null, null, p);
    processor.process(new ShutdownIndicator());
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
