package ru.sibinco.smpp.cmb;


/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 21, 2006
 * Time: 6:46:48 PM
 */
public interface ConstraintManager {
  public void check(String abonent)
      throws CheckConstraintsException,
      AttemptsLimitReachedException,
      UsageLimitReachedException,
      BalanceLimitException;


  public int getLeftAttempts (String abonent) throws CheckConstraintsException;

  public RegisterAttemptAnswer registerAttempt(String abonent) throws CheckConstraintsException;

  public int unregisterAttempt(String abonent) throws CheckConstraintsException;

  public int getLeftUsages (String abonent) throws CheckConstraintsException;
  
  public int registerUsage(String abonent, boolean isUsageDecreased) throws CheckConstraintsException;

  public int unregisterUsage(String abonent) throws CheckConstraintsException;

  public int getAttemptsLimit();

  public int getUsagesLimit();
}
