/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 30, 2003
 * Time: 3:40:19 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

public class WSmePreferences
{
  public final static int DEFAULT_visitorsPageSize = 10;
  public final static int DEFAULT_langsPageSize = 10;
  public final static int DEFAULT_adsPageSize = 10;

  private int visitorsPageSize = DEFAULT_visitorsPageSize;
  private int langsPageSize = DEFAULT_langsPageSize;
  private int adsPageSize = DEFAULT_adsPageSize;

  public final static String DEFAULT_visitorsSortOrder = "mask";
  public final static String DEFAULT_langsSortOrder = "mask";
  public final static String DEFAULT_adsSortOrder = "id";

  private String visitorsSortOrder = DEFAULT_visitorsSortOrder;
  private String langsSortOrder = DEFAULT_langsSortOrder;
  private String adsSortOrder = DEFAULT_adsSortOrder;

  private WSmeVisitorsFilter visitorsFilter = new WSmeVisitorsFilter();
  private WSmeLangsFilter langsFilter = new WSmeLangsFilter();
  private WSmeAdsFilter adsFilter = new WSmeAdsFilter();

  synchronized public int getVisitorsPageSize() {
    return visitorsPageSize;
  }
  synchronized public void setVisitorsPageSize(int visitorsPageSize) {
    this.visitorsPageSize = visitorsPageSize;
  }
  synchronized public int getLangsPageSize() {
    return langsPageSize;
  }
  synchronized public void setLangsPageSize(int langsPageSize) {
    this.langsPageSize = langsPageSize;
  }
  synchronized public int getAdsPageSize() {
    return adsPageSize;
  }
  synchronized public void setAdsPageSize(int adsPageSize) {
    this.adsPageSize = adsPageSize;
  }

  synchronized public String getVisitorsSortOrder() {
    return visitorsSortOrder;
  }
  synchronized public void setVisitorsSortOrder(String visitorsSortOrder) {
    this.visitorsSortOrder = visitorsSortOrder;
  }
  synchronized public String getLangsSortOrder() {
    return langsSortOrder;
  }
  synchronized public void setLangsSortOrder(String langsSortOrder) {
    this.langsSortOrder = langsSortOrder;
  }
  synchronized public String getAdsSortOrder() {
    return adsSortOrder;
  }
  synchronized public void setAdsSortOrder(String adsSortOrder) {
    this.adsSortOrder = adsSortOrder;
  }

  synchronized public WSmeVisitorsFilter getVisitorsFilter() {
    return visitorsFilter;
  }
  synchronized public WSmeLangsFilter getLangsFilter() {
    return langsFilter;
  }
  synchronized public WSmeAdsFilter getAdsFilter() {
    return adsFilter;
  }
}
