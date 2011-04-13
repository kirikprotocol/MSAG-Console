package ru.sibinco.sponsored.stats.backend;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredRequestPrototype {

  private Date from;

  private Date till;

  private float cost;


  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }

  public float getCost() {
    return cost;
  }

  public void setCost(float cost) {
    this.cost = cost;
  }
}
