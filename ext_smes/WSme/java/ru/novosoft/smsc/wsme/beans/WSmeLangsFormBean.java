/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 4:09:59 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.wsme.WSmeErrors;
import ru.novosoft.smsc.wsme.WSmePreferences;
import ru.novosoft.smsc.wsme.LangRow;

import java.util.List;
import java.util.ArrayList;
import java.security.Principal;

public class WSmeLangsFormBean extends WSmeBaseFormBean
{
  private String newMask = null;
  private String newLang = null;

  private List langs = new ArrayList();

  public int process(List errors, Principal loginedUserPrincipal)
  {
    int result = super.process(errors, loginedUserPrincipal);
    pageSize = (wsmePreferences != null) ?
        wsmePreferences.getLangsPageSize():WSmePreferences.DEFAULT_langsPageSize;
    if (result != RESULT_OK && result != RESULT_LANGS) return result;
    result = RESULT_OK;

    if (btnAdd != null && newMask != null && newLang != null) {
      result = addNewLang();
      if (result == RESULT_OK) return RESULT_LANGS; //redirect for refresh
    }
    else if (btnDel != null && selectedRows != null) {
      result = delLangs();
      if (result == RESULT_OK) return RESULT_LANGS; //redirect for refresh
    }

    int loadResult = loadLangs();
    result = (result == RESULT_OK) ? loadResult:result;

    selectedRows = null; btnAdd = null; btnDel = null;
    newLang = null; newMask = null;
    return result;
  }

  protected int addNewLang()
  {
    System.out.println("WSmeLangs::addNewLang() called");
    try {
      newMask = (new Mask(newMask)).getNormalizedMask().trim();
    } catch (AdminException exc) {
      return error(WSmeErrors.error.admin.ParseError, exc.getMessage());
    }
    int result = RESULT_OK;
    try {
      wsme.addLang(newMask, newLang);
    } catch (AdminException exc) {
      result = error(WSmeErrors.error.remote.failure, exc.getMessage());
    }
    return result;
  }
  protected int delLangs()
  {
    System.out.println("WSmeLangs::delLangs() called");
    int result = RESULT_OK;
    try {
      for (int i=0; i<selectedRows.length; i++)
        wsme.removeLang((new Mask(selectedRows[i])).getNormalizedMask().trim());
    }
    catch (AdminException exc) {
       result = error(WSmeErrors.error.remote.failure, exc.getMessage());
    }
    return result;
  }

  public int loadLangs()
  {
    int result = RESULT_OK;
    try {
      langs = wsme.getLangs();
      langs = getMaskFilteredList(langs, wsmePreferences.getLangsFilter().getMaskList());
      langs = getLangFilteredList(langs, wsmePreferences.getLangsFilter().getLangList());
      langs = getPaginatedList(langs);
    }
    catch (AdminException exc) {
      clearPaginatedList(langs);
      result = error(WSmeErrors.error.datasource.failure, exc.getMessage());
    }
    return result;
  }
  public List getLangs() {
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

  public int getMenuId() {
    return RESULT_LANGS;
  }
}
