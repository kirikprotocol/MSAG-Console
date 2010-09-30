package ru.novosoft.smsc.admin.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public interface ClosedGroupManager extends SmscConfiguration {

  public List<ClosedGroup> groups() throws AdminException;

  public ClosedGroup getGroup(int groupId) throws AdminException;

  public boolean containsGroup(int groupId) throws AdminException;

  public ClosedGroup addGroup(String name, String description) throws AdminException;

  public ClosedGroup removeGroup(int groupId) throws AdminException;
}
