package ru.sibinco.smppgw.backend.stat.stat;


import java.util.Collection;
import java.util.TreeMap;
//import java.util.Vector;
/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:58:44
 * To change this template use File | Settings | File Templates.
 */
public class ExtendedCountersSet extends CountersSet
{
  //private Vector errors = new Vector();
  private TreeMap err=new TreeMap();
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
   public void incError(int errcode, long count)
    {
        Integer key = new Integer(errcode);
        ErrorCounterSet set = (ErrorCounterSet)err.get(key);
        if (set == null) err.put(key, new ErrorCounterSet(errcode, count));
        else set.increment(count);
    }
    
   public void putErr(int errcode, ErrorCounterSet set)
    {
        err.put(new Integer(errcode), set);
    }
    public void addAllErr(Collection err)
    {
        err.addAll(err);
    }
    public Collection getErrors()
    {
        return err.values();
    }
    public TreeMap getErrorsMap()
    {
        return err;
    }

    public ErrorCounterSet getErr(int errcode)
    {
        return (ErrorCounterSet)err.get(new Integer(errcode));
    }
 /*
  public void addError(ErrorCounterSet set) {
    errors.addElement(set);
  }

  public Collection getErrors() {
    return errors;
  }  */
}
