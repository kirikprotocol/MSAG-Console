package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Collection;

/**
 * User: artem
 * Date: 18.11.2008
 */
public interface DistrList {

  public int getId();

  public String getName();

  public String getOwner();

  public int getMaxElements();

  public void setMaxElements(int maxElements) throws DataSourceException;


  public Collection<String> members() throws DataSourceException;

  public void addMember(String member) throws DataSourceException;

  public boolean removeMember(String member) throws DataSourceException;

  public boolean containsMember(String member) throws DataSourceException;

  public int membersSize() throws DataSourceException;


  public Collection<String> submitters() throws DataSourceException;

  public void addSubmitter(String submitter) throws DataSourceException;

  public boolean removeSubmitter(String submitter) throws DataSourceException;

  public boolean containsSubmitter(String address) throws DataSourceException;

  public int submittersSize() throws DataSourceException;

  public void copyFrom(DistrList list) throws DataSourceException;
}
