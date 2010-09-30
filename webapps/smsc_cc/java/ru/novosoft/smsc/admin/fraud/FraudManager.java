package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

/**
 * @author Artem Snopkov
 */
public interface FraudManager extends SmscConfiguration {

  public FraudSettings getSettings() throws AdminException;

  public void updateSettings(FraudSettings newSettings) throws AdminException;

}
