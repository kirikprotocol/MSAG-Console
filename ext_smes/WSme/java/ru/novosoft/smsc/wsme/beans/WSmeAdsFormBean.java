/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 14, 2003
 * Time: 4:16:41 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.wsme.WSmeErrors;
import ru.novosoft.smsc.wsme.WSmePreferences;
import ru.novosoft.smsc.wsme.AdRow;

import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.security.Principal;

public class WSmeAdsFormBean extends WSmeBaseFormBean
{
  private String newId;
  private String newLang;
  private String newAd;

  private List ads = new ArrayList();

  public final static char ID_LANG_SEPARATOR = '-';

  public int process(List errors, Principal loginedUserPrincipal)
  {
    int result = super.process(errors, loginedUserPrincipal);

    pageSize = (wsmePreferences != null) ?
        wsmePreferences.getAdsPageSize():WSmePreferences.DEFAULT_adsPageSize;
    if (sort == null)
      sort = (wsmePreferences != null) ?
        wsmePreferences.getAdsSortOrder():WSmePreferences.DEFAULT_adsSortOrder;
    else if (wsmePreferences != null)
        wsmePreferences.setAdsSortOrder(sort);

    if (result != RESULT_OK && result != RESULT_ADS) return result;
    result = RESULT_OK;

    if (btnAdd != null && newId != null && newLang != null && newAd != null) {
      result = addNewAd();
      if (result == RESULT_OK) return RESULT_ADS; //redirect for refresh
    }
    else if (btnDel != null && selectedRows != null) {
      result = delAds();
      if (result == RESULT_OK) return RESULT_ADS; //redirect for refresh
    }

    int loadResult = loadAds();
    result = (result == RESULT_OK) ? loadResult:result;

    selectedRows = null; btnAdd = null; btnDel = null;
    newId = null; newLang = null; newAd = null;
    return result;
  }

  protected int addNewAd()
  {
    System.out.println("WSmeAds::addNewAd() called");
    int result = RESULT_OK;
    try {
      wsme.addAd(Integer.parseInt(newId.trim()), newLang, newAd);
    }
    catch (NumberFormatException exc) {
      result = error(WSmeErrors.error.admin.ParseError, exc.getMessage());
    }
    catch (Exception exc) {
       result = error(WSmeErrors.error.remote.failure, exc.getMessage());
    }
    return result;
  }

  protected int delAds()
  {
    System.out.println("WSmeAds::delAds() called");
    int result = RESULT_OK;
    try {
      for (int i=0; i<selectedRows.length; i++) {
        String row = selectedRows[i];
        int separator = (row == null) ? -1:row.indexOf(ID_LANG_SEPARATOR);
        if (separator <= 0) continue;
        int id = Integer.parseInt(row.substring(0, separator));
        wsme.removeAd(id, row.substring(separator+1));
      }
    }
    catch (NumberFormatException exc) {
      result = error(WSmeErrors.error.admin.ParseError, exc.getMessage());
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

      if (ads != null)
      {
        Collections.sort(ads, new Comparator()
        {
          public int compare(Object o1, Object o2)
          {
            int result = 0;
            AdRow r1 = (AdRow) o1;
            AdRow r2 = (AdRow) o2;
            if (sortField.equalsIgnoreCase("id")) {
              String id1 = Integer.toString(r1.id);
              String id2 = Integer.toString(r2.id);
              result = id1.compareTo(id2);
            }
            if (sortField.equalsIgnoreCase("lang"))
              result = r1.lang.compareTo(r2.lang);

            return isNegativeSort ? -result : result;
          }
        });
      }
    }
    return RESULT_OK;
  }

  public int loadAds()
  {
    int result = RESULT_OK;
    try {
      ads = wsme.getAds();
      result = processSort();
      ads = getLangFilteredList(ads, wsmePreferences.getAdsFilter().getLangList());
      ads = getPaginatedList(ads);
    }
    catch (AdminException exc) {
       clearPaginatedList(ads);
       result = error(WSmeErrors.error.datasource.failure, exc.getMessage());
    }
    return result;
  }
  public List getAds() {
    return ads;
  }

  public String getNewId() {
    return (newId == null) ? "":newId;
  }
  public void setNewId(String newId) {
    this.newId = newId;
  }

  public String getNewLang() {
    return (newLang == null) ? "":newLang;
  }
  public void setNewLang(String newLang) {
    this.newLang = newLang;
  }

  public String getNewAd() {
    return (newAd == null) ? "":newAd;
  }
  public void setNewAd(String newAd) {
    this.newAd = newAd;
  }

  public int getMenuId() {
    return RESULT_ADS;
  }
}
