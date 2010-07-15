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

  protected long id;
  protected String name;
  protected String description;
  protected Collection<Address> masks;
  private ClusterController cc;
  protected ClosedGroupManager cgm;


  ClosedGroup(long id, String name, String description, Collection<Address> masks, ClusterController cc, ClosedGroupManager cgm) {
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

  private void _removeMasks(Collection<Address> masks) throws AdminException {
    cgm.checkBroken();
    if (cc.isOnline())
      cc.removeMasksFromClosedGroup(id, masks);
    this.masks.removeAll(masks);
    cgm.save();

  }

  public void removeMask(Address mask) throws AdminException {
    ArrayList<Address> masks = new ArrayList<Address>(1);
    masks.add(mask);
    _removeMasks(masks);
  }

  public void removeMasks(Collection<Address> masks) throws AdminException {
    _removeMasks(new ArrayList<Address>(masks));
  }

  private void _addMasks(Collection<Address> masks) throws AdminException {
    cgm.checkBroken();

    if (cc.isOnline())
      cc.addMasksToClosedGroup(id, masks);
    this.masks.addAll(masks);
    cgm.save();
  }

  public void addMask(Address mask) throws AdminException {
    ArrayList<Address> masks2add = new ArrayList<Address>(1);
    masks2add.add(mask);
    _addMasks(masks2add);
  }

  public void addMasks(Collection<Address> masks) throws AdminException {
    _addMasks(new ArrayList<Address>(masks));
  }

  public boolean equals(Object o) {
    return o instanceof ClosedGroup && ((ClosedGroup) o).id == id;
  }
}
