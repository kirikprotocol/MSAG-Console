package mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page;

import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page.SchedulePage;
import mobi.eyeline.mcahdb.engine.scheduler.ds.Task;

import java.util.Collection;
import java.util.Date;
import java.util.StringTokenizer;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.apache.log4j.Category;
import com.eyeline.utils.FileUtils;

/**
 * User: artem
 * Date: 27.08.2008
 */

class SchedulePageImpl implements SchedulePage {

  private static final Category log = Category.getInstance(SchedulePageImpl.class);

  private final SimpleDateFormat df = new SimpleDateFormat("yyMMddHHmmss");

  private RandomAccessFile f;
  private final File file;
  private final String id;

  public SchedulePageImpl(File file, String id) throws IOException {
    if (FileUtils.truncateFile(file, '\n', 20))
      log.warn("File " + file + " was repaired.");
    this.id = id;
    this.file = file;
  }

  public void open() throws DataSourceException {
    try {
      if (f == null)
        this.f = new RandomAccessFile(file, "rw");
    } catch (FileNotFoundException e) {
      throw new DataSourceException(e);
    }
  }

  public String getId() {
    return id;
  }

  public TaskPointer add(Task task) throws DataSourceException {
    try {
      long id = f.length();
      f.seek(id);
      writeLine(f, "0,", df.format(task.getTime()), ",", task.getCaller(), ",", task.getCalled(), ",", String.valueOf(task.getType()));
      return new TaskPointer(getId(), id);
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  public Task remove(long taskId) throws DataSourceException {
    try {
      f.seek(taskId);
      f.write('1');

      return readTask(readLine(f), taskId);
    } catch (IOException e) {
      throw new DataSourceException(e);
    } catch (ParseException e) {
      throw new DataSourceException(e);
    }
  }

  private Task readTask(String str, long pointer) throws ParseException {
    int i, k;
    i = str.indexOf(',', 1);
    Date time = df.parse(str.substring(1, i));

    k = i + 1;
    i = str.indexOf(',', k);
    String caller = str.substring(k,i).trim();

    k = i + 1;
    i = str.indexOf(',', k);
    String called = str.substring(k,i).trim();

    k = i + 1;
    int type = Integer.parseInt(str.substring(k));

//    StringTokenizer st = new StringTokenizer(str.substring(1), ",");
//    Date time = df.parse(st.nextToken());
//    String caller = st.nextToken().trim();
//    String called = st.nextToken().trim();
//    int type = Integer.parseInt(st.nextToken());

    Task t = new Task(caller, called);
    t.setTime(time);
    t.setType(type);
    t.setId(new TaskPointer(id, pointer));
    return t;
  }

  public Task get(long taskId) throws DataSourceException {
    try {
      f.seek(taskId);
      String line = readLine(f);
      if (line.charAt(0) == '1')
        return null;

      return readTask(line, taskId);

    } catch (IOException e) {
      throw new DataSourceException(e);
    } catch (ParseException e) {
      throw new DataSourceException(e);
    }
  }

  public void list(Date from, Date till, Collection<Task> result) throws DataSourceException {
    BufferedReader r = null;
    try {
      r = new BufferedReader(new FileReader(file));

      String line;
      long pointer = 0;
      while((line = r.readLine()) != null) {
        if (line.charAt(0) == '0') {
          Task t = readTask(line, pointer);
          if (!t.getTime().before(from) && !t.getTime().after(till))
            result.add(t);
        }
        pointer += line.length() + 1;
      }
    } catch (Exception e) {
      throw new DataSourceException(e);
    } finally {
      if (r != null)
        try {
          r.close();
        } catch (IOException e) {
        }
    }
  }

  private static void writeLine(RandomAccessFile f, String... strs) throws IOException {
    int len = 0;
    for (String str : strs) len += str.length();
    byte[] bytes = new byte[len + 1];
    int k=0;
    for (String str : strs)
      for (int j=0; j<str.length(); j++)
        bytes[k++] = (byte) str.charAt(j);
    bytes[len] = '\n';
    f.write(bytes);
  }

  private static String readLine(RandomAccessFile f) throws IOException {
    int len = (int)Math.min(100, f.length() - f.getFilePointer());
    byte[] bytes = new byte[len];
    f.readFully(bytes);
    StringBuilder sb = new StringBuilder(len);
    for (byte b : bytes) {
      if (b == '\n')
        break;
      sb.append((char)b);
    }
    return sb.toString();
  }

  public void close() {
    try {
      if (f != null)
        f.close();
      f = null;
    } catch (IOException e) {
    }
  }
}
