package ru.sibinco.smsx.stats.backend;

/**
 * @author Aleksandr Khalitov
 */
public interface Visitor {

  public boolean visit(Object o) throws StatisticsException;

}
