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

import java.util.List;

public class WSmeAdsFormBean extends WSmeBaseFormBean
{
  private String newId;
  private String newLang;
  private String newAd;

  public int process(List errors)
  {
    int result = super.process(errors);
    if (result != RESULT_OK ||
        result != RESULT_ADS) return result;

    if (btnAdd != null && newId != null && newLang != null && newAd != null)
      result = addNewAd();
    else if (btnDel != null && selectedRows != null)
      result = delAds();

    selectedRows = null; btnAdd = null; btnDel = null;
    newId = null; newLang = null; newAd = null;
    return RESULT_OK;
  }

  protected int addNewAd()
  {
    System.out.println("WSmeAds::addNewAd() called");
    int result = RESULT_OK;
    try {
      int id = Integer.parseInt(newId);
      wsme.addAd(id, newLang, newAd);
    }
    catch (Exception exc) {
       result = error(WSmeErrors.error.transport.failure, exc.getMessage());
    }
    return result;
  }

  protected int delAds()
  {
    System.out.println("WSmeAds::delAds() called");
    int result = RESULT_OK;
    try {
      for (int i=0; i<selectedRows.length; i++) {
        // TODO: parse selectedRows[i] here, get id and lang !!!
        wsme.removeAd(0, "");
      }
    }
    catch (AdminException exc) {
       result = error(WSmeErrors.error.transport.failure, exc.getMessage());
    }
    return result;
  }

  public List getAds()
  {
    List ads = null;
    try {
       ads = wsme.getAds();
    }
    catch (AdminException exc) {
       int result = error(WSmeErrors.error.datasource.failure, exc.getMessage());
    }
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

}
