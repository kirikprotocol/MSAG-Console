/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 4:10:39 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.wsme.AdRow;
import ru.novosoft.smsc.wsme.LangRow;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.ArrayList;

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

  public List getMaskFilteredList(List list, MaskList masks)
      throws AdminException
  {
    if (masks == null || masks.size() == 0) return list;

    List result = new ArrayList();
    for (int i=0; i<list.size(); i++)
    {
      Object obj = list.get(i);
      if (obj == null) continue;
      Mask mask = null;
      if (obj instanceof String)       mask = new Mask((String)obj);
      else if (obj instanceof LangRow) mask = new Mask(((LangRow)obj).mask);

      if (mask != null && masks.contains(mask))
        result.add(obj);
    }
    return result;
  }

  public List getLangFilteredList(List list, List langs)
  {
    if (langs == null || langs.size() == 0) return list;

    List result = new ArrayList();
    for (int i=0; i<list.size(); i++)
    {
      Object obj = list.get(i);
      if (obj == null) continue;
      String lang = null;
      if (obj instanceof String)       lang = (String)obj;
      else if (obj instanceof LangRow) lang = ((LangRow)obj).lang;
      else if (obj instanceof AdRow)   lang = ((AdRow)obj).lang;

      if (lang != null && langs.contains(lang.trim()))
        result.add(obj);
    }
    return result;
  }

}
