package ru.novosoft.smsc.admin.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class TestClosedGroup extends ClosedGroup {

  TestClosedGroup(int id, String name, String description, Collection<Address> masks, ClosedGroupManager cgm) {
    super(id, name, description, masks, null, cgm);
  }

  public void setDescription(String description) throws AdminException {
    this.description = description;    
  }

  public void removeMask(Address mask) throws AdminException {
   this.masks.remove(mask);
  }

  public void addMask(Address mask) throws AdminException {
    this.masks.add(mask);
  }

}
