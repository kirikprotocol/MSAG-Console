package ru.sibinco.smsx.stats.backend.datasource;

import junit.framework.TestCase;
import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.TestUtils;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxFileProcessorTest extends TestCase {

  private File artefactsDir;

  private SmsxFileProcessor processor;

  public void setUp() throws Exception {
    artefactsDir = TestUtils.createTestDir("smsx");
  }

  public void tearDown() throws Exception {
    if(artefactsDir != null) {
      TestUtils.recursiveDirRemove(artefactsDir);
    }
  }

  public void testProcess_SeveralFiles() throws IOException, StatisticsException {
    createFile("20110507", new String[]{
        "0,.1.1.79139095312,МР Сибирь-Новосибирск"
    });
    createFile("20110508", new String[]{
        "0,.1.1.79139095312,МР Сибирь-Новосибирск"        // Тот же самый номер
    });

    processor = new SmsxFileProcessor(artefactsDir, null, null, new ProgressImpl(), null);
    Iterator iter = processor.process(new ShutdownIndicator()).iterator();

    assertTrue(iter.hasNext());

    SmsxUsers users = (SmsxUsers)iter.next();
    assertEquals("МР Сибирь-Новосибирск", users.getRegion());
    assertEquals(0, users.getServiceId());
    assertEquals(1, users.getCount());

    assertFalse(iter.hasNext());
  }

  public void testProcess_SingleFile() throws Exception {

    createFile("20110601", new String[]{
      "0,.1.1.79139095312,МР Сибирь-Новосибирск",
      "1,.1.1.79139095683,МР Сибирь-Новосибирск",
      "0,.1.1.79139489906,МР Сибирь-Новосибирск",
      "0,.1.1.79109424802,Unknown"
    });

    final Set expected = new HashSet();
    expected.add(new SmsxUsers(0,"МР Сибирь-Новосибирск", 2));
    expected.add(new SmsxUsers(1,"МР Сибирь-Новосибирск", 1));
    expected.add(new SmsxUsers(0,"Unknown", 1));

    ProgressImpl p = new ProgressImpl();
    processor = new SmsxFileProcessor(artefactsDir, null, null, p, null);

    Iterator i = processor.process(new ShutdownIndicator()).iterator();
    while(i.hasNext()) {
      assertTrue(expected.remove(i.next()));
    }
    assertEquals(expected.size(), 0);
  }


  public void testProcessEmptyService() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new SmsxFileProcessor(artefactsDir, null, null, p, new HashSet(1){{add(new Integer(2));}});
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }


  public void testProcessEmptyDateFrom() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new SmsxFileProcessor(artefactsDir, new Date(System.currentTimeMillis()+1212121l), null, p, null);
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }

  public void testProcessEmptyDateTill() throws Exception {
    ProgressImpl p = new ProgressImpl();
    processor = new SmsxFileProcessor(artefactsDir, new Date(System.currentTimeMillis()-1212121l), null, p, null);
    assertTrue(processor.process(new ShutdownIndicator()).isEmpty());
  }


  public void testProggress() throws Exception{
    ProgressImpl p = new ProgressImpl();
    processor = new SmsxFileProcessor(artefactsDir, null, null, p, null);
    processor.process(new ShutdownIndicator());
    assertEquals(p.getProgress(), 100);
  }

  private void createFile(String date, String[] lines) throws IOException {
    PrintWriter writer = null;
    try{
      File f = new File(artefactsDir, date+"-smsx-users.csv");
      writer = new PrintWriter(new OutputStreamWriter(new FileOutputStream(f),"windows-1251"));
      writer.println("SERVICE_ID,SRC_ADDRESS,REGION");
      for (int i=0; i<lines.length; i++) {
        writer.println(lines[i]);
      }
    } finally {
      if(writer != null) {
        writer.close();
      }
    }
  }
}
