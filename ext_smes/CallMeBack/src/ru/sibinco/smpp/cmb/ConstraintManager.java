package ru.sibinco.smpp.cmb;

import ru.sibinco.smpp.MessageData;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 21, 2006
 * Time: 6:46:48 PM
 */
public interface ConstraintManager {
  public short check(String abonent)
      throws CheckConstraintsException,
      AttemptsLimitReachedException,
      UsageLimitReachedException,
      BalanceLimitException;


  public int registerUsage(String abonent) throws CheckConstraintsException;

  public int unregisterUsage(String abonent) throws CheckConstraintsException;

  public int getAttemptsLimit();

  public int getUsagesLimit();
}
