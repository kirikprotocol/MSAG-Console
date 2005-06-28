package ru.sibinco.scag.backend.stat.stat;

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

  public ExtendedCountersSet() {}
  public ExtendedCountersSet(long accepted, long rejected, long delivered, long tempError, long permError)
  {
    super(accepted, rejected, delivered, tempError, permError);
  }

  public ExtendedCountersSet(long accepted, long rejected, long delivered, long tempError, long permError,
                             long smsTrOk, long smsTrFailed, long smsTrBilled,
                             long ussdTrFromScOk,  long ussdTrFromScFailed, long ussdTrFromScBilled,
                             long ussdTrFromSmeOk, long ussdTrFromSmeFailed, long ussdTrFromSmeBilled)
  {
    super(accepted, rejected, delivered, tempError, permError,
          smsTrOk, smsTrFailed, smsTrBilled,
          ussdTrFromScOk, ussdTrFromScFailed, ussdTrFromScBilled,
          ussdTrFromSmeOk, ussdTrFromSmeFailed, ussdTrFromSmeBilled);
  }

  public void addError(ErrorCounterSet set) {
    errors.addElement(set);
  }
  public Collection getErrors() {
    return errors;
  }
}
