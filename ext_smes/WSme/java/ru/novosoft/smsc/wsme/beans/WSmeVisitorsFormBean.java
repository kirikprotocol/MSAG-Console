/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 3:30:29 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.wsme.WSmeErrors;
import ru.novosoft.smsc.wsme.WSmePreferences;

import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.security.Principal;

public class WSmeVisitorsFormBean extends WSmeBaseFormBean
{
  private String newVisitor = null;

  private List visitors = new ArrayList();

  public int process(List errors, Principal loginedUserPrincipal)
  {
    int result = super.process(errors, loginedUserPrincipal);

    pageSize = (wsmePreferences != null) ?
        wsmePreferences.getVisitorsPageSize():WSmePreferences.DEFAULT_visitorsPageSize;
    if (sort == null)
      sort = (wsmePreferences != null) ?
        wsmePreferences.getVisitorsSortOrder():WSmePreferences.DEFAULT_visitorsSortOrder;
    else if (wsmePreferences != null)
        wsmePreferences.setVisitorsSortOrder(sort);

    if (result != RESULT_OK && result != RESULT_VISITORS) return result;
    result = RESULT_OK;

    if (btnAdd != null && newVisitor != null) {
      result = addNewVisitor();
      if (result == RESULT_OK) return RESULT_VISITORS; //redirect for refresh
    }
    else if (btnDel != null && selectedRows != null) {
      result = delVisitors();
      if (result == RESULT_OK) return RESULT_VISITORS; //redirect for refresh
    }

    int loadResult = loadVisitors();
    result = (result == RESULT_OK) ? loadResult:result;

    selectedRows = null; btnAdd = null; btnDel = null;
    newVisitor = null;
    return result;
  }

  protected int addNewVisitor()
  {
    try {
      newVisitor = (new Mask(newVisitor)).getNormalizedMask().trim();
    } catch (AdminException exc) {
      return error(WSmeErrors.error.admin.ParseError, exc.getMessage());
    }
    int result = RESULT_OK;
    try {
       wsme.addVisitor(newVisitor);
    }
    catch (AdminException exc) {
       result = error(WSmeErrors.error.remote.failure, exc.getMessage());
    }
    return result;
  }
  protected int delVisitors()
  {
    int result = RESULT_OK;
    try {
      for (int i=0; i<selectedRows.length; i++)
        wsme.removeVisitor((new Mask(selectedRows[i])).getNormalizedMask().trim());
    }
    catch (AdminException exc) {
       result = error(WSmeErrors.error.remote.failure, exc.getMessage());
    }
    return result;
  }

  private int processSort()
  {
    if (sort != null && sort.length() > 0)
    {
      final boolean isNegativeSort = sort.startsWith("-");
      final String sortField = isNegativeSort ? sort.substring(1) : sort;

      if (visitors != null)
      {
        Collections.sort(visitors, new Comparator()
        {
          public int compare(Object o1, Object o2)
          {
            int result = 0;
            String r1 = (String) o1;
            String r2 = (String) o2;
            if (sortField.equalsIgnoreCase("mask"))
              result = r1.compareTo(r2);

            return isNegativeSort ? -result : result;
          }
        });
      }
    }
    return RESULT_OK;
  }

  public int loadVisitors()
  {
    int result = RESULT_OK;
    try {
      visitors = wsme.getVisitors();
      result = processSort();
      MaskList maskList = wsmePreferences.getVisitorsFilter().getMaskList();
      visitors = getPaginatedList(getMaskFilteredList(visitors, maskList));
    }
    catch (AdminException exc) {
      clearPaginatedList(visitors);
      result = error(WSmeErrors.error.datasource.failure, exc.getMessage());
    }
    return result;
  }
  public List getVisitors() {
    return visitors;
  }

  public String getNewVisitor() {
    return (newVisitor == null) ? "":newVisitor;
  }
  public void setNewVisitor(String newVisitor) {
    this.newVisitor = newVisitor;
  }

  public int getMenuId() {
    return RESULT_VISITORS;
  }
}
