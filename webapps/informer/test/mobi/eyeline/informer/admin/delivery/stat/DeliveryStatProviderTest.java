package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.text.ParseException;
import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.10.2010
 * Time: 12:23:57
 */
public class DeliveryStatProviderTest {

  File statDir;

  @Before
  public void before() throws Exception {
    URL u = DeliveryStatProviderTest.class.getResource("./");
    statDir = new File(u.toURI());
  }

  @Test
  public void testWithUnknownDir() throws AdminException {
    DeliveryStatProvider statProvider = new TestDeliveryStatProvider(new File("unknown"), FileSystem.getFSForSingleInst());

    statProvider.accept(null,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        return true;
      }
    });
  }

  @Test
  public void providerTest() throws AdminException, IOException, ParseException {

    DeliveryStatProvider statProvider = new TestDeliveryStatProvider(statDir, FileSystem.getFSForSingleInst());

    final List<DeliveryStatRecord> records = new ArrayList<DeliveryStatRecord>();


    statProvider.accept(null,new DeliveryStatVisitor(){

      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        assertEquals(3, total);
        assertTrue(current<total);
        records.add(rec);
        return true;
      }
    });
    assertEquals(9, records.size());

    records.clear();
    DeliveryStatFilter filter = new DeliveryStatFilter();
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size()==9);

    records.clear();
    filter.setUser("a");
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        assertEquals(total, 3);
        records.add(rec);
        return true;
      }
    });
    assertEquals(records.size(),7);

    //todo fix timezones differences
    records.clear();
    filter.setUser(null);
    List<Integer> taskIds = new ArrayList<Integer>();
    taskIds.add(2);
    filter.setTaskIds(taskIds);
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size()==2);

    records.clear();
    filter.setUser(null);
    filter.setTaskIds(null);

    Calendar c1 = Calendar.getInstance();
    c1.setTimeZone(TimeZone.getTimeZone("UTC"));
    c1.set(2010,10-1,16,12,11);
    filter.setFromDate(c1.getTime());
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertEquals(records.size(), 8);

    records.clear();
    c1.set(2010,10-1,16,12,14);
    filter.setFromDate(c1.getTime());
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size()==6);


    records.clear();
    c1.set(2010,10-1,16,12,14);
    filter.setFromDate(null);
    filter.setTillDate(c1.getTime());
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertEquals(records.size(),3);


    records.clear();
    c1.set(2010,10-1,16,12,15);
    filter.setFromDate(c1.getTime());
    c1.set(2010,10-1,16,14,30);
    filter.setTillDate(c1.getTime());
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size()==4);


    records.clear();
    c1.set(2010,10-1,16,12,15);
    filter.setFromDate(c1.getTime());
    c1.set(2010,10-1,16,14,30);
    filter.setTillDate(c1.getTime());
    filter.setUser("b");
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size() == 1);
  }


  private void testExpectedRecorsSize(DeliveryStatProvider statProvider, int h_f, int h_t, int m_f, int m_t, int expected) throws AdminException {

    final List<DeliveryStatRecord> records = new LinkedList<DeliveryStatRecord>();
    DeliveryStatFilter filter = new DeliveryStatFilter();
    Calendar c = Calendar.getInstance();
    c.setTimeZone(TimeZone.getTimeZone("UTC"));
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    c.set(2010, 9, 16, h_f, m_f);  // months starts from 0 ...
    filter.setFromDate(c.getTime());
    c.set(2010, 9, 16, h_t, m_t);
    filter.setTillDate(c.getTime());

    statProvider.accept(filter, new DeliveryStatVisitor() {
      @Override
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertEquals(records.size(), expected);

  }

  @Test
  public void testMinuteOffset() throws AdminException {

    DeliveryStatProvider statProvider = new TestDeliveryStatProvider(statDir, FileSystem.getFSForSingleInst());

    testExpectedRecorsSize(statProvider, 12, 12, 10, 11, 2);

    testExpectedRecorsSize(statProvider, 12, 12, 11, 12, 2);

    testExpectedRecorsSize(statProvider, 12, 12, 11, 11, 2);

    testExpectedRecorsSize(statProvider, 12, 12, 10, 12, 2);

    testExpectedRecorsSize(statProvider, 12, 12, 12, 13, 0);

    testExpectedRecorsSize(statProvider, 12, 12, 10, 10, 0);

    testExpectedRecorsSize(statProvider, 12, 12, 9, 10, 0);
  }


  @Test
  public void testOOOORecordFromAnotherFile()  throws AdminException{

    DeliveryStatProvider statProvider = new TestDeliveryStatProvider(statDir, FileSystem.getFSForSingleInst());

    testExpectedRecorsSize(statProvider, 10, 10, 0, 59, 1);

  }



}
