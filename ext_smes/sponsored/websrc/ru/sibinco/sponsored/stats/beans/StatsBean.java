package ru.sibinco.sponsored.stats.beans;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.PageBean;
import ru.sibinco.sponsored.stats.SponsoredContext;
import ru.sibinco.sponsored.stats.backend.SponsoredRequest;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class StatsBean extends PageBean {

  public static final int VIEW_SMSX_USERS = PageBean.PRIVATE_RESULT;
  public static final int VIEW_WEB_DAILY = PageBean.PRIVATE_RESULT+1;
  public static final int VIEW_WEB_REGIONS = PageBean.PRIVATE_RESULT+2;
  public static final int VIEW_TRAFFIC = PageBean.PRIVATE_RESULT+3;

  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 4;

  private String mbMenu = null;

  protected Category logger = Category.getInstance(SponsoredContext.class);

  protected SponsoredContext sponsoredContext;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    sponsoredContext = SponsoredContext.getInstance();
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if ("smsx_users".equals(mbMenu))
      result = VIEW_SMSX_USERS;
    else if ("smsx_web_daily".equals(mbMenu))
      result = VIEW_WEB_DAILY;
    else if ("smsx_web_regions".equals(mbMenu))
      result = VIEW_WEB_REGIONS;
    else if ("smsx_traffic".equals(mbMenu))
      result = VIEW_TRAFFIC;
    mbMenu = null;
    return result;
  }

  public String getMbMenu() {
    return mbMenu;
  }

  public void setMbMenu(String mbMenu) {
    this.mbMenu = mbMenu;
  }

  public SponsoredRequest.Status[] getRequestStatuses() {
    SponsoredRequest.Status[] ss = SponsoredRequest.Status.values();
    Arrays.sort(ss, new Comparator() {
      public int compare(Object o1, Object o2) {
        SponsoredRequest.Status s1 = (SponsoredRequest.Status)o1;
        SponsoredRequest.Status s2 = (SponsoredRequest.Status)o2;
        return s1.toString().compareTo(s2.toString());
      }
    });
    return ss;
  }

}
