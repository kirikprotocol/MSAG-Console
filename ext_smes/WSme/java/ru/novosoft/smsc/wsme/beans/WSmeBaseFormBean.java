/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 4:10:39 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

public class WSmeBaseFormBean extends WSmeFormBean
{
  protected String[] selectedRows = null;

  protected String btnAdd = null;
  protected String btnDel = null;

  public String getBtnAdd() {
    return btnAdd;
  }
  public void setBtnAdd(String btnAdd) {
    this.btnAdd = btnAdd;
  }

  public String getBtnDel() {
    return btnDel;
  }
  public void setBtnDel(String btnDel) {
    this.btnDel = btnDel;
  }

  public String[] getSelectedRows() {
    return selectedRows;
  }
  public void setSelectedRows(String[] selectedRows) {
    this.selectedRows = selectedRows;
  }
  public boolean isRowSelected(String row) {
    if (selectedRows == null) return false;
    for (int i=0; i<selectedRows.length; i++)
      if (selectedRows[i].equalsIgnoreCase(row))
        return true;
    return false;
  }

}
