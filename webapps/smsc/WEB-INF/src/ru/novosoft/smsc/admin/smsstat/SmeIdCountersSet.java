/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

import java.util.Vector;
import java.util.Collection;

public class SmeIdCountersSet extends CountersSet
{
  public String smeid;
  private Vector errors = new Vector(); // contains ErrorCounterSet

  public SmeIdCountersSet(String smeid)
  {
    this.smeid = smeid;
  }

  public SmeIdCountersSet(int accepted, int rejected, int delivered,
                          int failed, int rescheduled, int temporal, String smeid)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal);
    this.smeid = smeid;
  }

  public SmeIdCountersSet(CountersSet set, String smeid)
  {
    super(set);
    this.smeid = smeid;
  }

  public void addError(ErrorCounterSet set) {
    errors.addElement(set);
  }
  public Collection getErrors() {
    return errors;
  }

}
