package ru.sibinco.smsx.stats.beans;

import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.sibinco.smsx.stats.backend.SmsxRequest;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.jsp.JspWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxRegionsBean extends StatsBean{

  public static final int RESULT_RESULTS = StatsBean.PRIVATE_RESULT;
  public static final int RESULT_BACK = StatsBean.PRIVATE_RESULT + 1;
  public static final int RESULT_DOWNLOAD = StatsBean.PRIVATE_RESULT + 2;
  protected static final int PRIVATE_RESULT = StatsBean.PRIVATE_RESULT + 3;

  private Integer requestId;

  private Date from;

  private Date till;

  private SmsxRegionsTableHelper tableHelper;

  private final SmsxWebFilter filter = new SmsxWebFilter();

  private String mbBack;
  private String mbDownload;

  private Set reports;

  protected int init(List errors) {
    int result =  super.init(errors);
    if(result != RESULT_OK) {
      return result;
    }
    if(requestId == null) {
      return error("Request is not found: id=null");
    }
    SmsxRequest smsxRequest;
    try{
      smsxRequest= smsxContext.getStatRequestManager().getRequest(requestId.intValue());
    }catch (StatisticsException e){
      logger.error(e,e);
      return error(e.getMessage());
    }
    if(smsxRequest == null) {
      return error("Request is not found: id="+requestId);
    }
    from = smsxRequest.getFrom();
    till = smsxRequest.getTill();
    reports = new HashSet(smsxRequest.getReportTypesFilter());

    tableHelper = new SmsxRegionsTableHelper("smsx_regions", smsxContext.getStatRequestManager(), filter, smsxRequest.getId());
    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if(mbBack != null) {
      mbBack = null;
      return RESULT_BACK;
    }


    tableHelper.setLocale(smsxContext.getUserLocale(request.getUserPrincipal()));
    try {
      tableHelper.processRequest(request);
      tableHelper.fillTable();
    } catch (Exception e) {
      e.printStackTrace();
      return error(e.getMessage());
    }
    if (mbDownload != null) {
      mbDownload = null;
      return RESULT_DOWNLOAD;
    }


    return RESULT_OK;
  }

  public Set getReports() {
    return reports == null ? new HashSet() : reports;
  }

  public void download(final HttpServletRequest servletRequest, final JspWriter out) {
    try {
      out.clear();
      if(requestId != null) {
        new SmsxWriter(smsxContext.getStatRequestManager()).
            writeWebRegions(out, requestId.intValue(), filter, smsxContext.getUserLocale(servletRequest.getUserPrincipal()));
      }
      out.flush();
    } catch (Exception e) {
      logger.error(e, e);
    }
  }

  public Integer getRequestId() {
    return requestId;
  }

  public void setRequestId(Integer requestId) {
    this.requestId = requestId;
  }

  public SmsxRegionsTableHelper getTableHelper() {
    return tableHelper;
  }

  private final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");

  public String getFrom() {
    return from == null ? "" : sdf.format(from);
  }

  public String getTill() {
    return till == null ? "" : sdf.format(till);
  }

  public Collection getRegions() {
    if(requestId == null || tableHelper == null) {
      return Collections.EMPTY_LIST;
    }
    return tableHelper.getRegions();
  }

  public String getRegion() {
    return filter.getRegion();
  }

  public void setRegion(String region) {
    filter.setRegion(region != null && region.length()>0 ? region : null);
  }

  public String getMbBack() {
    return mbBack;
  }

  public void setMbBack(String mbBack) {
    this.mbBack = mbBack;
  }

  public String getMbDownload() {
    return mbDownload;
  }

  public void setMbDownload(String mbDownload) {
    this.mbDownload = mbDownload;
  }

  public void setMsc(String msc) {
    filter.setMsc(msc == null || msc.length() == 0 ? null : Boolean.valueOf(msc));
  }

  public String getMsc() {
    return filter.getMsc() == null ? null : filter.getMsc().toString();
  }
}
