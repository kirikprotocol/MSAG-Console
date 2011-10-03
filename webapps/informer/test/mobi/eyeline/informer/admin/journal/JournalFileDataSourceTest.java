package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
public class JournalFileDataSourceTest { 

  private static JournalFileDataSource ds;

  private static File jDir = new File(".journalDir");

  @BeforeClass
  public static void before() throws Exception {
    TestUtils.recursiveDeleteFolder(jDir);
    ds = new JournalFileDataSource(jDir, FileSystem.getFSForSingleInst());
  }


  @Test
  public void convertTest() throws Exception {
    JournalRecord record = new JournalRecord(JournalRecord.Type.CHANGE);
    record.setTime(1000);
    record.setUser("user1");
    record.setDescription("descriptionKey", "arg1", "arg2");
    record.setSubject(Subject.CONFIG);
    testConverting(record);
  }

  @Test
  public void convertWithSepTest() throws Exception {
    JournalRecord record = new JournalRecord(JournalRecord.Type.CHANGE);
    record.setTime(1000);
    record.setUser("|us|er|1");
    record.setDescription("descri|pti|onKey", "arg|1", "ar|g2");
    record.setSubject(Subject.CONFIG);
    testConverting(record);
  }


  @Test
  public void convertTestNewLine() throws Exception {
    JournalRecord record = new JournalRecord(JournalRecord.Type.CHANGE);
    record.setTime(1000);
    record.setUser("u\ns|er|1\n");
    record.setDescription("descri|pti|onKey", "\nar\r\ng|1", "ar|g\n2");
    record.setSubject(Subject.CONFIG);
    testConverting(record);
  }

  @Test
  public void convertTestTab() throws Exception {
    JournalRecord record = new JournalRecord(JournalRecord.Type.CHANGE);
    record.setTime(1000);
    record.setUser("u\ts|er|1\t");
    record.setDescription("descri|pti|onKey", "\tar\t\tg|1", "ar|g\t2");
    record.setSubject(Subject.CONFIG);
    testConverting(record);
  }

  private void testConverting(JournalRecord record) throws Exception{
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new OutputStreamWriter(os));
      ds.write(record, writer);
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
    String recordStr = os.toString();
    System.out.println("Record in file: "+recordStr);
    BufferedReader reader = null;
    JournalRecord record2;
    try{
      reader = new BufferedReader(new InputStreamReader(new ByteArrayInputStream(recordStr.getBytes())));
      String line = reader.readLine();
      assertNotNull(line);
      assertNull(reader.readLine());
      record2 = JournalFileDataSource.convert(line);
    }finally {
      if(reader != null) {
        try{
          reader.close();
        }catch (IOException e){}
      }
    }
    assertEquals(record, record2);

  }

  @Test
  public void writeRead() throws Exception {
    final JournalRecord record1 = new JournalRecord(JournalRecord.Type.CHANGE).setTime(new Date(1).getTime()).setUser("user1")
        .setDescription("descr").setSubject(Subject.USERS);
    final JournalRecord record2 = new JournalRecord(JournalRecord.Type.CHANGE).setTime(new Date(100000).getTime()).setUser("user2")
        .setDescription("descr").setSubject(Subject.CONFIG);
    final JournalRecord record3 = new JournalRecord(JournalRecord.Type.CHANGE).setTime(new Date().getTime()).setUser("user1")
        .setDescription("descr").setSubject(Subject.USERS);
    final JournalRecord record4 = new JournalRecord(JournalRecord.Type.CHANGE).setTime(new Date(System.currentTimeMillis()-(1000*60*60*24)).getTime()).setUser("user2")
        .setDescription("descr").setSubject(Subject.CONFIG);
    ds.addRecords(record1, record2, record3, record4);

    ds.visit(new JournalFilter().setUser("user1"), new JournalVisitor() {
      public boolean visit(JournalRecord r) {
        assertTrue(r.equals(record1) || r.equals(record3));
        return true;
      }
    });
    ds.visit(new JournalFilter().setUser("subj1"), new JournalVisitor() {
      public boolean visit(JournalRecord r) {
        assertTrue(r.equals(record1) || r.equals(record3));
        return true;
      }
    });
    ds.visit(new JournalFilter().setEndDate(new Date(100001)), new JournalVisitor() {
      public boolean visit(JournalRecord r) {
        assertTrue(r.equals(record1) || r.equals(record2));
        return true;
      }
    });
    ds.visit(new JournalFilter().setStartDate(new Date(System.currentTimeMillis()-(2*1000*60*60*24))), new JournalVisitor() {
      public boolean visit(JournalRecord r) {
        assertTrue(r.equals(record3) || r.equals(record4));
        return true;
      }
    });

  }

  @Test
  public void truncateTest() throws Exception{
    Date date = new Date(System.currentTimeMillis() + 10000000);
    final JournalRecord record1 = new JournalRecord(JournalRecord.Type.CHANGE).setTime(date.getTime()).setUser("user1")
        .setDescription("descr").setSubject(Subject.CONFIG);


    ds.addRecords(record1);

    ds.visit(new JournalFilter().setStartDate(new Date(date.getTime() - 1)), new JournalVisitor() {
      public boolean visit(JournalRecord r) {
        assertTrue(r.equals(record1));
        return true;
      }
    });

    PrintWriter writer = null;
    char sep = File.separatorChar;
    try{
      writer = new PrintWriter(new BufferedWriter(new FileWriter(
          new File(jDir, new SimpleDateFormat("yyyy"+sep+"MM"+sep+"dd"+sep+"HH").format(date)+".csv")
          ,true
      )));
      writer.write("dsafsasfafsafsasf");
    }finally {
      if(writer != null) {
        writer.close();
      }
    }

    try{
      ds.visit(new JournalFilter().setStartDate(new Date(date.getTime() - 1)), new JournalVisitor() {
        public boolean visit(JournalRecord r) {
          assertTrue(r.equals(record1));
          return true;
        }
      });
      assertTrue(false);
    }catch (AdminException e){}

    ds = new JournalFileDataSource(jDir, FileSystem.getFSForSingleInst());

    ds.visit(new JournalFilter().setStartDate(new Date(date.getTime() - 1)), new JournalVisitor() {
      public boolean visit(JournalRecord r) {
        assertTrue(r.equals(record1));
        return true;
      }
    });


  }


  @AfterClass
  public static void after() {
    if(jDir != null) {
     TestUtils.recursiveDeleteFolder(jDir);
    }

  }



}
