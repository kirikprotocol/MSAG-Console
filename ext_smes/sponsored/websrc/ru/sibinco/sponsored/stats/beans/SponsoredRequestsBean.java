package ru.sibinco.sponsored.stats.beans;

import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.sibinco.sponsored.stats.backend.SponsoredRequest;
import ru.sibinco.sponsored.stats.backend.StatisticsException;
import ru.sibinco.sponsored.stats.backend.Visitor;
import ru.sibinco.sponsored.stats.backend.datasource.SponsoredConverter;
import ru.sibinco.sponsored.stats.backend.datasource.SponsoredRecord;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.jsp.JspWriter;
import java.util.Iterator;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredRequestsBean extends StatsBean{

  private RequestFilter filter = new RequestFilter();

  public static final int RESULT_RESULTS = StatsBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = StatsBean.PRIVATE_RESULT + 1;
  public static final int RESULT_DOWNLOAD = StatsBean.PRIVATE_RESULT + 2;
  protected static final int PRIVATE_RESULT = StatsBean.PRIVATE_RESULT + 3;

  private String mbAdd = null;
  private String mbDelete = null;

  private Integer selected;

  private RequestsTableHelper tableHelper;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    tableHelper = new SponsorRequestsTableHelper("sponsor-requests", sponsoredContext.getStatRequestManager(), filter);
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    tableHelper.setLocale(sponsoredContext.getUserLocale(request.getUserPrincipal()));
    try {
      tableHelper.processRequest(request);
    } catch (TableHelperException e) {
      return error(e.getMessage());
    }

    if (mbAdd != null) {
      mbAdd = null;
      return RESULT_ADD;
    }
    if (mbDelete != null) {
      mbDelete = null;
      int r = delete(request);
      if(r != RESULT_OK) {
        return r;
      }
    }

    String selectedCell = tableHelper.getSelectedCellId();
    if(selectedCell != null) {
      if(selectedCell.startsWith("download_")) {
        selected = Integer.valueOf(selectedCell.substring("download_".length()));
        return RESULT_DOWNLOAD;
      }else if(selectedCell.startsWith("cancel_")) {
        int requestId = Integer.parseInt(selectedCell.substring("cancel_".length()));
        int r = cancel(requestId);
        if(r != RESULT_OK) {
          return r;
        }
      }else {
        selected = Integer.valueOf(selectedCell);
        return RESULT_RESULTS;
      }
    }

    try {
      tableHelper.fillTable();
    } catch (TableHelperException e) {
      logger.error(e,e);
      return error(e.getMessage());
    }
    return RESULT_OK;
  }

  public void download(final JspWriter writer) {
    try{
      writer.clear();
      writer.write("ADDRESS,BONUS");
      writer.write(System.getProperty("line.separator"));
      sponsoredContext.getStatRequestManager().getSponsoredResult(selected.intValue(), new Visitor() {
        public boolean visit(Object o) throws StatisticsException {
          SponsoredRecord r = (SponsoredRecord)o;
          SponsoredConverter.write(writer, r.getAddress(), r.getBonus());
          return true;
        }
      });
      writer.flush();
      selected = null;
    } catch (Exception e) {
      logger.error(e,e);
    }
  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }

  private int cancel(int requestId) {
    try {
      sponsoredContext.getStatRequestManager().cancelRequest(requestId);
      return RESULT_OK;
    } catch (StatisticsException e) {
      logger.error(e,e);
      return error(e.getMessage());
    }
  }


  private int delete(HttpServletRequest r) {
    List selected = tableHelper.getSelectedRequests(r);
    if(!selected.isEmpty()) {
      Iterator i = selected.iterator();
      while(i.hasNext()) {
        Integer reqId = (Integer)i.next();
        try {
          sponsoredContext.getStatRequestManager().removeRequest(reqId.intValue());
        } catch (StatisticsException e) {
          logger.error(e,e);
          return error(e.getMessage());
        }
      }
    }
    return RESULT_OK;
  }

  public SponsoredRequest getSponsorRequest() {

    if(selected != null) {
      try {
        return (SponsoredRequest) sponsoredContext.getStatRequestManager().getRequest(selected.intValue());
      } catch (StatisticsException e) {
        logger.error(e,e);
      }
    }
    return null;
  }

  public String getStatus() {
    return filter.getStatus() != null ? filter.getStatus().toString() : null;
  }

  public void setStatus(String status) {
    if(status == null || status.length() == 0) {
      filter.setStatus(null);
    }else {
      filter.setStatus(SponsoredRequest.Status.valueOf(status));
    }
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete() {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  public String getSelected() {
    return selected == null ? null : selected.toString();
  }

  public void setSelected(String selected) {
    this.selected = selected == null || selected.length() == 0 ? null : Integer.valueOf(selected);
  }

}
