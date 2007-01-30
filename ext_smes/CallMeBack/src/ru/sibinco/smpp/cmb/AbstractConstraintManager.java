package ru.sibinco.smpp.cmb;

import ru.sibinco.smpp.InitializationException;

import java.util.Properties;
import java.util.Date;
import java.text.SimpleDateFormat;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 22, 2006
 * Time: 4:14:15 PM
 */
public abstract class AbstractConstraintManager implements ConstraintManager {

  protected static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(ConstraintManager.class);

  protected Properties config = null;

  protected boolean checkBalance = false;
  protected boolean checkAttempts = false;
  protected boolean checkUsages = false;
  protected int attemptsLimit = 0;
  protected int usagesLimit = 0;

  private SimpleDateFormat sdf = new SimpleDateFormat("dd/MM/yy");

  public AbstractConstraintManager() throws InitializationException {
    try {
      config = new Properties();
      config.load(getClass().getClassLoader().getResourceAsStream("ConstraintManager.properties"));
    } catch (Exception e) {
      Logger.fatal("Could not load ConstraintManager configuration from ConstraintManager.properties: "+e.getMessage(), e);
      throw new InitializationException("Could not load ConstraintManager configuration from ConstraintManager.properties: "+e.getMessage(), e);
    }
    if (config.getProperty("check.balance") == null) {
      Logger.warn("Mandatory parameter \"check.balance\" not specified in ConstraintManager.properties. Use FALSE by default.");
      config.setProperty("check.balance", "false");
    } else  if (!config.getProperty("check.balance").trim().equalsIgnoreCase("true") &&
        !config.getProperty("check.balance").trim().equalsIgnoreCase("false")) {
      Logger.warn("\"check.balance\" value is invalid: "+config.getProperty("check.balance")+" Use FALSE by default.");
      config.setProperty("check.balance", "false");
    } else {
      checkBalance = config.getProperty("check.balance").trim().equalsIgnoreCase("true");
    }
    if (config.getProperty("check.attempts") == null) {
      config.setProperty("check.attempts", "false");
      Logger.warn("Mandatory parameter \"check.attempts\" not specified in ConstraintManager.properties. Use FALSE by default.");
    } else  if (!config.getProperty("check.attempts").trim().equalsIgnoreCase("true") &&
        !config.getProperty("check.attempts").trim().equalsIgnoreCase("false")) {
      Logger.warn("\"check.attempts\" value is invalid: "+config.getProperty("check.attempts")+" Use FALSE by default.");
      config.setProperty("check.attempts", "false");
    } else {
      checkAttempts = config.getProperty("check.attempts").trim().equalsIgnoreCase("true");
    }
    if (config.getProperty("check.usages") == null) {
      config.setProperty("check.usages", "false");
      Logger.warn("Mandatory parameter \"check.usages\" not specified in ConstraintManager.properties. Use FALSE by default.");
    } else  if (!config.getProperty("check.usages").trim().equalsIgnoreCase("true") &&
        !config.getProperty("check.usages").trim().equalsIgnoreCase("false")) {
      Logger.warn("\"check.usages\" value is invalid: "+config.getProperty("check.usages")+" Use FALSE by default.");
      config.setProperty("check.usages", "false");
    } else {
      checkUsages = config.getProperty("check.usages").trim().equalsIgnoreCase("true");
    }
    if (config.getProperty("usages.limit") == null) {
      config.setProperty("usages.limit", "0");
      Logger.warn("Mandatory parameter \"usages.limit\" not specified in ConstraintManager.properties. Use 0 (unlimited) by default.");
    } else {
      try {
        usagesLimit = Integer.parseInt(config.getProperty("usages.limit"));
        if (usagesLimit < 0) {
          Logger.warn("\"usages.limit\" value is invalid: "+config.getProperty("usages.limit")+" Use 0 (unlimited) by default.");
          config.setProperty("usages.limit", "0");
        }
      } catch (NumberFormatException e) {
        Logger.warn("\"usages.limit\" value is invalid: "+config.getProperty("usages.limit")+" Use 0 (unlimited) by default.");
        config.setProperty("usages.limit", "0");
      }
    }
    if (config.getProperty("attempts.limit") == null) {
      config.setProperty("attempts.limit", "0");
      Logger.warn("Mandatory parameter \"attempts.limit\" not specified in ConstraintManager.properties. Use 0 (unlimited) by default.");
    } else {
      try {
        attemptsLimit = Integer.parseInt(config.getProperty("attempts.limit"));
        if (attemptsLimit < 0) {
          Logger.warn("\"attempts.limit\" value is invalid: "+config.getProperty("attempts.limit")+" Use 0 (unlimited) by default.");
          config.setProperty("attempts.limit", "0");
        }
      } catch (NumberFormatException e) {
        Logger.warn("\"attempts.limit\" value is invalid: "+config.getProperty("attempts.limit")+" Use 0 (unlimited) by default.");
        config.setProperty("attempts.limit", "0");
      }
    }
  }

  public abstract short check(String abonent) throws CheckConstraintsException, AttemptsLimitReachedException, UsageLimitReachedException, BalanceLimitException;

  public abstract int registerUsage(String abonent) throws CheckConstraintsException;

  public abstract int unregisterUsage(String abonent) throws CheckConstraintsException;

  public abstract int getAttemptsLimit();

  public abstract int getUsagesLimit();

  protected String getCurrentRdate() {
    return sdf.format(new Date(System.currentTimeMillis()));
  }
}
