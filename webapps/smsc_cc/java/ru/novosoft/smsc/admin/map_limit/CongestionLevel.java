package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.Serializable;

/**
 * @author Artem Snopkov
 */
public class CongestionLevel implements Serializable {

  private static final ValidationHelper vh = new ValidationHelper(CongestionLevel.class.getCanonicalName());

  private int dialogsLimit;
  private int failLowerLimit;
  private int failUpperLimit;
  private int okToLower;

  public CongestionLevel() {
  }

  public CongestionLevel(CongestionLevel congestionLevel) {
    this.dialogsLimit = congestionLevel.dialogsLimit;
    this.failLowerLimit = congestionLevel.failLowerLimit;
    this.failUpperLimit = congestionLevel.failUpperLimit;
    this.okToLower = congestionLevel.okToLower;
  }

  public CongestionLevel(int dialogsLimit, int failLowerLimit, int failUpperLimit, int okToLower) throws AdminException {
    this.dialogsLimit = dialogsLimit;
    this.failLowerLimit = failLowerLimit;
    this.failUpperLimit = failUpperLimit;
    this.okToLower = okToLower;
  }

  public int getDialogsLimit() {
    return dialogsLimit;
  }

  public int getFailLowerLimit() {
    return failLowerLimit;
  }

  public int getFailUpperLimit() {
    return failUpperLimit;
  }

  public int getOkToLower() {
    return okToLower;
  }

  public void setDialogsLimit(int dialogsLimit) throws AdminException{
    vh.checkPositive("dialogsLimit", dialogsLimit);
    this.dialogsLimit = dialogsLimit;
  }

  public void setFailLowerLimit(int failLowerLimit) throws AdminException{
    vh.checkPositive("failLowerLimit", failLowerLimit);
    this.failLowerLimit = failLowerLimit;
  }

  public void setFailUpperLimit(int failUpperLimit) throws AdminException{
    vh.checkPositive("failUpperLimit", failUpperLimit);
    this.failUpperLimit = failUpperLimit;
  }

  public void setOkToLower(int okToLower) throws AdminException{
    vh.checkPositive("okToLower", okToLower);
    this.okToLower = okToLower;
  }

  public boolean equals(Object o) {
    if (o instanceof CongestionLevel) {
      CongestionLevel c = (CongestionLevel)o;
      return c.dialogsLimit == dialogsLimit && c.failLowerLimit == failLowerLimit && c.failUpperLimit == failUpperLimit && c.okToLower ==okToLower;
    } else
      return false;
  }
}
