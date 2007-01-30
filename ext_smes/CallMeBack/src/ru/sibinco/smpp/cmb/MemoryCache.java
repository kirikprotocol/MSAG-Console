package ru.sibinco.smpp.cmb;

import java.util.Calendar;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 26, 2006
 * Time: 2:19:24 PM
 */
public class MemoryCache {

  private short[] cache = new short[9999999];

  private long rdate;

  private static MemoryCache instance = null;

  public static MemoryCache getInstance() {
    return getInstance(null);
  }

  protected static MemoryCache getInstance(String filePath) {
    if (instance == null)
      instance = new MemoryCache();
    if (filePath != null)
      instance.load(filePath);
    return instance;
  }

  private MemoryCache() {
    Calendar c = Calendar.getInstance();
    rdate = c.get(Calendar.YEAR)*10000+(c.get(Calendar.MONTH)+1)*100+c.get(Calendar.DAY_OF_MONTH);
  }

  public synchronized short getCache(int index) throws IndexOutOfBoundsException {
    if (rdate == getRdate())
      return cache[index];
    else {
      cache = new short[9999999];
      return 0;
    }
  }

  public synchronized void setCache(int index, short data) throws IndexOutOfBoundsException {
    cache[index] = data;
  }

  private long getRdate() {
    Calendar c = Calendar.getInstance();
    return c.get(Calendar.YEAR)*10000+(c.get(Calendar.MONTH)+1)*100+c.get(Calendar.DAY_OF_MONTH);
  }

  protected void dump(String filePath) {
    // todo
  }

  protected void load(String filePath) {
    // todo
  }
}
