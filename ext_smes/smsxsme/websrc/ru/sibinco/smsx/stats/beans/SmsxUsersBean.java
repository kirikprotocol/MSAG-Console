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
public class SmsxUsersBean extends StatsBean{

  public static final int RESULT_RESULTS = StatsBean.PRIVATE_RESULT;
  public static final int RESULT_BACK = StatsBean.PRIVATE_RESULT + 1;
  public static final int RESULT_DOWNLOAD = StatsBean.PRIVATE_RESULT + 2;
  protected static final int PRIVATE_RESULT = StatsBean.PRIVATE_RESULT + 3;

  private Integer requestId;

  private Date from;

  private Date till;

  private SmsxUsersTableHelper tableHelper;

  private final SmsxUsersFilter filter = new SmsxUsersFilter();

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

    tableHelper = new SmsxUsersTableHelper("smsx_users", smsxContext.getStatRequestManager(), filter, smsxRequest.getId());
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
    } catch (TableHelperException e) {
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
            writeSmsxUsers(out, requestId.intValue(), filter, smsxContext.getUserLocale(servletRequest.getUserPrincipal()));
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

  public SmsxUsersTableHelper getTableHelper() {
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
    return smsxContext.getRegions();
  }


  public String getServiceId() {
    return filter.getServiceId() == null ? null : filter.getServiceId().toString();
  }

  public void setServiceId(String serviceId) {
    filter.setServiceId((serviceId != null && serviceId.length()>0) ? Integer.valueOf(serviceId) : null);
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
}

