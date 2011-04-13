package ru.sibinco.smsx.stats.beans;

import ru.sibinco.smsx.stats.backend.SmsxRequest;
import ru.sibinco.smsx.stats.backend.SmsxRequestPrototype;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class CreateSmsxRequestBean extends StatsBean{

  public static final int RESULT_SAVE = StatsBean.PRIVATE_RESULT;
  public static final int RESULT_CANCEL = StatsBean.PRIVATE_RESULT+1;

  private String mbApply;
  private String mbCancel;

  private String from = "";

  private String till = "";

  private String[] serviceId;

  private String[] reports;


  protected int init(List errors) {
    return super.init(errors);
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if(result != RESULT_OK) {
      return result;
    }

    if (mbApply != null) {
      mbApply = null;
      return save();
    } else if (mbCancel != null) {
      mbCancel = null;
      return RESULT_CANCEL;
    }

    return RESULT_OK;
  }


  private int save() {
    SmsxRequestPrototype prototype = new SmsxRequestPrototype();
    if(serviceId != null) {
      Set sId = new HashSet(serviceId.length);
      for(int i=0; i<serviceId.length;i++) {
        sId.add(Integer.valueOf(serviceId[i]));
      }
      prototype.setServiceIdFilter(sId);
    }else {
      prototype.setServiceIdFilter(new HashSet());
    }
    if(reports != null) {
      Set rs = new HashSet(reports.length);
      for(int i=0; i<reports.length;i++) {
        rs.add(SmsxRequest.ReportType.valueOf(reports[i]));
      }
      prototype.setReportTypeFilter(rs);
    }else {
      prototype.setReportTypeFilter(new HashSet());
    }
    SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");
    try{
      if(from != null && from.length()>0) {
        prototype.setFrom(sdf.parse(from));
      }
      if(till != null && till.length()>0) {
        prototype.setTill(sdf.parse(till));
      }
    }catch (ParseException e){
      logger.error(e,e);
      return error(e.getMessage());
    }
    try {
      smsxContext.getStatRequestManager().createRequest(prototype);
    } catch (StatisticsException e) {
      logger.error(e,e);
      return error(e.getMessage());
    }
    return RESULT_SAVE;

  }


  public SmsxRequest.ReportType[] getAllReports() {
    SmsxRequest.ReportType[] rs = SmsxRequest.ReportType.values();
    Arrays.sort(rs, new Comparator() {
      public int compare(Object o1, Object o2) {
        SmsxRequest.ReportType r1 = (SmsxRequest.ReportType)o1;
        SmsxRequest.ReportType r2 = (SmsxRequest.ReportType)o2;
        return r1.toString().compareTo(r2.toString());
      }
    });
    return rs;
  }

  public boolean isServiceIsChecked(String service) {
    if(serviceId != null) {
      for(int i=0;i<serviceId.length;i++) {
        if(serviceId[i].equals(service)) {
          return true;
        }
      }
    }
    return false;
  }

  public boolean isReportIsChecked(String report) {
    if(reports != null) {
      for(int i=0;i<reports.length;i++) {
        if(reports[i].equals(report)) {
          return true;
        }
      }
    }
    return false;
  }

  public String[] getServiceId() {
    return serviceId;
  }

  public void setServiceId(String[] serviceId) {
    this.serviceId = serviceId;
  }

  public String[] getReports() {
    return reports;
  }

  public void setReports(String[] reports) {
    this.reports = reports;
  }

  public String getFrom() {
    return from;
  }

  public void setFrom(String from) {
    this.from = from;
  }

  public String getTill() {
    return till;
  }

  public void setTill(String till) {
    this.till = till;
  }

  public String getMbApply() {
    return mbApply;
  }

  public void setMbApply(String mbApply) {
    this.mbApply = mbApply;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }
}
