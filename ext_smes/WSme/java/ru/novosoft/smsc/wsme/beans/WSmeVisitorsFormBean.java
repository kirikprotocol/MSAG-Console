/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 3:30:29 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.wsme.WSmeErrors;

import java.util.List;
import java.util.ArrayList;

public class WSmeVisitorsFormBean extends WSmeBaseFormBean
{
  private String newVisitor = null;

  private List visitors = new ArrayList();

  public int process(List errors)
  {
    int result = super.process(errors);
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
    System.out.println("WSmeVisitors::addNewVisitor() called");
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
    System.out.println("WSmeVisitors::delVisitor() called");
    int result = RESULT_OK;
    try {
      for (int i=0; i<selectedRows.length; i++)
        wsme.removeVisitor(selectedRows[i]);
    }
    catch (AdminException exc) {
       result = error(WSmeErrors.error.remote.failure, exc.getMessage());
    }
    return result;
  }

  public int loadVisitors()
  {
    int result = RESULT_OK;
    try {
       visitors = wsme.getVisitors();
    }
    catch (AdminException exc) {
       visitors.clear();
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
