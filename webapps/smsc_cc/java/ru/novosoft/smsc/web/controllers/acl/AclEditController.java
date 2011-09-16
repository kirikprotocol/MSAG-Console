package ru.novosoft.smsc.web.controllers.acl;

import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.util.Address;

import javax.faces.application.FacesMessage;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class AclEditController extends AclController {

  private int id = -1;
  private String name;
  private String description;
  private DynamicTableModel addresses = new DynamicTableModel();


  public AclEditController() {
    String idParam = getRequestParameter("aclId");
    if (idParam != null && getRequestParameter("initialized") == null) {
      try {
        id = Integer.parseInt(idParam);
        Acl acl = aclManager.getAcl(id);
        name = acl.getName();
        description = acl.getDescription();
        for (Address addr : acl.getAddresses()) {
          DynamicTableRow r = new DynamicTableRow();
          r.setValue("address", addr.getSimpleAddress());
          addresses.addRow(r);
        }
      } catch (AdminException e) {
        addError(e);
      }
    }
  }

  public String save() {
    try {
      List<Address> addresses = new ArrayList<Address>();
      for (DynamicTableRow r : this.addresses.getRows())
        addresses.add(new Address((String) r.getValue("address")));

      if (id == -1) {
        aclManager.createAcl(name, description, addresses);
      } else {
        Acl acl = aclManager.getAcl(id);
        acl.updateInfo(name, description);
        if (addresses != null) {
          List<Address> oldAddresses = acl.getAddresses();
          Set<Address> addresses2add = new HashSet<Address>(addresses);
          addresses2add.removeAll(oldAddresses);
          if (!addresses2add.isEmpty())
            acl.addAddresses(new ArrayList<Address>(addresses2add));

          Set<Address> addresses2remove = new HashSet<Address>(oldAddresses);
          addresses2remove.removeAll(addresses);
          if (!addresses2remove.isEmpty())
            acl.removeAddresses(new ArrayList<Address>(addresses2remove));
        }
      }
    } catch (AdminException e) {
      addError(e);
      return null;
    } catch (IllegalArgumentException e) {
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage());
      return null;
    }
    return "ACLS";
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getDescription() {
    return description;
  }

  public void setDescription(String description) {
    this.description = description;
  }

  public DynamicTableModel getAddresses() {
    return addresses;
  }

  public void setAddresses(DynamicTableModel model) {
    this.addresses = model;
  }

  public int getId() {
    return id;
  }

  public String getIdStr() {
    return " (Id=" + id + ")";
  }

  public void setId(int id) {
    this.id = id;
  }
}
