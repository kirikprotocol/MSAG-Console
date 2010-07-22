package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

/**
 * @author Artem Snopkov
 */
public class TestMscManager extends MscManager {

  private final Collection<Address> mscs;

  public TestMscManager(ClusterController cc) {
    super(null, cc, null);
    mscs = new ArrayList<Address>();
    Collections.addAll(mscs,
        new Address("123443"),
        new Address("23443"),
        new Address("3443"),
        new Address("443")
    );
  }

  public Collection<Address> mscs() throws AdminException {
    return new ArrayList<Address>(mscs);
  }

  public void addMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");

    mscs.add(msc);
  }

  public void removeMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");
    mscs.remove(msc);
  }
  
}
