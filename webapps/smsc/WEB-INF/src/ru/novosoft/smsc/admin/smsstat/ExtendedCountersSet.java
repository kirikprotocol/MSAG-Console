package ru.novosoft.smsc.admin.smsstat;

import java.util.ArrayList;
import java.util.Collection;

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

  public ExtendedCountersSet()
  {
  }

  public ExtendedCountersSet(int accepted, int rejected, int delivered,
                             int failed, int rescheduled, int temporal, int i, int o)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
  }

  public void addError(ErrorCounterSet set)
  {
    errors.add(set);
  }

  public void addAllErrors(Collection err)
  {
    errors.addAll(err);
  }

  public Collection getErrors()
  {
    return errors;
  }

}
