package ru.sibinco.smsx.stats.backend;

/**
 * @author Aleksandr Khalitov
 */
public interface SmsxResults {

  public void getSmsxUsers(Visitor visitor) throws StatisticsException;

  public void getWebDaily(Visitor visitor) throws StatisticsException;

  public void getWebRegions(Visitor visitor) throws StatisticsException;

  public void getTraffic(Visitor visitor) throws StatisticsException;


}
