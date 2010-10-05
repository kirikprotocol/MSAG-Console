package ru.novosoft.smsc.web.controllers.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 16.09.2010
 * Time: 15:32:53
 */
public class MscController extends SmscController {



  private String prefixFilter=null;
  private List<String> selectedRows;
  private String newAddress=null;


  public MscController() {
    super();

  }

  public String getPrefixFilter() {
    return prefixFilter;
  }

  public void setPrefixFilter(String prefixFilter) {
    if(prefixFilter.trim().length()==0) prefixFilter=null;
    this.prefixFilter = prefixFilter;
  }

  public void setSelectedRows(List<String> selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }

  public String getNewAddress() {
    return newAddress;
  }

  public void setNewAddress(String newAddres) {
    this.newAddress = newAddres;
  }

  public String clearFilter() {
    prefixFilter = null;
    return null;
  }


  public void removeSelected(ActionEvent actionEvent) throws AdminException {
    if(selectedRows!=null)
      for(String s : selectedRows) {
        Address a = new Address(s);
        WebContext.getInstance().getMscManager().removeMsc(a);
      }
  }

  public DataTableModel getCommutatorsModel() throws AdminException {
    final List<Address> filteredMscs = new ArrayList<Address>();

    Collection<Address> mscs = WebContext.getInstance().getMscManager().mscs();
    for(Address msc : mscs) {
      if (prefixFilter != null && !msc.getSimpleAddress().startsWith(prefixFilter)) {
        continue;
      }
      filteredMscs.add(msc);
    }

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        if (sortOrder != null) {
          Collections.sort(filteredMscs, new Comparator<Address>() {
            public int compare(Address o1, Address o2) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              return (mul) * o1.getSimpleAddress().compareTo(o2.getSimpleAddress());
            }
          });
        }
        List<Address> result = new LinkedList<Address>();
        for (Iterator<Address> i = filteredMscs.iterator(); i.hasNext() && count > 0;) {
          Address a = i.next();
          if (--startPos < 0) {
            result.add(a);
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return filteredMscs.size();
      }
    };

  }


  public void addAddress(ActionEvent actionEvent) throws AdminException {
    if(newAddress!=null) {
      if(Address.validate(newAddress)) {
        Address a = new Address(newAddress);
        WebContext.getInstance().getMscManager().addMsc(a);
        newAddress = null;
      }
      else {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN,"smsc.msc.invlid.addr");
      }
    }
  }
}