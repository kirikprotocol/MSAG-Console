package ru.sibinco.sponsored.stats.backend;

/**
 * @author Aleksandr Khalitov
 */
public interface Visitor {

  public boolean visit(Object o) throws StatisticsException;

}
