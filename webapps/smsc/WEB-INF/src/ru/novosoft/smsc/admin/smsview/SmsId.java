package ru.novosoft.smsc.admin.smsview;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 20.04.2005
 * Time: 17:49:33
 * To change this template use File | Settings | File Templates.
 */
public class SmsId
{
   private long pointer=0;
   private long id;
   private long counter;

  public long getId()
  {
    return id;
  }

  public void setId(long id)
  {
    this.id = id;
  }

  public long getPointer()
  {
    return pointer;
  }

  public void setPointer(long pointer)
  {
    this.pointer = pointer;
  }

  public long getCounter()
  {
    return counter;
  }

  public void setCounter(long counter)
  {
    this.counter = counter;
  }
}
