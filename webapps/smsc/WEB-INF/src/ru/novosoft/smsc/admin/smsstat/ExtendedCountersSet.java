package ru.novosoft.smsc.admin.smsstat;

import java.util.Vector;
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
  private Vector errors = new Vector();

  public ExtendedCountersSet() {
  }

  public ExtendedCountersSet(int accepted, int rejected, int delivered,
                             int failed, int rescheduled, int temporal) {
    super(accepted, rejected, delivered, failed, rescheduled, temporal);
  }

  public void addError(ErrorCounterSet set) {
    errors.addElement(set);
  }
  public Collection getErrors() {
    return errors;
  }

}
