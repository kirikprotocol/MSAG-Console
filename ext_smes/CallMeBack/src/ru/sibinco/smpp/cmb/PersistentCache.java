package ru.sibinco.smpp.cmb;

import ru.sibinco.smpp.util.Utils;

import java.io.*;
import java.util.Calendar;
import java.util.Properties;


public class PersistentCache implements Cache {

  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(PersistentCache.class);

  private String cacheFile = "cmb_cache" + System.getProperty("fie.separator") + "cache.dat";
  private boolean cacheChanged = false;
  private long cacheSavindFrequency = 300000L;
  private char resetFrequency = 'D';

  private static final int cacheSize = 9999999;
  private short[] cache = new short[cacheSize];

  private long rdate;

  private static PersistentCache instance = null;

  public static PersistentCache getInstance() {
    if (instance == null)
      instance = new PersistentCache();
    return instance;
  }

  private PersistentCache() {
    rdate = getRdate(-1);
    init();
    load();
    new CacheController(cacheSavindFrequency).startService();
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
    cacheFile = config.getProperty("cache.file", cacheFile);
    if (Logger.isInfoEnabled()) Logger.info("cache.file=" + cacheFile);
    cacheSavindFrequency = Utils.getLongProperty(config, "cache.saving.frequency", cacheSavindFrequency);
    if (Logger.isInfoEnabled()) Logger.info("cache.saving.frequency=" + cacheSavindFrequency);
    resetFrequency = config.getProperty("reset.frequency", "D").toUpperCase().charAt(0);
    if (Logger.isInfoEnabled()) Logger.info("reset.frequency=" + resetFrequency);
  }

  public synchronized short getCache(int index) throws IndexOutOfBoundsException {
    if (rdate == getRdate(-1))
      return cache[index];
    else {
      if (Logger.isDebugEnabled())
        Logger.debug("Reset cache, reason - expired.");
      cache = new short[cacheSize];
      rdate = getRdate(-1);
      return 0;
    }
  }

  public synchronized void setCache(int index, short data) throws IndexOutOfBoundsException {
    if (!cacheChanged) cacheChanged = true;
    cache[index] = data;
  }

  private long getRdate(long timeInMillis) {
    Calendar c = Calendar.getInstance();
    if (timeInMillis != -1) {
      c.setTimeInMillis(timeInMillis);
    }
    if (resetFrequency == 'M')
      return c.get(Calendar.YEAR) * 100000000 + (c.get(Calendar.MONTH) + 1) * 1000000 + c.get(Calendar.DAY_OF_MONTH)*10000+c.get(Calendar.HOUR_OF_DAY)*100+c.get(Calendar.MINUTE);
    else if (resetFrequency == 'H')
      return c.get(Calendar.YEAR) * 1000000 + (c.get(Calendar.MONTH) + 1) * 10000 + c.get(Calendar.DAY_OF_MONTH)*100+c.get(Calendar.HOUR_OF_DAY);
    else
      return c.get(Calendar.YEAR) * 10000 + (c.get(Calendar.MONTH) + 1) * 100 + c.get(Calendar.DAY_OF_MONTH);
  }

  private void save() {
    if (!cacheChanged) {
      if (Logger.isDebugEnabled())
        Logger.debug("Could not find cache changes to save.");
      return;
    }
    long start = System.currentTimeMillis();
    BufferedOutputStream out = null;
    try {
      try {
        out = new BufferedOutputStream(new FileOutputStream(cacheFile), 16 * 1024);
      } catch (FileNotFoundException e) {
        Logger.error("Could not open cache file " + cacheFile, e);
        return;
      }
      int i;
      int index = 0;
      while (index < cacheSize) {
        int bufferLength = index + 1024 * 8 <= cacheSize ? 1024 * 8 : cacheSize - index;
        byte[] data = new byte[bufferLength * 2];
        int dataIndex = 0;
        synchronized (cache) {
          for (i = index; i < index+bufferLength; i++) {
            data[dataIndex++] = (byte) (cache[i]>>8);
            data[dataIndex++] = (byte) cache[i];
          }
        }
        index += bufferLength;
        try {
          out.write(data, 0, data.length);
          out.flush();
        } catch (IOException e) {
          Logger.error("Could not save data buffer into cache file.", e);
        }
      }
      if (Logger.isInfoEnabled())
        Logger.info("Cache saved (" + (System.currentTimeMillis() - start) + " ms).");
    } finally {
      if (out != null) {
        try {
          out.close();
          cacheChanged = false;
        } catch (IOException e) {
          Logger.error("Could not close cache file.", e);
        }
      }
    }
  }

  private void load() {
    File f = new File(cacheFile);
    if (!f.exists()) {
      if (Logger.isDebugEnabled())
        Logger.debug("Could not find cache file to load.");
      return;
    }
    if (getRdate(-1) != getRdate(f.lastModified())) {
      if (Logger.isDebugEnabled())
        Logger.debug("Ignore expired cache file.");
      return;
    }
    long start = System.currentTimeMillis();
    BufferedInputStream in = null;
    try {
      try {
        in = new BufferedInputStream(new FileInputStream(cacheFile), 16 * 1024);
      } catch (FileNotFoundException e) {
        Logger.error("Could not open cache file " + cacheFile, e);
        return;
      }
      int index = 0;
      byte[] buffer = new byte[16 * 1024];
      while (index < cacheSize) {
        try {
          int readed = in.read(buffer, 0, buffer.length);
          int bufferIndex = 0;
          synchronized (cache) {
            for (int i = index; i < index+(readed/2); i++) {
              cache[i] = (short) (buffer[bufferIndex++] << 8 | buffer[bufferIndex++]);
            }
          }
          if (readed < buffer.length)
            break;
          index += readed/2;
        } catch (Exception e) {
          Logger.warn("Could not read data buffer from cache file.", e);
        }
      }
      if (Logger.isInfoEnabled())
        Logger.info("Cache loaded (" + (System.currentTimeMillis() - start) + " ms).");
    } finally {
      if (in != null) {
        try {
          in.close();
        } catch (IOException e) {
          Logger.error("Could not close cache file.", e);
        }
      }
    }
  }

  class CacheController extends Thread {

    private boolean started = false;
    private long pollingInterval = 60000L;
    private Object monitor = new Object();


    public CacheController(long pollingInterval) {
      setName("PersistentCacheController");
      this.pollingInterval = pollingInterval;
    }

    public void startService() {
      started = true;
      start();
      if (Logger.isInfoEnabled())
        Logger.info(getName()+" started.");
    }

    public void stopService() {
      synchronized (monitor) {
        started = false;
        monitor.notifyAll();
      }
      if (Logger.isInfoEnabled())
        Logger.info(getName()+" stopped.");
    }

    public boolean isStarted() {
      return started;
    }

    public void run() {
      while (true) {
        save();
        synchronized (monitor) {
          if (!started) break;
          try {
            monitor.wait(pollingInterval);
          } catch (InterruptedException e) {
            if (Logger.isDebugEnabled()) Logger.debug(getName()+" interrupted.", e);
          }
        }
      }
    }
  }
}
