package ru.novosoft.smsc.mcisme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.04.2004
 * Time: 19:38:51
 * To change this template use Options | File Templates.
 */
public class RuntimeSet
{
  public long activeTasks = 0;
  public long inQueueSize  = 0;
  public long outQueueSize = 0;
  public long inSpeed  = 0;
  public long outSpeed = 0;

  public RuntimeSet() {};
  public RuntimeSet(long activeTasks, long inQueueSize, long outQueueSize, long inSpeed, long outSpeed)
  {
    this.activeTasks = activeTasks;
    this.inQueueSize = inQueueSize;
    this.outQueueSize = outQueueSize;
    this.inSpeed = inSpeed;
    this.outSpeed = outSpeed;
  }
}
