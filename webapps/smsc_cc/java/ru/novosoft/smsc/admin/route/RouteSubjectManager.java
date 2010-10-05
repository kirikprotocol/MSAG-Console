package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

/**
 * @author Artem Snopkov
 */
public interface RouteSubjectManager extends SmscConfiguration {

  RouteSubjectSettings getSettings() throws AdminException;

  void updateSettings(RouteSubjectSettings settings) throws AdminException;
}
