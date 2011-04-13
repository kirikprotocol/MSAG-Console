package ru.sibinco.smsx.stats.beans;

import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.sibinco.smsx.stats.backend.SmsxRequest;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.*;
import java.security.Principal;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxRequestsBean extends StatsBean{

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
    tableHelper = new SmsxRequestsTableHelper("smsx-requests", filter, smsxContext.getStatRequestManager());
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    tableHelper.setLocale(smsxContext.getUserLocale(request.getUserPrincipal()));
    try {
      tableHelper.processRequest(request);
      tableHelper.fillTable();
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

  private File createSmsxUsers(String user, Locale locale) throws IOException, StatisticsException {
    File file = new File(smsxContext.getWorkDir(), "smsxusers-"+user+System.currentTimeMillis()+".csv");
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "windows-1251")));
      new SmsxWriter(smsxContext.getStatRequestManager()).
          writeSmsxUsers(writer, selected.intValue(), new SmsxUsersFilter(), locale);
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
    return file;
  }

  private File createWebDaily(String user, Locale locale) throws IOException, StatisticsException{
    File file = new File(smsxContext.getWorkDir(), "webdaily-"+user+System.currentTimeMillis()+".csv");
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "windows-1251")));
      new SmsxWriter(smsxContext.getStatRequestManager()).
          writeWebDaily(writer, selected.intValue(), new SmsxWebFilter(), locale);
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
    return file;
  }

  private File createWebRegions(String user, Locale locale) throws IOException, StatisticsException {
    File file = new File(smsxContext.getWorkDir(), "webregions-"+user+System.currentTimeMillis()+".csv");
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "windows-1251")));
      new SmsxWriter(smsxContext.getStatRequestManager()).
          writeWebRegions(writer, selected.intValue(), new SmsxWebFilter(), locale);
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
    return file;
  }

  private File createTraffic(String user, Locale locale) throws IOException, StatisticsException{
    File file = new File(smsxContext.getWorkDir(), "traffic-"+user+System.currentTimeMillis()+".csv");
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "windows-1251")));
      new SmsxWriter(smsxContext.getStatRequestManager()).
          writeTraffic(writer, selected.intValue(), new SmsxTrafficFilter(), locale);
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
    return file;
  }

  private void createZip(HttpServletRequest servletRequest, OutputStream os, Locale locale) throws IOException, StatisticsException {
    SmsxRequest r = (SmsxRequest)smsxContext.getStatRequestManager().getRequest(selected.intValue());
    Principal p = servletRequest.getUserPrincipal();
    String user = p == null ? "" : p.getName();
    List files = new ArrayList(4);
    if(r.getReportTypesFilter().contains(SmsxRequest.ReportType.SMSX_USERS)) {
      files.add(createSmsxUsers(user, locale));
    }
    if(r.getReportTypesFilter().contains(SmsxRequest.ReportType.TRAFFIC)) {
      files.add(createTraffic(user, locale));
    }
    if(r.getReportTypesFilter().contains(SmsxRequest.ReportType.WEB_DAILY)) {
      files.add(createWebDaily(user, locale));
    }
    if(r.getReportTypesFilter().contains(SmsxRequest.ReportType.WEB_REGIONS)) {
      files.add(createWebRegions(user, locale));
    }
    ZipOutputStream zos = null;
    try{
      zos = new ZipOutputStream(os);
      Iterator i = files.iterator();
      while(i.hasNext()) {
        File f = (File)i.next();
        ZipEntry zipEntry = new ZipEntry(f.getName().substring(0, f.getName().indexOf("-"))+".csv");
        zos.putNextEntry(zipEntry);
        BufferedInputStream inputStream = null;
        try{
          inputStream = new BufferedInputStream(new FileInputStream(f));
          int n;
          byte[] buffer = new byte[4096];
          while ((n = inputStream.read(buffer)) != -1) {
            zos.write(buffer, 0, n);
          }
        }finally {
          if(inputStream != null) {
            try{
              inputStream.close();
            }catch (IOException ignored){}
          }
        }
        zos.closeEntry();
        f.delete();
      }
    }finally {
      if(zos != null) {
        try {
          zos.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    }
  }

  public void download(HttpServletRequest request, HttpServletResponse response) {

    Locale locale = smsxContext.getUserLocale(request.getUserPrincipal());

    OutputStream os = null;
    try{
      os = response.getOutputStream();
      createZip(request, os, locale);
    } catch (Exception e) {
      logger.error(e,e);
    }finally {
      if(os != null) {
        try {
          os.close();
        } catch (IOException ignored) {}
      }
    }
  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }

  private int cancel(int requestId) {
    try {
      smsxContext.getStatRequestManager().cancelRequest(requestId);
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
          smsxContext.getStatRequestManager().removeRequest(reqId.intValue());
        } catch (StatisticsException e) {
          logger.error(e,e);
          return error(e.getMessage());
        }
      }
    }
    return RESULT_OK;
  }

  public SmsxRequest getSmsxRequest() {

    if(selected != null) {
      try {
        return (SmsxRequest)smsxContext.getStatRequestManager().getRequest(selected.intValue());
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
      filter.setStatus(SmsxRequest.Status.valueOf(status));
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
