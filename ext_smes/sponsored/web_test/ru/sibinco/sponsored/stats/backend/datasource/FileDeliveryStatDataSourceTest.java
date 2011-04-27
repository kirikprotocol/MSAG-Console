package ru.sibinco.sponsored.stats.backend.datasource;

import junit.framework.TestCase;
import ru.sibinco.sponsored.stats.backend.TestUtils;

import java.io.File;
import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class FileDeliveryStatDataSourceTest extends TestCase {


  private File testDir;

  private FileDeliveryStatDataSource ds;

  private Date date;

  public void setUp() throws Exception {
    testDir = TestUtils.createTestDir("sponsor");
    ds = new FileDeliveryStatDataSource(testDir);
    date = new Date();
  }

  public void tearDown() throws Exception {
    if(testDir != null) {
      TestUtils.recursiveDirRemove(testDir);
    }
  }


  private void createArtefacts() throws Exception{
    DeliveryStat deliveryStat = new DeliveryStat();
    deliveryStat.setDelivered(3);
    deliveryStat.setSubscriberAddress("+79139489906");
    StatsFile file = null;
    try{
      file = ds.getFile(date, true);
      file.addStat(deliveryStat);
    }finally {
      if(file != null) {
        try{
          file.close();
        }catch (Exception ignored){}
      }
    }
  }

  public void testAggregateDeliveryStats() throws Exception {
    createArtefacts();
    ResultSet rs = null;
    try{
      rs = ds.aggregateDeliveryStats(1, new Date(0), new Date(), new DeliveryStatsQuery() {
        public boolean isAllowed(DeliveryStat stat) {
          return true;
        }
      }, new ProgressListener(){
        public void setProgress(int progress) {}
      }, new ShutdownIndicator());
      assertTrue(rs.next());
      DeliveryStat stat = (DeliveryStat)rs.get();
      assertFalse(rs.next());
      assertEquals(stat.getDelivered(), 3);
      assertEquals(stat.getSended(), 0);
      assertEquals(stat.getAdvertiserId(), 0);
      assertEquals(stat.getSubscriberAddress(), "+79139489906");
    }finally {
      if(rs != null) {
        try{
          rs.close();
        }catch (Exception ignored){}
      }
    }
  }
}
