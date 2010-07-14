package ru.novosoft.smsc.admin.map_limit;

/**
 * @author Artem Snopkov
 */
public class CongestionLevel {
  private final int dialogsLimit;
  private final int failLowerLimit;
  private final int failUpperLimit;
  private final int okToLower;

  public CongestionLevel(int dialogsLimit, int failLowerLimit, int failUpperLimit, int okToLower) {
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

  public boolean equals(Object o) {
    if (o instanceof CongestionLevel) {
      CongestionLevel c = (CongestionLevel)o;
      return c.dialogsLimit == dialogsLimit && c.failLowerLimit == failLowerLimit && c.failUpperLimit == failUpperLimit && c.okToLower ==okToLower;
    } else
      return false;
  }
}
