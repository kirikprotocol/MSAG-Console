package ru.novosoft.smsc.web.config.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WClosedGroupManager implements ClosedGroupManager {

  private final ClosedGroupManager wrapped;
  private final String user;
  private final Journal j;


  public WClosedGroupManager(ClosedGroupManager wrapped, String user, Journal j) {
    this.wrapped = wrapped;
    this.user = user;
    this.j = j;
  }

  public List<ClosedGroup> groups() throws AdminException {
    List<ClosedGroup> gs = wrapped.groups();

    List<ClosedGroup> result = new ArrayList<ClosedGroup>(gs.size());
    for(ClosedGroup g : gs)
      result.add(new WClosedGroup(g, j, user));
    return result;
  }

  public ClosedGroup getGroup(int groupId) throws AdminException {
    ClosedGroup g = wrapped.getGroup(groupId);
    if (g == null)
      return null;
    return new WClosedGroup(g, j, user);
  }

  public boolean containsGroup(int groupId) throws AdminException {
    return wrapped.containsGroup(groupId);
  }

  public ClosedGroup addGroup(String name, String description) throws AdminException {
    ClosedGroup g = wrapped.addGroup(name, description);
    if (g == null)
      return null;
    j.user(user).add().closedGroup(name);
    return new WClosedGroup(g, j, user);
  }

  public ClosedGroup removeGroup(int groupId) throws AdminException {
    ClosedGroup g = wrapped.removeGroup(groupId);
    if (g == null)
      return null;
    j.user(user).remove().closedGroup(g.getName());    
    return new WClosedGroup(g, j, user);
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }
}
