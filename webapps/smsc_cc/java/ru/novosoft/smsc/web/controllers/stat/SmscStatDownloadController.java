package ru.novosoft.smsc.web.controllers.stat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.stat.*;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;
import ru.novosoft.smsc.web.controllers.reschedule.SmppStatusConverter;

import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * author: Aleksandr Khalitov
 */
public class SmscStatDownloadController extends SmscController{

  private SmscStatFilter filter = new SmscStatFilter();

  private boolean includeErrors;

  private Statistics statistics;

  private final SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
  private final SimpleDateFormat statDateformat = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss");


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

  public String getStatDate() {
    return statDateformat.format(new Date());
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    try {
      statistics = WebContext.getInstance().getSmscStatProvider().getStatistics(filter);
    } catch (AdminException e) {
      addError(e);
      return;
    }
    Locale locale = getLocale();
    super._download(writer);
    exportStatistics(writer, locale);
  }

  private final static char COL_SEP = ',';
  private final static String CAPTION_STR = "Eyeline SMSC statistics report,,,,,,,,";
  private final static String GEN_TIME_STR = "Generated at:,";
  private final static String FROM_STR = "From:,";
  private final static String TILL_STR = "Till:,";
  private final static String GEN_STAT_STR = "General statistics,,,,,,,,";
  private final static String GEN_CAP_STR = ",Accepted,Rejected,Delivered,Failed,Rescheduled,Temporal,Peak in,Peak out";
  private final static String GEN_TOTAL_STR = "Total SMS processed:,";
  private final static String SME_STAT_STR = "SME activity,,,,,,,,";
  private final static String RT_STAT_STR = "Traffic by routes,,,,,,,,";
  private final static String ERR_STAT_STR = "Error,Count,,,,,,,";
  private final static String ST_STAT_STR = "SMS delivery state,,,,,,,,";

  private void exportCounters(PrintWriter out, CountersSet set) throws IOException {
    out.print(set.getAccepted());
    out.print(COL_SEP);
    out.print(set.getRejected());
    out.print(COL_SEP);
    out.print(set.getDelivered());
    out.print(COL_SEP);
    out.print(set.getFailed());
    out.print(COL_SEP);
    out.print(set.getRescheduled());
    out.print(COL_SEP);
    out.print(set.getTemporal());
    out.print(COL_SEP);
    out.print(set.getPeak_i());
    out.print(COL_SEP);
    out.print(set.getPeak_o());
    out.println();
  }

  private void exportErrors(PrintWriter out, ExtendedCountersSet set, Locale locale) throws IOException {
    if (!includeErrors) return;

    out.println(ERR_STAT_STR);
    for (ErrorCounterSet s : set.getErrors()) {
      String errMessage = SmppStatusConverter.getAsString(s.getErrcode(), locale);
      errMessage = errMessage + " (" + s.getErrcode() + ')';
      out.print('\"' + errMessage + '\"');
      out.print(COL_SEP);
      out.print(s.getCounter());
      out.print(COL_SEP);
      out.print(COL_SEP);
      out.print(COL_SEP);
      out.print(COL_SEP);
      out.print(COL_SEP);
      out.print(COL_SEP);
      out.println(COL_SEP);
    }
  }


  private void printHeader(PrintWriter out) throws IOException{
    out.println(CAPTION_STR);
    out.print(GEN_TIME_STR);
    out.print(convertDateToString(new Date()));
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.println(COL_SEP);
    out.println();
    out.print(FROM_STR);
    String date = filter.getFrom() != null ? convertDateToString(filter.getFrom()) : "-";
    out.print(date);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.println(COL_SEP);
    out.print(TILL_STR);
    date = filter.getTill() != null ? convertDateToString(filter.getTill()) : "-";
    out.print(date);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.print(COL_SEP);
    out.println(COL_SEP);
    out.println();
  }

  private void exportStatistics(PrintWriter out, Locale locale) throws IOException{
    // Header
    printHeader(out);
    // General statistics
    out.println(GEN_STAT_STR);
    out.println(GEN_CAP_STR);
    out.print(GEN_TOTAL_STR);
    final CountersSet total = statistics;
    exportCounters(out, total);
    for (DateCountersSet s : statistics.getGeneralStat()) {
      String date = formatter.format(s.getDate());
      out.print(date);
      out.print(COL_SEP);
      exportCounters(out, s);

      for (HourCountersSet h : s.getHourStat()) {
        out.print(h.getHour());
        out.print(COL_SEP);
        exportCounters(out, h);
      }
    }

    // SME activity
    out.println();
    out.println(SME_STAT_STR);
    out.println("SME Id" + GEN_CAP_STR);
    for (SmeIdCountersSet s : statistics.getSmeIdStat()) {
      out.print('\"' + s.getSmeid() + '\"');
      out.print(COL_SEP);
      exportCounters(out, s);
      exportErrors(out, s, locale);
    }

    // Traffic by routes
    out.println();
    out.println(RT_STAT_STR);
    out.println("Route Id" + GEN_CAP_STR);
    for (RouteIdCountersSet s : statistics.getRouteIdStat()) {
      out.print('\"' + s.getRouteid() + '\"');
      out.print(COL_SEP);
      exportCounters(out, s);
      exportErrors(out, s, locale);
    }

    if (includeErrors) {
      // SMS delivery state
      out.println();
      out.println(ST_STAT_STR);
      exportErrors(out, statistics, locale);
    }
  }


  private String convertDateToString(Date date) {
    return formatter.format(date);
  }
}
