package ru.novosoft.smsc.admin.smsstat;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 24.10.2003
 * Time: 13:27:41
 * To change this template use Options | File Templates.
 */
public class ExtendedCountersSet extends CountersSet
{
  private ArrayList errors = new ArrayList();
  private HashMap err=new HashMap();
  public ExtendedCountersSet()
  {
  }

  public ExtendedCountersSet(long accepted, long rejected, long delivered,
                             long failed, long rescheduled, long temporal, long i, long o)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
  }

  public void addError(ErrorCounterSet set)
  {
    errors.add(set);
  }
  public void putErr(int errcode ,ErrorCounterSet set)
    {
      err.put(new Integer(errcode),set);
    }

  public void addAllErrors(Collection err)
  {
    errors.addAll(err);
  }

  public Collection getErrors()
  {
    return err.values();
  }
    public ErrorCounterSet getErr(int errcode)
  {
    return (ErrorCounterSet)err.get(new Integer(errcode));
  }
}
