package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.*;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

import static org.junit.Assert.*;

/**
 * author: Aleksandr Khalitov
 */
public class DetailedPeriodSaveStrategyTest {

  private MemoryFileSystem fs;

  private TestDeliveryManager deliveryManager;

  private User user;

  private ResourceOptions resourceOptions;

  private UserCPsettings settings;

  private FileResource resource;

  private SingleUserContentPContextStub context;

  private File localDir;

  private File resourceDir;

  @Before
  public void init() throws AdminException {
    fs = new MemoryFileSystem();
    deliveryManager = new TestDeliveryManager(new File(""), fs, false, 50);

    settings = prepareSettings();
    user = prepareUser();
    resourceOptions = new ResourceOptions(user, new File("workDir"), settings, 60);
    resourceOptions.setSourceAddress(new Address("+79139489906"));
    resourceOptions.setEncoding("utf-8");
    resource = new LocalResourceStubWithChecking(new File("dir"), fs);
    context = new SingleUserContentPContextStub(prepareUser(), deliveryManager, fs);


    resourceDir = new File("dir");
    fs.mkdirs(resourceDir);

    localDir = new File("workDir", "detailedPeriodLocalCopy");
    fs.mkdirs(localDir);
  }

  public DetailedPeriodSaveStrategy createStrategy() throws AdminException {
    DetailedPeriodSaveStrategy s = new DetailedPeriodSaveStrategy(context, resource , resourceOptions);
    s.setReportTimeoutMillis(0);
    s.setReportFileFormat("yyyyMMddHHmmssSSS");
    return s;
  }


  @After
  public void shutdown() {
    if(deliveryManager != null) {
      deliveryManager.shutdown();
    }
  }


  private UserCPsettings prepareSettings() {
    UserCPsettings settings = new UserCPsettings();
    settings.setHost("host");
    settings.setWorkType(UserCPsettings.WorkType.detailed_period);
    settings.setSourceAddress(new Address("1312313131"));
    settings.setCreateReports(true);
    settings.setReportTimeoutMin(0);
    return settings;
  }


  private static User prepareUser() {
    User u = new User();
    u.setLogin("a");
    u.setPassword("");
    u.setAllRegionsAllowed(true);
    return u;
  }

  private File createEmptyLocalFile(String posfix) throws AdminException{
    File file = new File(localDir, "test.csv"+(posfix == null ? "" : posfix));
    fs.createNewFile(file);
    return file;
  }


  private File createEmptyResourceFile(String posfix) throws AdminException{
    File file = new File(resourceDir, "test.csv"+(posfix == null ? "" : posfix));
    fs.createNewFile(file);
    return file;
  }


  private File prepareResourceFileWithErrors(int lines, int errors, String posfix) throws AdminException {
    File file = new File(resourceDir, "test.csv"+(posfix == null ? "" : posfix));
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(file, false))));
      prepareFileWithError(writer, lines, errors);
    }finally{
      if(writer != null) {
        writer.close();
      }
    }
    assertTrue("File doesn't exist!", fs.exists(file));
    return file;
  }


  private File prepareResourceFile(boolean isSingleText, int lines, String posfix, CpFileFormat fileFormat) throws AdminException {
    File file = new File(resourceDir, "test.csv"+(posfix == null ? "" : posfix));
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(file, false))));
      prepareFile(writer, lines, isSingleText, fileFormat);
    }finally{
      if(writer != null) {
        writer.close();
      }
    }
    assertTrue("File doesn't exist!", fs.exists(file));
    return file;
  }

  private File prepareLocalFile(boolean isSingleText, int lines,  String posfix, CpFileFormat fileFormat) throws AdminException {
    File file = new File(localDir, "test.csv"+(posfix == null ? "" : posfix));
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(file, false))));
      prepareFile(writer, lines, isSingleText, fileFormat);
    }finally{
      if(writer != null) {
        writer.close();
      }
    }
    assertTrue("File doesn't exist!", fs.exists(file));
    return file;
  }

  private void prepareFile(PrintWriter writer, int lines, boolean isSingleText, CpFileFormat fileFormat) {
    long nb = 79130000000l;
    for(int i=0; i<lines; i++) {
      long n = nb + i;
      if(fileFormat == CpFileFormat.EYELINE) {
        writer.print("+");writer.print(n);writer.print("|id");writer.print(n);writer.print("|key");writer.print(n);writer.print("|text");writer.println(isSingleText ? "" : i);
      }else {
        writer.print("id");writer.print(n);writer.print("|");writer.print("+");writer.print(n);writer.print("|text");writer.println(isSingleText ? "" : i);
      }
    }
  }
  private void prepareFileWithError(PrintWriter writer, int lines, int errors) {
    for(int i=0; i<lines; i++) {
      writer.print("+7913948990"+i);writer.println("|text");
    }
    for(int i=0; i<errors; i++) {
      writer.print("+");writer.print("dasda");writer.println("|text");
    }
  }


  @Test
  public void testSynchrInProgressNothing() throws AdminException {

    File resourceFile = prepareResourceFile(false, 0, ".active", null);
    File localFile = prepareLocalFile(false, 0,  ".active", null);

    createStrategy().synchronize(true);

    assertTrue("File doesn't exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist!", fs.exists(localFile));

  }

  @Test
  public void testSynchrInProgressRename() throws AdminException {

    File resourceFile = prepareResourceFile(false, 0, null, null);
    File localFile = prepareLocalFile(false, 0, ".active", null);

    createStrategy().synchronize(true);

    assertFalse("File exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath()+".active")));
    assertTrue("File doesn't exist!", fs.exists(localFile));

  }

  @Test
  public void testSynchrInProgressRemove() throws AdminException {

    File localFile = prepareLocalFile(false, 0, ".active", null);
    File report = createEmptyLocalFile(".20111213141516177.report");

    createStrategy().synchronize(true);

    assertFalse("File exist", fs.exists(localFile));
    assertFalse("File exist", fs.exists(report));

  }

  @Test
  public void testSynchrFinished() throws AdminException {

    File resourceFile = prepareResourceFile(false, 0, ".active", null);
    File localFile = prepareLocalFile(false, 0, ".finished", null);
    File report = prepareLocalFile(false, 1, ".20111213141516177.report.final", null);
    File report1 = prepareLocalFile(false, 1, ".20111213141516179.report", null);

    createStrategy().synchronize(true);

    assertTrue("File doesn't exist", fs.exists(localFile));
    assertFalse("File exists", fs.exists(report));
    assertFalse("File exists", fs.exists(report1));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.finished")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.20111213141516177.report.final")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.20111213141516179.report")));
    assertFalse("File exists", fs.exists(resourceFile));
  }


  @Test
  public void testCleanOnStart() throws AdminException {
    File file = createEmptyLocalFile(".tmp");
    assertTrue(fs.exists(file));
    createStrategy();
    assertFalse(fs.exists(file));
  }

  @Test
  public void testCreate() throws Exception {  //проверяет, что рассылка создается
    File file = prepareResourceFile(false, 10, null, null);
    assertTrue(fs.exists(file));
    DetailedPeriodSaveStrategy strategy = createStrategy();
    strategy.process(true);
    assertFalse(fs.exists(file));
    assertTrue(fs.exists(new File(file.getParent(), file.getName()+".active")));

    assertTrue(fs.exists(new File(localDir, file.getName()+".active")));

    boolean reportInfo = false;

    for(File f : fs.listFiles(localDir)) {
      String name = f.getName();
      if(name.contains("period")){
        reportInfo = true;
      }
    }

    assertTrue(reportInfo);
  }


  @Test
  public void testEyelineReport() throws Exception {
    testReport(CpFileFormat.EYELINE);
  }

  @Test
  public void testMtsReport() throws Exception {
    testReport(CpFileFormat.MTS);
  }

  private void testReport(CpFileFormat fileFormat) throws Exception {          //проходит весь цикл, проверяет, что все сообщения попали в отчеты на ресурсе
    File file = prepareResourceFile(false, 1000, null, fileFormat);
    assertTrue(fs.exists(file));
    context.setCpFileFormat(fileFormat);
    DetailedPeriodSaveStrategy strategy = createStrategy();
    strategy.process(true);

    Thread.sleep(1000);

    deliveryManager.forceModifyDeliveries(); // send 50 messages


    for(int i=0;i<20;i++) {
      Thread.sleep(10);
      deliveryManager.forceModifyDeliveries();
      strategy.process(true);
    }

    int countMessages = 0;
    boolean _final = false, _finished = false;
    for(File f : fs.listFiles(resourceDir)) {
      String name = f.getName();
      System.out.println(name);
      if(name.contains("report")) {
        countMessages+=countLines(f);
      }
      if(name.contains("final"))  {
        _final = true;
      }
      if(name.contains("finished")) {
        _finished = true;
      }
    }

    assertTrue("Delivery is not finished", _finished);

    assertTrue("Final report is not found", _final);

    assertEquals(1000, countMessages);

    assertMessages(countMessages, getFinalStates());
  }

  private Set<MessageState> getFinalStates() {
    Set<MessageState> set = new HashSet<MessageState>();
    set.add(MessageState.Delivered);
    set.add(MessageState.Failed);
    return set;
  }

  private void assertMessages(int countMessages, final Set<MessageState> states) throws Exception{

    final Delivery[] deliveries = new Delivery[1];
    deliveryManager.getDeliveries("","", new DeliveryFilter(), 100, new Visitor<Delivery>() {
      @Override
      public boolean visit(Delivery value) throws AdminException {
        deliveries[0] = value;
        return false;
      }
    });
    assertNotNull(deliveries[0]);
    MessageFilter filter = new MessageFilter(deliveries[0].getId(), deliveries[0].getCreateDate(), new Date());
    assertEquals(deliveryManager.countMessages("", "",filter) , countMessages);

    if(states != null) {
      deliveryManager.getMessages("","", filter, 10000, new Visitor<Message>() {
        @Override
        public boolean visit(Message value) throws AdminException {
          if(!states.contains(value.getState())) {
            fail();
          }
          return true;
        }
      }
      );
    }

  }


  @Test
  public void testErrorReport() throws Exception {
    File file = prepareResourceFileWithErrors(1, 4, null);
    assertTrue(fs.exists(file));
    DetailedPeriodSaveStrategy strategy = createStrategy();
    strategy.process(true);

    Thread.sleep(1000);

    deliveryManager.forceModifyDeliveries(); // send 50 messages

    strategy.process(true);

    boolean _final = false, _finished = false;
    int countMessages = 0;
    int report = 0;
    for(File f : fs.listFiles(resourceDir)) {
      String name = f.getName();
      System.out.println(name);
      if(name.contains("report")) {
        report++;
        countMessages+=countLines(f);
      }
      if(name.contains("final"))  {
        _final = true;
      }
      if(name.contains("finished")) {
        _finished = true;
      }
    }
    assertTrue("Delivery is not finished", _finished);

    assertTrue("Final report is not found", _final);

    assertEquals(2, report);

    assertEquals(5, countMessages);

    assertMessages(1, null);

  }

  @Test
  public void testAllErrors() throws Exception {
    File file = prepareResourceFileWithErrors(0, 4, null);
    assertTrue(fs.exists(file));
    DetailedPeriodSaveStrategy strategy = createStrategy();
    strategy.process(true);

    Thread.sleep(1000);

    deliveryManager.forceModifyDeliveries(); // send 50 messages

    strategy.process(true);

    assertEquals(deliveryManager.countDeliveries("","", new DeliveryFilter()), 0);

    File csvFile = new File(localDir, "test.csv");
    assertFalse(fs.exists(csvFile));
    assertFalse(fs.exists(new File(csvFile.getAbsolutePath()+".active")));
    assertFalse(fs.exists(new File(csvFile.getAbsolutePath()+".error")));

    assertTrue(fs.exists(new File(resourceDir, "test.csv.error")));
  }


  @Test
  public void testLocalCleanupOnFinished() throws AdminException {      // очистка локальной директории, если на ресурс нет csv файла
    File csvFile = new File(localDir, "test.csv");
    File finished = new File(csvFile.getParent(), csvFile.getName()+".finished");
    fs.createNewFile(finished);
    File report1 =   new File(csvFile.getParent(), csvFile.getName()+".20110203145634678.report");
    fs.createNewFile(report1);
    File report2 =   new File(csvFile.getParent(), csvFile.getName()+".20110203145634678.report.final");
    fs.createNewFile(report2);
    File reportInfo1 =   new File(csvFile.getParent(), csvFile.getName()+".period.20110203145634.12345");
    fs.createNewFile(reportInfo1);
    File reportInfo2 =   new File(csvFile.getParent(), csvFile.getName()+".period.20110203145630.12341");
    fs.createNewFile(reportInfo2);

    assertTrue(fs.exists(finished) && fs.exists(report1) && fs.exists(report2)  && fs.exists(reportInfo1)  && fs.exists(reportInfo2));

    DetailedPeriodSaveStrategy strategy = createStrategy();
    strategy.process(true);

    assertFalse("Some of files are exist!", fs.exists(finished) && fs.exists(report1) && fs.exists(report2)  && fs.exists(reportInfo1)  && fs.exists(reportInfo2));
  }

  @Test
  public void testLocalCleanupOnProcess() throws AdminException {    // очистка локальной директории, если на ресурс нет csv файла
    File csvFile = new File(localDir, "test.csv");
    File active = new File(csvFile.getParent(), csvFile.getName()+".active");
    fs.createNewFile(active);
    File report1 =   new File(csvFile.getParent(), csvFile.getName()+".20110203145634678.report");
    fs.createNewFile(report1);
    File report2 =   new File(csvFile.getParent(), csvFile.getName()+".20110203145634621.report");
    fs.createNewFile(report2);
    File reportInfo1 =   new File(csvFile.getParent(), csvFile.getName()+".period.20110203145634.12345");
    fs.createNewFile(reportInfo1);
    File reportInfo2 =   new File(csvFile.getParent(), csvFile.getName()+".period.20110203145630.12341");
    fs.createNewFile(reportInfo2);

    assertTrue(fs.exists(active) && fs.exists(report1) && fs.exists(report2)  && fs.exists(reportInfo1)  && fs.exists(reportInfo2));

    DetailedPeriodSaveStrategy strategy = createStrategy();
    strategy.process(true);

    assertFalse("Some of files are exist!", fs.exists(active) && fs.exists(report1) && fs.exists(report2) && fs.exists(reportInfo1) && fs.exists(reportInfo2));
  }


  private int countLines(File f) throws Exception{
    BufferedReader reader = null;
    int counter = 0;
    try{
      reader = new BufferedReader(new InputStreamReader(fs.getInputStream(f)));
      while(reader.readLine()!= null) {
        counter++;
      }
      return counter;
    }finally {
      if(reader != null) {
        try{
          reader.close();
        }catch (Exception ignored){}
      }
    }
  }

}
