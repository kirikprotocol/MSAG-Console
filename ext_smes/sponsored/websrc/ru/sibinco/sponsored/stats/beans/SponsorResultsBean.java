package ru.sibinco.sponsored.stats.beans;

import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.sibinco.sponsored.stats.backend.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.jsp.JspWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SponsorResultsBean extends StatsBean{

  public static final int RESULT_RESULTS = StatsBean.PRIVATE_RESULT;
  public static final int RESULT_BACK = StatsBean.PRIVATE_RESULT + 1;
  public static final int RESULT_DOWNLOAD = StatsBean.PRIVATE_RESULT + 2;
  protected static final int PRIVATE_RESULT = StatsBean.PRIVATE_RESULT + 3;

  private Integer requestId;

  private Date from;

  private Date till;

  private SponsorResultsTableHelper tableHelper;

  private SponsoredResultFilter filter = new SponsoredResultFilter();

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
    SponsoredRequest sponsoredRequest = sponsoredContext.getStatRequestManager().getRequest(requestId.intValue());
    if(sponsoredRequest == null) {
      return error("Request is not found: id="+requestId);
    }
    from = sponsoredRequest.getFrom();
    till = sponsoredRequest.getTill();

    tableHelper = new SponsorResultsTableHelper("sponsored_results", sponsoredContext.getStatRequestManager(), filter, sponsoredRequest.getId());
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


    tableHelper.setLocale(sponsoredContext.getUserLocale(request.getUserPrincipal()));
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

  public void download(final JspWriter out) {
    try {
      out.clear();
      if(requestId != null) {
        out.write("ADDRESS,BONUS");
        out.write(System.getProperty("line.separator"));
        sponsoredContext.getStatRequestManager().getSponsoredResult(requestId.intValue(), new Visitor() {
          public boolean visit(Object o) throws StatisticsException {
            SponsoredRecord r = (SponsoredRecord) o;
            SponsoredConverter.write(out, r.getAddress(), r.getBonus());
            return true;
          }
        });
      }
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

  public SponsorResultsTableHelper getTableHelper() {
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
    return sponsoredContext.getRegions();
  }

  public String getAddress() {
    return filter.getAddressPrefix() == null ? "" : filter.getAddressPrefix();
  }

  public void setAddress(String address) {
    filter.setAddressPrefix(address != null && address.length()>0 ? address : null);
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

