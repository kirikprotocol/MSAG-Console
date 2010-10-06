package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public interface SmscManager extends SmscConfiguration {
  SmscSettings getSettings() throws AdminException;

  void updateSettings(SmscSettings s) throws AdminException;

  void startSmsc(int instanceNumber) throws AdminException;

  void stopSmsc(int instanceNumber) throws AdminException;

  void switchSmsc(int instanceNumber, String toHost) throws AdminException;

  String getSmscOnlineHost(int instanceNumber) throws AdminException;

  List<String> getSmscHosts(int instanceNumber) throws AdminException;

}
