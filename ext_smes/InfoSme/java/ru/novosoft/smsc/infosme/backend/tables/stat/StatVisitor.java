package ru.novosoft.smsc.infosme.backend.tables.stat;

/**
 * User: artem
 * Date: 20.04.2009
 */
public interface StatVisitor {

  public boolean visit(StatisticDataItem item);
}
