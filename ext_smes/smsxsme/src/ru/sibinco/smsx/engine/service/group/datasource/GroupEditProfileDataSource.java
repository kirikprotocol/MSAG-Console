package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 25.07.2008
 */

public interface GroupEditProfileDataSource {
  public void saveProfile(GroupEditProfile profile) throws DataSourceException;
  public GroupEditProfile loadProfile(String address) throws DataSourceException;
  public void shutdown();
}
