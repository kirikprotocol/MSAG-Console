package ru.sibinco.smpp.cmb;

import java.util.Calendar;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 26, 2006
 * Time: 2:19:24 PM
 */
public class MemoryCache implements Cache {

  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(MemoryCache.class);

  private char resetFrequency = 'D';

  private static final int cacheSize = 9999999;
  private short[] cache = new short[cacheSize];

  private long rdate;

  private static MemoryCache instance = null;

  public static MemoryCache getInstance() {
    if (instance == null)
      instance = new MemoryCache();
    return instance;
  }

  private MemoryCache() {
    rdate = getRdate();
    init();
    if (Logger.isInfoEnabled()) Logger.info("Ready.");
  }

  private void init() {
    String configFile = this.getClass().getName().substring(this.getClass().getName().lastIndexOf('.') + 1) + ".properties";
    Properties config = new Properties();
    try {
      config.load(this.getClass().getClassLoader().getResourceAsStream(configFile));
      if (Logger.isInfoEnabled()) Logger.info("Config from " + configFile + " loaded.");
    } catch (Exception e) {
      Logger.warn("Could not load config from " + configFile, e);
    }
    resetFrequency = config.getProperty("reset.frequency", "D").toUpperCase().charAt(0);
    if (Logger.isInfoEnabled()) Logger.info("reset.frequency=" + resetFrequency);
  }

  public synchronized short getCache(int index) throws IndexOutOfBoundsException {
    if (rdate == getRdate())
      return cache[index];
    else {
      if (Logger.isDebugEnabled())
        Logger.debug("Reset cache, reason - expired.");
      cache = new short[cacheSize];
      rdate = getRdate();
      return 0;
    }
  }

  public synchronized void setCache(int index, short data) throws IndexOutOfBoundsException {
    cache[index] = data;
  }

  private long getRdate() {
    Calendar c = Calendar.getInstance();
    if (resetFrequency == 'M')
      return c.get(Calendar.YEAR) * 100000000 + (c.get(Calendar.MONTH) + 1) * 1000000 + c.get(Calendar.DAY_OF_MONTH)*10000+c.get(Calendar.HOUR_OF_DAY)*100+c.get(Calendar.MINUTE);
    else if (resetFrequency == 'H')
      return c.get(Calendar.YEAR) * 1000000 + (c.get(Calendar.MONTH) + 1) * 10000 + c.get(Calendar.DAY_OF_MONTH)*100+c.get(Calendar.HOUR_OF_DAY);
    else
      return c.get(Calendar.YEAR) * 10000 + (c.get(Calendar.MONTH) + 1) * 100 + c.get(Calendar.DAY_OF_MONTH);
  }
}
