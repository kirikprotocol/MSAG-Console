package ru.novosoft.smsc.admin.stat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.admin.filesystem.TestFileSystem;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import static org.junit.Assert.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmscStatProviderTest {

  private SmscStatProvider provider;
  private MemoryFileSystem fs = new MemoryFileSystem();

  private TestSmscStatContext context;

  private Collection<File> prepareFiles() throws IOException, AdminException {
    List<File> result = new ArrayList<File>(2);
    File result1 = fs.mkdirs("-smsc-stat1");
    File f1 = new File(result1, "2011-09");
    fs.mkdirs(f1);

    File result2 = fs.mkdirs("-smsc-stat2");
    File f2 = new File(result2, "2011-09");
    fs.mkdirs(f2);

    fs.createNewFile(new File(f1, "05.rts"), this.getClass().getResourceAsStream("05.rts"));
    fs.createNewFile(new File(f1, "06.rts"), this.getClass().getResourceAsStream("06.rts"));
    fs.createNewFile(new File(f1, "07.rts"), this.getClass().getResourceAsStream("07.rts"));
    fs.createNewFile(new File(f2, "05.rts"), this.getClass().getResourceAsStream("05.rts"));
    fs.createNewFile(new File(f2, "06.rts"), this.getClass().getResourceAsStream("06.rts"));
    fs.createNewFile(new File(f2, "07.rts"), this.getClass().getResourceAsStream("07.rts"));

    result.add(result1);
    result.add(result2);

    return result;
  }


  @Before
  public void init() throws IOException, AdminException {
    final Collection<File> dirs = prepareFiles();
    context = new TestSmscStatContext(dirs);
    provider = new SmscStatProvider(context, null);
  }


  private static SmscStatFilter getFilter() {
    return new SmscStatFilter();
  }


  @Test
  public void testOneInstance() throws AdminException {
    context.addExcludeDir(0);
    Statistics statistics = provider.getStatistics(getFilter());
    assertEquals(4, statistics.getGeneralStat().size());
    assertEquals(3, statistics.getSmeIdStat().size());
    assertEquals(2, statistics.getRouteIdStat().size());
    assertEquals(3, statistics.getErrors().size());
  }

  @Test
  public void testOneInstanceTotal() throws AdminException {
    context.addExcludeDir(0);
    Statistics statistics = provider.getStatistics(getFilter());
    assertEquals(280, statistics.getAccepted());
    assertEquals(0, statistics.getRejected());
    assertEquals(135, statistics.getDelivered());
    assertEquals(135, statistics.getFailed());
    assertEquals(1350, statistics.getRescheduled());
    assertEquals(1360, statistics.getTemporal());
    assertEquals(3, statistics.getPeak_i());
    assertEquals(2, statistics.getPeak_o());
  }

  @Test
  public void testOneInstanceErrors() throws AdminException {
    context.addExcludeDir(0);
    Statistics statistics = provider.getStatistics(getFilter());
    for(ErrorCounterSet s : statistics.getErrors()) {
      switch (s.getErrcode()) {
        case 0: assertEquals(135, s.getCounter()); break;
        case 1026: assertEquals(135, s.getCounter()); break;
        case 1028: assertEquals(1360, s.getCounter()); break;
        default: fail();
      }
    }
  }

  @Test
  public void testOneInstanceSmeId() throws AdminException {
    context.addExcludeDir(0);
    Statistics statistics = provider.getStatistics(getFilter());
    for(SmeIdCountersSet s : statistics.getSmeIdStat()) {
      if(s.getSmeid().equals("Informer")) {
        assertEquals(145, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(135, s.getDelivered());
        assertEquals(0, s.getFailed());
        assertEquals(0, s.getRescheduled());
        assertEquals(0, s.getTemporal());
        assertEquals(3, s.getPeak_i());
        assertEquals(2, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 0: assertEquals(e.getCounter(), 135) ;break;
            default: fail();
          }
        }
      }else if(s.getSmeid().endsWith("SILENT")) {
        assertEquals(0, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(0, s.getDelivered());
        assertEquals(135, s.getFailed());
        assertEquals(1350, s.getRescheduled());
        assertEquals(1360, s.getTemporal());
        assertEquals(0, s.getPeak_i());
        assertEquals(0, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 1026: assertEquals(e.getCounter(), 135) ;break;
            case 1028: assertEquals(e.getCounter(), 1360) ;break;
            default: fail();
          }
        }
      }else if(s.getSmeid().equals("smscsme")) {
        assertEquals(135, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(0, s.getDelivered());
        assertEquals(0, s.getFailed());
        assertEquals(0, s.getRescheduled());
        assertEquals(0, s.getTemporal());
        assertEquals(2, s.getPeak_i());
        assertEquals(0, s.getPeak_o());
        if(!s.getErrors().isEmpty()) {
          fail();
        }
      }else {
        fail();
      }
    }
  }

  @Test
  public void testOneInstanceRoutes() throws AdminException{
    context.addExcludeDir(0);
    Statistics statistics = provider.getStatistics(getFilter());
    for(RouteIdCountersSet s : statistics.getRouteIdStat()) {
      if(s.getRouteid().equals("Informer < smscsme")) {
        assertEquals(135, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(135, s.getDelivered());
        assertEquals(0, s.getFailed());
        assertEquals(0, s.getRescheduled());
        assertEquals(0, s.getTemporal());
        assertEquals(2, s.getPeak_i());
        assertEquals(2, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 0: assertEquals(e.getCounter(), 135) ;break;
            default: fail();
          }
        }
      }else if(s.getRouteid().equals("Informer > silent")) {
        assertEquals(145, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(0, s.getDelivered());
        assertEquals(135, s.getFailed());
        assertEquals(1350, s.getRescheduled());
        assertEquals(1360, s.getTemporal());
        assertEquals(3, s.getPeak_i());
        assertEquals(0, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 1026: assertEquals(e.getCounter(), 135) ;break;
            case 1028: assertEquals(e.getCounter(), 1360) ;break;
            default: fail();
          }
        }
      }else {
        fail();
      }
    }
  }

  @Test
  public void testOneInstanceOneDay() throws AdminException {
    context.addExcludeDir(0);
    Statistics _statistics = provider.getStatistics(getFilter());
    Collection<DateCountersSet> set =  _statistics.getGeneralStat();
    assertTrue(!set.isEmpty());
    DateCountersSet d = set.iterator().next();
    assertEquals(5, d.getAccepted());
    assertEquals(0, d.getRejected());
    assertEquals(0, d.getDelivered());
    assertEquals(0, d.getFailed());
    assertEquals(0, d.getRescheduled());
    assertEquals(5, d.getTemporal());
    assertEquals(3, d.getPeak_i());
    assertEquals(0, d.getPeak_o());
  }


  @Test
  public void testTwoInstance() throws AdminException {
    Statistics statistics = provider.getStatistics(getFilter());
    assertEquals(4, statistics.getGeneralStat().size());
    assertEquals(3, statistics.getSmeIdStat().size());
    assertEquals(2, statistics.getRouteIdStat().size());
    assertEquals(3, statistics.getErrors().size());
  }

  @Test
  public void testTwoInstanceTotal() throws AdminException {
    Statistics statistics = provider.getStatistics(getFilter());
    assertEquals(560, statistics.getAccepted());
    assertEquals(0, statistics.getRejected());
    assertEquals(270, statistics.getDelivered());
    assertEquals(270, statistics.getFailed());
    assertEquals(2700, statistics.getRescheduled());
    assertEquals(2720, statistics.getTemporal());
    assertEquals(3, statistics.getPeak_i());
    assertEquals(2, statistics.getPeak_o());
  }

  @Test
  public void testTwoInstanceErrors() throws AdminException {
    Statistics statistics = provider.getStatistics(getFilter());
    for(ErrorCounterSet s : statistics.getErrors()) {
      switch (s.getErrcode()) {
        case 0: assertEquals(270, s.getCounter()); break;
        case 1026: assertEquals(270, s.getCounter()); break;
        case 1028: assertEquals(2720, s.getCounter()); break;
        default: fail();
      }
    }
  }

  @Test
  public void testTwoInstanceTwoDay() throws AdminException {
    Statistics _statistics = provider.getStatistics(getFilter());
    Collection<DateCountersSet> set =  _statistics.getGeneralStat();
    assertTrue(!set.isEmpty());
    DateCountersSet d = set.iterator().next();
    assertEquals(10, d.getAccepted());
    assertEquals(0, d.getRejected());
    assertEquals(0, d.getDelivered());
    assertEquals(0, d.getFailed());
    assertEquals(0, d.getRescheduled());
    assertEquals(10, d.getTemporal());
    assertEquals(3, d.getPeak_i());
    assertEquals(0, d.getPeak_o());
  }


  @Test
  public void testTwoInstanceSmeId() throws AdminException {
    Statistics statistics = provider.getStatistics(getFilter());
    for(SmeIdCountersSet s : statistics.getSmeIdStat()) {
      if(s.getSmeid().equals("Informer")) {
        assertEquals(290, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(270, s.getDelivered());
        assertEquals(0, s.getFailed());
        assertEquals(0, s.getRescheduled());
        assertEquals(0, s.getTemporal());
        assertEquals(3, s.getPeak_i());
        assertEquals(2, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 0: assertEquals(e.getCounter(), 270) ;break;
            default: fail();
          }
        }
      }else if(s.getSmeid().endsWith("SILENT")) {
        assertEquals(0, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(0, s.getDelivered());
        assertEquals(270, s.getFailed());
        assertEquals(2700, s.getRescheduled());
        assertEquals(2720, s.getTemporal());
        assertEquals(0, s.getPeak_i());
        assertEquals(0, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 1026: assertEquals(e.getCounter(), 270) ;break;
            case 1028: assertEquals(e.getCounter(), 2720) ;break;
            default: fail();
          }
        }
      }else if(s.getSmeid().equals("smscsme")) {
        assertEquals(270, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(0, s.getDelivered());
        assertEquals(0, s.getFailed());
        assertEquals(0, s.getRescheduled());
        assertEquals(0, s.getTemporal());
        assertEquals(2, s.getPeak_i());
        assertEquals(0, s.getPeak_o());
        if(!s.getErrors().isEmpty()) {
          fail();
        }
      }else {
        fail();
      }
    }
  }
  @Test
  public void testTwoInstanceRoutes() throws AdminException{
    Statistics statistics = provider.getStatistics(getFilter());
    for(RouteIdCountersSet s : statistics.getRouteIdStat()) {
      if(s.getRouteid().equals("Informer < smscsme")) {
        assertEquals(270, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(270, s.getDelivered());
        assertEquals(0, s.getFailed());
        assertEquals(0, s.getRescheduled());
        assertEquals(0, s.getTemporal());
        assertEquals(2, s.getPeak_i());
        assertEquals(2, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 0: assertEquals(e.getCounter(), 270) ;break;
            default: fail();
          }
        }
      }else if(s.getRouteid().equals("Informer > silent")) {
        assertEquals(290, s.getAccepted());
        assertEquals(0, s.getRejected());
        assertEquals(0, s.getDelivered());
        assertEquals(270, s.getFailed());
        assertEquals(2700, s.getRescheduled());
        assertEquals(2720, s.getTemporal());
        assertEquals(3, s.getPeak_i());
        assertEquals(0, s.getPeak_o());
        for(ErrorCounterSet e : s.getErrors()) {
          switch (e.getErrcode()) {
            case 1026: assertEquals(e.getCounter(), 270) ;break;
            case 1028: assertEquals(e.getCounter(), 2720) ;break;
            default: fail();
          }
        }
      }else {
        fail();
      }
    }
  }



  private class TestSmscStatContext implements SmscStatContext {

    private final List<File> dirs = new LinkedList<File>();

    private Collection<Integer> excludeDir = new LinkedList<Integer>();

    private TestSmscStatContext(Collection<File> dirs) {
      if(dirs != null) {
        this.dirs.addAll(dirs);
      }
    }

    public File[] getStatDirs() {
      List<File> r = new ArrayList<File>(dirs);
      for(Integer i : excludeDir) {
        r.remove(i.intValue());
      }
      return r.toArray(new File[r.size()]);
    }

    public FileSystem getFileSystem() {
      return fs;
    }

    private void addExcludeDir(int num) {
      excludeDir.add(num);
    }

    private void clearExcludeDirs() {
      excludeDir.clear();
    }
  }
}
