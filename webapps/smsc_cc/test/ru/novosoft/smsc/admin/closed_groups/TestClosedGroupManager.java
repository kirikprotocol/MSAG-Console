package ru.novosoft.smsc.admin.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;

/**
 * @author Artem Snopkov
 */
public class TestClosedGroupManager extends ClosedGroupManager {
  public TestClosedGroupManager() throws AdminException {
    super(null, null, null, null);
  }

  protected void load() throws AdminException {
    load(TestClosedGroupManager.class.getResourceAsStream("ClosedGroups.xml"));
  }

  public ClosedGroup addGroup(String name, String description) throws AdminException {
    ClosedGroup g = new TestClosedGroup(getNextId(), name, description, new ArrayList<Address>(), this);
    groups.add(g);
    return g;
  }

  public boolean removeGroup(int groupId) throws AdminException {
    ClosedGroup group2remove = null;
    for (ClosedGroup g : groups) {
      if (g.getId() == groupId) {
        group2remove = g;
        break;
      }
    }

    return group2remove != null && groups.remove(group2remove);
  }
}
