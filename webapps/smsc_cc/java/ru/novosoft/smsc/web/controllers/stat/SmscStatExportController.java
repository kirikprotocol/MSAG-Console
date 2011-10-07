package ru.novosoft.smsc.web.controllers.stat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.stat.CountersSet;
import ru.novosoft.smsc.admin.stat.SmscStatFilter;
import ru.novosoft.smsc.admin.stat.Statistics;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class SmscStatExportController extends SmscController{

  private SmscStatFilter filter = new SmscStatFilter();

  private boolean includeErrors;

  private Statistics statistics;


  public String start() {
    try {
      statistics = WebContext.getInstance().getSmscStatProvider().getStatistics(filter);
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public Date getFrom() {
    return filter.getFrom();
  }

  public void setFrom(Date from) {
    filter.setFrom(from);
  }

  public Date getTill() {
    return filter.getTill();
  }

  public void setTill(Date till) {
    filter.setTill(till);
  }

  public boolean isIncludeErrors() {
    return includeErrors;
  }

  public void setIncludeErrors(boolean includeErrors) {
    this.includeErrors = includeErrors;
  }

  private void downloadGeneral(PrintWriter writer) throws IOException {
    writer.println("DATE,HOUR,ACCEPTED,REJECTED,DELIVERED,FAILED,RESCHEDULED,TEMPORAL,PEAK_IN,PEAK_OUT");

//    CSVTokenizer.es

  }

  private void statToCsv(StringBuilder sb, CountersSet d){
    sb.append(d.getAccepted());
    sb.append(';');
    sb.append(d.getRejected());
    sb.append(';');
    sb.append(d.getDelivered());
    sb.append(';');
    sb.append(d.getFailed());
    sb.append(';');
    sb.append(d.getRescheduled());
    sb.append(';');
    sb.append(d.getTemporal());
    sb.append(';');
    sb.append(d.getPeak_i());
    sb.append(';');
    sb.append(d.getPeak_o());
  }

  private void downloadSMEs(PrintWriter writer) throws IOException{
//    SimpleDateFormat sdf = new SimpleDateFormat("dd-MMM-yyyy");
//    for(DateCountersSet d : statistics.getGeneralStat()) {
//      StringBuilder sb = new StringBuilder();
//      CSVTokenizer.escape(sb, sdf.format(d.getDate()));
//      sb.append(';');
//      sb.append(';');
//      statToCsv(sb, d);
//      writer.println(sb);
//      for(HourCountersSet h : d.getHourStat()) {
//        sb.delete(0, sb.length());
//        sb.append(';');
//        sb.append(h.getHour());
//        sb.append(';');
//        statToCsv(sb, d);
//        writer.println(sb);
//      }
//    }
  }

  private void downloadRoutes(PrintWriter writer) throws IOException{

  }

  private void downloadErrors(PrintWriter writer) throws IOException{

  }



  @Override
  protected void _download(PrintWriter writer) throws IOException {
    super._download(writer);

  }
}
