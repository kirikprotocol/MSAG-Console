package mobi.eyeline.informer.admin.monitoring;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.Date;

import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class MonitoringFileJournalTest {

  private MonitoringFileJournal dataSource;

  private File dir;

  @Before
  public void before() throws AdminException {
    dir = TestUtils.createRandomDir("monitoring");
    dataSource = new MonitoringFileJournal(dir, new TestFileSystem());
  }


  private static MonitoringEvent createEvent() {
    MonitoringEvent e = new MonitoringEvent("alarmId");
    e.setTime(System.currentTimeMillis());
    e.setSeverity(MonitoringEvent.Severity.MINOR);
    e.setText("text-text");
    e.setSource(MBean.Source.DCP);
    e.setProperty("prop1", "value1");
    e.setProperty("prop2", "value2");
    return e;
  }



  @Test
  public void addTest() throws AdminException {
    MonitoringEvent e = createEvent();
    dataSource.addEvents(e);
  }

  @Test
  public void addVisitCheckTest() throws AdminException {
    final MonitoringEvent e = createEvent();
    dataSource.addEvents(e);

    final boolean[] equals = new boolean[]{false};
    dataSource.visit(new MonitoringEventsFilter(), new MonitoringJournal.Visitor() {
      @Override
      public boolean visit(MonitoringEvent e1) throws AdminException {
        if(e.equals(e1)) {
          equals[0] = true;
        }
        return false;
      }
    });

    assertTrue("Such event is not found", equals[0]);
  }

  @Test
  public void checkFilterSourceIdTest() throws AdminException {
    final MonitoringEvent e = createEvent();
    dataSource.addEvents(e);

    MonitoringEventsFilter eF = new MonitoringEventsFilter();
    eF.setSource(MBean.Source.DCP);

    final boolean[] found = new boolean[]{false};
    dataSource.visit(eF, new MonitoringJournal.Visitor() {
      @Override
      public boolean visit(MonitoringEvent e1) throws AdminException {
          found[0] = true;
        return false;
      }
    });

    assertTrue("Such event is not found", found[0]);

    eF = new MonitoringEventsFilter();
    eF.setSource(MBean.Source.SIEBEL);

    found[0] = false;
    dataSource.visit(eF, new MonitoringJournal.Visitor() {
      @Override
      public boolean visit(MonitoringEvent e1) throws AdminException {
        return false;
      }
    });

    assertTrue("MonitoringEvent is found", !found[0]);
  }

  @Test
  public void checkFilterStartDateTest() throws AdminException {
    final MonitoringEvent e = createEvent();
    dataSource.addEvents(e);

    MonitoringEventsFilter eF = new MonitoringEventsFilter();
    eF.setStartDate(new Date(System.currentTimeMillis() - 60000));

    final boolean[] found = new boolean[]{false};
    dataSource.visit(eF, new MonitoringJournal.Visitor() {
      @Override
      public boolean visit(MonitoringEvent e1) throws AdminException {
          found[0] = true;
        return false;
      }
    });

    assertTrue("Such event is not found", found[0]);

    eF = new MonitoringEventsFilter();
    eF.setStartDate(new Date(System.currentTimeMillis() + 60000));

    found[0] = false;
    dataSource.visit(eF, new MonitoringJournal.Visitor() {
      @Override
      public boolean visit(MonitoringEvent e1) throws AdminException {
        return false;
      }
    });

    assertTrue("MonitoringEvent is found", !found[0]);
  }

  @Test
  public void checkFilterEndDateTest() throws AdminException {
    final MonitoringEvent e = createEvent();
    dataSource.addEvents(e);

    MonitoringEventsFilter eF = new MonitoringEventsFilter();
    eF.setEndDate(new Date(System.currentTimeMillis() + 60000));

    final boolean[] found = new boolean[]{false};
    dataSource.visit(eF, new MonitoringJournal.Visitor() {
      @Override
      public boolean visit(MonitoringEvent e1) throws AdminException {
          found[0] = true;
        return false;
      }
    });

    assertTrue("Such event is not found", found[0]);

    eF = new MonitoringEventsFilter();
    eF.setEndDate(new Date(System.currentTimeMillis() - 60000));

    found[0] = false;
    dataSource.visit(eF, new MonitoringJournal.Visitor() {
      @Override
      public boolean visit(MonitoringEvent e1) throws AdminException {
        return false;
      }
    });

    assertTrue("MonitoringEvent is found", !found[0]);
  }


  @After
  public void after() {
    if(dir != null) {
      TestUtils.recursiveDeleteFolder(dir);
    }
  }


}
