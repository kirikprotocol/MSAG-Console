package ru.sibinco.sponsored.stats.backend.datasource;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredRecord {

  private final String address;

  private final float bonus;

  public SponsoredRecord(String address, float bonus) { //todo убрать public
    this.address = address;
    this.bonus = bonus;
  }

  public String getAddress() {
    return address;
  }

  public float getBonus() {
    return bonus;
  }
}
