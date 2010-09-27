package ru.novosoft.smsc.web.controllers.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class AclListController extends AclController {

  private List selectedRows;

  public String removeAcl() {
    if (selectedRows != null) {
      for (Object aclId : selectedRows)
      try {
        getConfiguration().removeAcl(Integer.parseInt((String)aclId), getUserName());
      } catch (AdminException e) {
        addError(e);
      }
    }    
    return null;
  }

  public void setSelectedRows(List selectedRows) {
    this.selectedRows = selectedRows;
  }

  public DataTableModel getAcls() {
    List<Acl> aclsLst;
    try {
      aclsLst = getConfiguration().acls(getUserName());
    } catch (AdminException e) {
      addError(e);
      aclsLst = Collections.emptyList();
    }

    final List<Acl> acls = aclsLst;

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        if (sortOrder != null && sortOrder.getColumnId().equals("name")) {
          Collections.sort(acls, new Comparator<Acl>() {
            public int compare(Acl o1, Acl o2) {
              int val = o1.getName().compareTo(o2.getName());
              return sortOrder.isAsc() ? val : -val;
            }
          });
        }

        List<Acl> res = new ArrayList<Acl>(count);
        for (int i = 0; i < Math.min(acls.size(), startPos + count); i++)
          res.add(acls.get(i));
        return res;
      }

      public int getRowsCount() {
        return acls.size();
      }
    };
  }
}
