package ru.sibinco.sponsored.stats.backend.datasource;

/**
 * @author Aleksandr Khalitov
 */
public interface DeliveryStatsQuery {
  public boolean isAllowed(DeliveryStat stat);
}
