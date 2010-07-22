package ru.novosoft.smsc.admin.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

/**
 * @author Artem Snopkov
 */
public class ClosedGroup {

  protected int id;
  protected String name;
  protected String description;
  protected Collection<Address> masks;
  private ClusterController cc;
  protected ClosedGroupManager cgm;

  ClosedGroup(int id, String name, String description, Collection<Address> masks, ClusterController cc, ClosedGroupManager cgm) {
    this.id= id;
    this.name = name;
    this.description = description;
    this.cc = cc;
    this.cgm = cgm;
    this.masks = masks;
  }

  public long getId() throws AdminException {
    cgm.checkBroken();
    return id;
  }

  public String getName() throws AdminException {
    cgm.checkBroken();
    return name;
  }

  public String getDescription() throws AdminException {
    cgm.checkBroken();
    return description;
  }

  public void setDescription(String description) throws AdminException {
    cgm.checkBroken();

    this.description = description;
    cgm.save();
  }

  public Collection<Address> getMasks() throws AdminException {
    cgm.checkBroken();
    return new ArrayList<Address>(masks);
  }

  public void removeMask(Address mask) throws AdminException {
    cgm.checkBroken();
    if (cc.isOnline())
      cc.removeMaskFromClosedGroup(id, mask);
    this.masks.remove(mask);
    cgm.save();
  }

  public void addMask(Address mask) throws AdminException {
    cgm.checkBroken();
    if (cc.isOnline())
      cc.addMaskToClosedGroup(id, mask);
    this.masks.add(mask);
    cgm.save();
  }

  public boolean equals(Object o) {
    return o instanceof ClosedGroup && ((ClosedGroup) o).id == id;
  }
}
