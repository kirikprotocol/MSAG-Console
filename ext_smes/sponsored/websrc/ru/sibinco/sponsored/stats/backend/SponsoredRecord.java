package ru.sibinco.sponsored.stats.backend;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredRecord {

  private final String address;

  private final float bonus;

  SponsoredRecord(String address, float bonus) {
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
