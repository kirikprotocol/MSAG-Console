package ru.sibinco.smsx.stats.beans;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.PageBean;
import ru.sibinco.smsx.stats.SmsxContext;
import ru.sibinco.smsx.stats.backend.SmsxRequest;

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

  protected Category logger = Category.getInstance(SmsxContext.class);

  protected SmsxContext smsxContext;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    smsxContext = SmsxContext.getInstance();
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

  public SmsxRequest.Status[] getRequestStatuses() {
    SmsxRequest.Status[] ss = SmsxRequest.Status.values();
    Arrays.sort(ss, new Comparator() {
      public int compare(Object o1, Object o2) {
        SmsxRequest.Status s1 = (SmsxRequest.Status)o1;
        SmsxRequest.Status s2 = (SmsxRequest.Status)o2;
        return s1.toString().compareTo(s2.toString());
      }
    });
    return ss;
  }

  private static final String[] allSmsxSerivices = new String[]{"0","1","2","3","4","5","6","7",};

  public String[] getAllServices() {
    return allSmsxSerivices;
  }

}
