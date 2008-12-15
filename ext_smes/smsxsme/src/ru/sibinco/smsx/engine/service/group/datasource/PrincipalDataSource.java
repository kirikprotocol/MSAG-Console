package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Collection;

/**
 * User: artem
 * Date: 18.11.2008
 */
public interface PrincipalDataSource {

  public void addPrincipal(Principal principal) throws DataSourceException;

  public boolean removePrincipal(String address) throws DataSourceException;

  public Principal getPrincipal(String address) throws DataSourceException;

  public Collection<Principal> getPrincipals() throws DataSourceException;

  public boolean containsPrincipal(String address) throws DataSourceException;

  public void close();
}
