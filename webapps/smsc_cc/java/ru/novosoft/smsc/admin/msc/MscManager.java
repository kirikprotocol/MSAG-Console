package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.RuntimeConfiguration;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class MscManager implements RuntimeConfiguration {

  private final ClusterController cc;

  public MscManager(ClusterController cc) {
    this.cc = cc;
  }

  public Collection<Address> mscs() throws AdminException {
    return new ArrayList<Address>(cc.getMscs());
  }

  public void addMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");

    cc.registerMsc(new Address(msc));
  }

  public void removeMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");
    cc.unregisterMsc(new Address(msc));

  }
}
