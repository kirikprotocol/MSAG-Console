package mobi.eyeline.informer.web.controllers.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.10.2010
 * Time: 11:35:04
 */
public class RetryPolicyListController extends InformerController {
  private List<String> selectedRows;


  public RetryPolicyListController() throws AdminException {
    super();
  }

  public DataTableModel getPoliciesModel() throws AdminException {
    final List<RetryPolicy> policies = getConfig().getRetryPolicies();

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {


        if (sortOrder != null) {
          Collections.sort(policies, new Comparator<RetryPolicy>() {
            public int compare(RetryPolicy o1, RetryPolicy o2) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if ("name".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getPolicyId().compareTo(o2.getPolicyId());
              }
              return 0;
            }
          });
        }
        List<RetryPolicy> result = new LinkedList<RetryPolicy>();
        for (Iterator<RetryPolicy> i = policies.iterator(); i.hasNext() && count > 0;) {
          RetryPolicy p = i.next();
          if (--startPos < 0) {
            result.add(p);
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return policies.size();
      }
    };

  }


  public void setSelectedRows(List<String> selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }



  public String removeSelected() throws AdminException {
    for(String id : selectedRows) {
      try {
        getConfig().removeRetryPolicy(id,getUserName());
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }


  
}
