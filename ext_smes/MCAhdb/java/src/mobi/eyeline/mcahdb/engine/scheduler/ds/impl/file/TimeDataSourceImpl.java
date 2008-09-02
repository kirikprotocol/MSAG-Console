package mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file;

import mobi.eyeline.mcahdb.engine.scheduler.ds.TimeDataSource;
import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.Date;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Lock;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.*;
import java.nio.channels.FileChannel;

import org.apache.log4j.Category;
import com.eyeline.utils.FileUtils;

/**
 * User: artem
 * Date: 01.09.2008
 */

public class TimeDataSourceImpl implements TimeDataSource {

  private static final Category log = Category.getInstance(TimeDataSourceImpl.class);
  private static final SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss");

  private RandomAccessFile f;
  private Date time;
  private final Lock lock = new ReentrantLock();

  public TimeDataSourceImpl(File storeDir) throws DataSourceException {
    try {
      if (!storeDir.exists() && !storeDir.mkdirs())
        throw new DataSourceException("Can't create dir: " + storeDir.getAbsolutePath());
      File file = new File(storeDir, "scheduler.time");
      if (FileUtils.truncateFile(file, '\n', 5))
        log.warn("File " + file + " was repaired.");
      this.f = new RandomAccessFile(file, "rw");
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  public void setTime(Date time) throws DataSourceException {
    try {
      lock.lock();
      if (this.time != null && this.time.equals(time))
        return;
      f.seek(f.length());
      f.write(df.format(time).getBytes());
      f.write('\n');
      this.time = new Date(time.getTime());
    } catch (IOException e) {
      throw new DataSourceException(e);
    } finally {
      lock.unlock();
    }
  }

  public Date getTime() throws DataSourceException {
    try {
      lock.lock();
      if (time != null)
        return time;

      if (f.length() < df.toPattern().length())
        return null;

      f.seek(f.length() - df.toPattern().length() - 1);      
      time = df.parse(f.readLine());

      return time;

    } catch (IOException e) {
      throw new DataSourceException(e);
    } catch (ParseException e) {
      throw new DataSourceException(e);
    } finally {
      lock.unlock();
    }
  }

  public void close() {
    if (f != null)
      try {
        f.close();
      } catch (IOException e) {
      }
  }
}
