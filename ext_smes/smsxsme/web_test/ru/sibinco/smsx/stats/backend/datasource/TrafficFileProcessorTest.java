package ru.sibinco.smsx.stats.backend.datasource;

import junit.framework.TestCase;
import ru.sibinco.smsx.stats.backend.TestUtils;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
public class TrafficFileProcessorTest extends TestCase {

   private File artefactsDir;

  private TrafficFileProcessor processor;

  private Date date = new Date();

  private SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");

  public void setUp() throws Exception {
    artefactsDir = TestUtils.createTestDir("traffic");
    createArtefacts();
  }

  public void tearDown() throws Exception {
    if(artefactsDir != null) {
      TestUtils.recursiveDirRemove(artefactsDir);
    }
  }

  public void testProcessAll() throws Exception {
    ProgressImpl p = new ProgressImpl();
    final Set expected = new HashSet();
    expected.add(new Traffic("МР Дальний Восток-Якутск",0, false, 440));
    expected.add(new Traffic("МР Дальний Восток-Якутск",1, false, 320));
    expected.add(new Traffic("МР Дальний Восток-Якутск",0, true, 120));
    processor = new TrafficFileProcessor(artefactsDir, null, null, p, null);

    Iterator i = processor.process(new ShutdownIndicator()).iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }
    assertEquals(expected.size(), 0);
  }

  public void testProcessService() throws Exception {
    ProgressImpl p = new ProgressImpl();
    final Set expected = new HashSet();
    expected.add(new Traffic("МР Дальний Восток-Якутск",0, false, 440));
    expected.add(new Traffic("МР Дальний Восток-Якутск",0, true, 120));

    processor = new TrafficFileProcessor(artefactsDir, new Date(System.currentTimeMillis() - 1212212121212l),
        new Date(System.currentTimeMillis() + 12121212l), p, new HashSet(1){{add(new Integer(0));}});

    Iterator i = processor.process(new ShutdownIndicator()).iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }
    assertEquals(expected.size(), 0);
  }

  public void testProcessEmptyService() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new TrafficFileProcessor(artefactsDir, null, null, p, new HashSet(1){{add(new Integer(2));}});
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }


  public void testProcessEmptyDateFrom() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new TrafficFileProcessor(artefactsDir, new Date(System.currentTimeMillis()+1212121l), null, p, null);
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }

  public void testProcessEmptyDateTill() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new TrafficFileProcessor(artefactsDir, new Date(System.currentTimeMillis()-1212121l), null, p, null);
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }


  public void testProggress() throws Exception{
    ProgressImpl p = new ProgressImpl();
    processor = new TrafficFileProcessor(artefactsDir, null, null, p, null);
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
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }
}
