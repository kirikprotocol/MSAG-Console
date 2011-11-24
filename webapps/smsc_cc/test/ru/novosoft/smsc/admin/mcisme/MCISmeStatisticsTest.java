package ru.novosoft.smsc.admin.mcisme;

import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;

import static org.junit.Assert.assertEquals;

/**
 * author: Aleksandr Khalitov
 */
public class MCISmeStatisticsTest {

  private MemoryFileSystem fileSystem;

  private File statspath;

  @Before
  public void before() throws Exception {
    fileSystem = new MemoryFileSystem();
    statspath  = new File("stats");
    File dir = new File(statspath, "2011-11");
    fileSystem.mkdirs(dir);
    InputStream in= null;
    try{
      in = this.getClass().getResourceAsStream("18.rts");
      fileSystem.createNewFile(new File(dir, "18.rts"), in);
    }finally {
      if(in != null) {
        try{
          in.close();
        }catch (IOException ignored){}
      }
    }
  }

  @Test
  public void testClosedDates() throws AdminException, ParseException {
    MCISmeStatistics statistics = new MCISmeStatistics(statspath, fileSystem);
    StatFilter filter = new StatFilter();
    SimpleDateFormat format = new SimpleDateFormat("yyyy.MM.dd HH:mm");
    filter.setFromDate(format.parse("2011.11.18 16:00"));
    filter.setTillDate(format.parse("2011.11.18 19:00"));
    MessageStatistics res = statistics.getStatistics(filter);
    assertEquals(res.getCountersByDates().size(), 1);
    assertEquals(res.getCountersByDates().iterator().next().getHourStat().size(), 2);
    assertEquals(res.getCounters().getDelivered(), 147738);
    assertEquals(res.getCounters().getFailed(), 16291);
    assertEquals(res.getCounters().getMissed(), 147377);
    assertEquals(res.getCounters().getNotified(), 147738);
  }

  @Test
  public void testEmptyFilter() throws AdminException {
    MCISmeStatistics statistics = new MCISmeStatistics(statspath, fileSystem);
    MessageStatistics res = statistics.getStatistics(null);
    assertEquals(res.getCountersByDates().size(), 1);
    assertEquals(res.getCountersByDates().iterator().next().getHourStat().size(), 2);
    assertEquals(res.getCounters().getDelivered(), 147738);
    assertEquals(res.getCounters().getFailed(), 16291);
    assertEquals(res.getCounters().getMissed(), 147377);
    assertEquals(res.getCounters().getNotified(), 147738);
  }

  @Test
  public void testEmptyResByFromDate() throws AdminException, ParseException {
    MCISmeStatistics statistics = new MCISmeStatistics(statspath, fileSystem);
    StatFilter filter = new StatFilter();
    SimpleDateFormat format = new SimpleDateFormat("yyyy.MM.dd HH:mm");
    filter.setFromDate(format.parse("2011.11.18 20:00"));
    MessageStatistics res = statistics.getStatistics(filter);
    assertEquals(1, res.getCountersByDates().size());
    assertEquals(0, res.getCounters().getDelivered());
    assertEquals(0, res.getCounters().getFailed());
    assertEquals(0, res.getCounters().getMissed());
    assertEquals(0, res.getCounters().getNotified());
  }

  @Test
  public void testEmptyResByTillDate() throws AdminException, ParseException {
    MCISmeStatistics statistics = new MCISmeStatistics(statspath, fileSystem);
    StatFilter filter = new StatFilter();
    SimpleDateFormat format = new SimpleDateFormat("yyyy.MM.dd HH:mm");
    filter.setTillDate(format.parse("2011.11.18 16:00"));
    MessageStatistics res = statistics.getStatistics(filter);
    assertEquals(1, res.getCountersByDates().size());
    assertEquals(0, res.getCounters().getDelivered());
    assertEquals(0, res.getCounters().getFailed());
    assertEquals(0, res.getCounters().getMissed());
    assertEquals(0, res.getCounters().getNotified());
  }


}
