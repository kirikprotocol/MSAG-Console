package mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page;

import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.LinkedList;

import mobi.eyeline.mcahdb.engine.scheduler.ds.Task;
import mobi.eyeline.mcahdb.engine.DataSourceException;

/**
 * User: artem
 * Date: 03.09.2008
 */

public class SchedulePageImplTest {

  @Test
  public void addTest() throws IOException, DataSourceException {
    File f = new File("test.csv");
    f.delete();

    SchedulePageImpl impl = new SchedulePageImpl(f , "test.csv");
    ArrayList<Task> tasks = new ArrayList<Task>(100000);
    for (int i=0; i<99999; i++) {
      Task t = new Task("+79139023974", "+79607891901");
      t.setTime(new Date());
      tasks.add(t);
    }

    try {
      impl.open();
      long start = System.nanoTime();
      for (int i=0; i<tasks.size(); i++)
        impl.add(tasks.get(i));
      System.out.println(System.nanoTime() - start);
    } finally {
      impl.close();
    }
  }

  @Test
  public void listTest() throws DataSourceException, IOException {
    SchedulePageImpl impl = new SchedulePageImpl(new File("test.csv"), "test.csv");

    try {
      impl.open();
      LinkedList<Task> result = new LinkedList<Task>();
      long start = System.nanoTime();
      impl.list(new Date(0), new Date(Long.MAX_VALUE), result);
      System.out.println(System.nanoTime() - start);
    } finally {
      impl.close();
    }
  }
}
