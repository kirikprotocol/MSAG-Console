package subscription;

import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.AfterClass;
import mobi.eyeline.smsquiz.subscription.datasource.impl.DBSubscriptionDataSource;
import mobi.eyeline.smsquiz.subscription.Subscription;
import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.util.Date;


public class DBDataSourceTester {

  private static DBSubscriptionDataSource dataSource;
  private String addressOff = "+791394899off";
  private String addressOn = "+7913948999on";


  @BeforeClass
  public static void initPool() {
    try {
      ConnectionPoolFactory.init("conf/config.xml");
      dataSource = new DBSubscriptionDataSource();
    } catch (StorageException e) {
      e.printStackTrace();
      assertTrue(false);
    }
  }

  //@Ignore
  @Test
  public void addSubscription() {
    try {
      dataSource.remove(addressOff);
    } catch (StorageException e) {
    }
    try {
      dataSource.remove(addressOn);
    } catch (StorageException e) {
    }

    try {
      Subscription subscription = new Subscription();
      subscription.setAddress(addressOn);
      subscription.setStartDate(new Date());
      subscription.setEndDate(null);
      dataSource.save(subscription);
      subscription.setAddress(addressOff);
      subscription.setStartDate(new Date());
      subscription.setEndDate(new Date());
      dataSource.save(subscription);
    } catch (StorageException e) {
      e.printStackTrace();
      assertTrue(false);
    }
  }

  @AfterClass
  public static void close() {
    dataSource.close();
  }

  //  @Ignore
  @Test
  public void getByAddress() {
    try {
      assertNotNull(dataSource.get(addressOff));
      assertNotNull(dataSource.get(addressOn));
    } catch (StorageException e) {
      e.printStackTrace();
      assertTrue(false);
    }
  }


  @Test
  public void list() {
    try {
      ResultSet result = dataSource.list(new Date());
      while (result.next()) {
        System.out.println(result.get());
      }
      result.close();
    } catch (StorageException e) {
      e.printStackTrace();
      assertTrue(false);
    }
  }

}
