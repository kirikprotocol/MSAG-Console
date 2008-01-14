package ru.sibinco.smpp.ub_sme;

public class BalanceResult {
  private String balance = null;
  private String currency = null;
  private long balanceDate = 0L;

  public BalanceResult(String balance, String currency, long balanceDate) {
    this.balance = balance;
    this.currency = currency;
    this.balanceDate = balanceDate;
  }

  public BalanceResult(String balance, String currency) {
    this.balance = balance;
    this.currency = currency;
  }

  public String getBalance() {
    return balance;
  }

  public String getCurrency() {
    return currency;
  }

  public long getBalanceDate() {
    return balanceDate;
  }
}
