package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.Functions;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.TimeZone;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertEquals;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.10.2010
 * Time: 12:23:57
 */
public class DeliveryStatProviderTest {

  File statDir;
  private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");
  private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();


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
         assertEquals(2, total);
         assertTrue(current<total);
         records.add(rec);         
         return true;
       }
     });
     assertEquals(8, records.size());

     records.clear();
     DeliveryStatFilter filter = new DeliveryStatFilter();
     statProvider.accept(filter,new DeliveryStatVisitor(){
       public boolean visit(DeliveryStatRecord rec, int total, int current) {
         records.add(rec);
         return true;
       }
     });
     assertTrue(records.size()==8);

     records.clear();
     filter.setUser("a");
     statProvider.accept(filter,new DeliveryStatVisitor(){
       public boolean visit(DeliveryStatRecord rec, int total, int current) {
         assertTrue(total==2);
         records.add(rec);
         return true;
       }
     });
     assertTrue(records.size()==6);

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
     c1.set(2010,10-1,16,12,12);
     filter.setFromDate(Functions.convertTime(c1.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));
     statProvider.accept(filter,new DeliveryStatVisitor(){
       public boolean visit(DeliveryStatRecord rec, int total, int current) {
         records.add(rec);
         return true;
       }
     });
     assertTrue(records.size()==8);

    records.clear();
    c1.set(2010,10-1,16,12,14);
    filter.setFromDate(Functions.convertTime(c1.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));
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
    filter.setTillDate(Functions.convertTime(c1.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size()==2);


    records.clear();
    c1.set(2010,10-1,16,12,15);
    filter.setFromDate(Functions.convertTime(c1.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));
    c1.set(2010,10-1,16,14,30);
    filter.setTillDate(Functions.convertTime(c1.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size()==4);


    records.clear();
    c1.set(2010,10-1,16,12,15);
    filter.setFromDate(Functions.convertTime(c1.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));
    c1.set(2010,10-1,16,14,30);
    filter.setTillDate(Functions.convertTime(c1.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));
    filter.setUser("b");
    statProvider.accept(filter,new DeliveryStatVisitor(){
      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        records.add(rec);
        return true;
      }
    });
    assertTrue(records.size()==1);
  }



}
