package ru.sibinco.smppgw.backend.stat.stat;

import java.util.Vector;
import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:58:44
 * To change this template use File | Settings | File Templates.
 */
public class ExtendedCountersSet extends CountersSet
{
  private Vector errors = new Vector();

  public ExtendedCountersSet() {
  }

  public ExtendedCountersSet(int accepted, int rejected, int delivered,
                             int tempError, int permError) {
    super(accepted, rejected, delivered, tempError, permError);
  }

  public void addError(ErrorCounterSet set) {
    errors.addElement(set);
  }
  public Collection getErrors() {
    return errors;
  }
}
