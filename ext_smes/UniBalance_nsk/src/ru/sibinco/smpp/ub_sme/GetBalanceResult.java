package ru.sibinco.smpp.ub_sme;

public class GetBalanceResult {
  private String balance = null;
  private String currency = null;
  private long balanceDate = 0L;

  public GetBalanceResult(String balance, String currency, long balanceDate) {
    this.balance = balance;
    this.currency = currency;
    this.balanceDate = balanceDate;
  }

  public GetBalanceResult(String balance) {
    this.balance = balance;
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
