/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 4:09:59 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.wsme.WSmeErrors;

import java.util.List;

public class WSmeLangsFormBean extends WSmeBaseFormBean
{
  private String newMask = null;
  private String newLang = null;

  public int process(List errors)
  {
    int result = super.process(errors);
    if (result != RESULT_OK &&
        result != RESULT_LANGS) return result;

    if (btnAdd != null && newMask != null && newLang != null)
      result = addNewLang();
    else if (btnDel != null && selectedRows != null)
      result = delLangs();

    selectedRows = null; btnAdd = null; btnDel = null;
    newLang = null; newMask = null;
    return RESULT_OK;
  }

  protected int addNewLang()
  {
    System.out.println("WSmeLangs::addNewLang() called");
    int result = RESULT_OK;
    try {
       wsme.addLang(newMask, newLang);
    }
    catch (AdminException exc) {
       result = error(WSmeErrors.error.failed, exc.getMessage());
    }
    return result;
  }
  protected int delLangs()
  {
    System.out.println("WSmeLangs::delLangs() called");
    int result = RESULT_OK;
    try {
      for (int i=0; i<selectedRows.length; i++)
        wsme.removeLang(selectedRows[i]);
    }
    catch (AdminException exc) {
       result = error(WSmeErrors.error.failed, exc.getMessage());
    }
    return result;
  }

  public List getLangs()
  {
    List langs = null;
    try {
       langs = wsme.getLangs();
    }
    catch (AdminException exc) {
       int result = error(WSmeErrors.error.datasource.failure, exc.getMessage());
    }
    return langs;
  }

  public String getNewLang() {
    return (newLang == null) ? "":newLang;
  }
  public void setNewLang(String newLang) {
    this.newLang = newLang;
  }

  public String getNewMask() {
    return (newMask == null) ? "":newMask;
  }
  public void setNewMask(String newMask) {
    this.newMask = newMask;
  }
}
