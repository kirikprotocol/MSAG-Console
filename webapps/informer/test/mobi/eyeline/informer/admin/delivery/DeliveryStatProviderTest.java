package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import static org.junit.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.10.2010
 * Time: 12:23:57
 */
public class DeliveryStatProviderTest {

  DeliveryStatProvider statProvider;

  @Before
  public void before() throws Exception {
    URL u = TestDeliveryStatProvider.class.getResource("./");
    File statDir = new File(u.toURI());
    statDir  = new File(statDir,"stat");

    statProvider = new TestDeliveryStatProvider(statDir, FileSystem.getFSForSingleInst());
  }

  @Test
  public void providerTest() throws AdminException, IOException {
     final List<DeliveryStatRecord> records = new ArrayList<DeliveryStatRecord>();


     statProvider.accept(null,new DeliveryStatVisitor(){

       public boolean visit(DeliveryStatRecord rec, int total, int current) {
         assertTrue(total==2);
         assertTrue(current<total);
         records.add(rec);         
         return true;
       }
     });
     assertTrue(records.size()==8);

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


     records.clear();
     filter.setUser(null);
     filter.setTaskId(2);
     statProvider.accept(filter,new DeliveryStatVisitor(){
       public boolean visit(DeliveryStatRecord rec, int total, int current) {
         records.add(rec);
         return true;
       }
     });
     assertTrue(records.size()==2);

     records.clear();
     filter.setUser(null);
     filter.setTaskId(null);

     Calendar c1 = Calendar.getInstance();
     c1.set(2010,10-1,16,12,12);
     filter.setFromDate(c1.getTime());
     statProvider.accept(filter,new DeliveryStatVisitor(){
       public boolean visit(DeliveryStatRecord rec, int total, int current) {
         records.add(rec);
         return true;
       }
     });
     assertTrue(records.size()==8);

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
    assertTrue(records.size()==2);


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
    assertTrue(records.size()==1);
  }



}
