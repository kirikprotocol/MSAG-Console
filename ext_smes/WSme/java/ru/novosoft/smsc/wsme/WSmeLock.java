/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 16, 2003
 * Time: 5:45:11 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

public class WSmeLock
{
  private int readers = 0;
  private int writer = 0;

  public WSmeLock() {
  }

  public synchronized void acquireRead()
    throws InterruptedException
  {
     while( writer != 0 ) wait();
     readers++;
  }

  public synchronized void acquireWrite()
      throws InterruptedException
  {
     while( !(writer == 0 && readers <= 1) ) wait();
     readers = 1; // a writer is a reader
     writer = 1;
  }

  public synchronized boolean release()
  {
     --readers;
     writer = 0;
     notifyAll();
     return readers == 0;
  }

}
