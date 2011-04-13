package ru.sibinco.sponsored.stats.backend.datasource;

import ru.sibinco.sponsored.stats.backend.StatisticsException;

/**
 * @author Aleksandr Khalitov
 */
public interface ResultSet {

  public Object get();

  public boolean next() throws StatisticsException;

  public void close() throws StatisticsException;
}
