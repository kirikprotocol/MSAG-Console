package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;

/**
 * @author Aleksandr Khalitov
 */
public interface SmsxResults { //todo перенести в backend

  public void getSmsxUsers(Visitor visitor) throws StatisticsException;

  public void getWebDaily(Visitor visitor) throws StatisticsException;

  public void getWebRegions(Visitor visitor) throws StatisticsException;

  public void getTraffic(Visitor visitor) throws StatisticsException;


}
