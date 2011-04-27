package ru.sibinco.smsx.stats.beans;

import ru.sibinco.smsx.stats.backend.SmsxResults;
import ru.sibinco.smsx.stats.backend.StatRequestManager;
import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;
import ru.sibinco.smsx.stats.backend.datasource.SmsxUsers;
import ru.sibinco.smsx.stats.backend.datasource.Traffic;
import ru.sibinco.smsx.stats.backend.datasource.WebDaily;
import ru.sibinco.smsx.stats.backend.datasource.WebRegion;

import java.io.IOException;
import java.io.Writer;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
class SmsxWriter {

  private final StatRequestManager requestManager;

  SmsxWriter(StatRequestManager requestManager) {
    this.requestManager = requestManager;
  }

  void writeSmsxUsers(final Writer out, int requestId, final SmsxUsersFilter filter, Locale locale) throws StatisticsException {
    SmsxResults rs = requestManager.getSmsxResult(requestId);
    if(rs != null) {
      final String lineSep = System.getProperty("line.separator");
      final ResourceBundle bundle = ResourceBundle.getBundle("locales.messages", locale);
      final String comma = ",";
      try{
        out.write("REGION,SERVICE_ID,SERVICE_NAME,COUNT");
        out.write(lineSep);
        rs.getSmsxUsers(new Visitor() {
          public boolean visit(Object o) throws StatisticsException {
            SmsxUsers u = (SmsxUsers)o;
            if(filter.isAllowed(u)) {
              try{
                out.write(u.getRegion());
                out.write(comma);
                out.write(Integer.toString(u.getServiceId()));
                out.write(comma);
                out.write(bundle.getString("smsx.service."+u.getServiceId()));
                out.write(comma);
                out.write(Integer.toString(u.getCount()));
                out.write(lineSep);
              }catch (IOException e){
                throw new StatisticsException(e);
              }
            }
            return true;
          }
        });
      }catch (IOException e){
        throw new StatisticsException(e);
      }
    }
  }

  void writeWebDaily(final Writer out, int requestId, final SmsxWebFilter filter, Locale locale) throws StatisticsException{
    SmsxResults rs =requestManager.getSmsxResult(requestId);
    if(rs != null) {
      try {
        final String lineSep = System.getProperty("line.separator");
        final ResourceBundle bundle = ResourceBundle.getBundle("locales.messages", locale);
        final String comma = ",";
        out.write("DATE,REGION,SERVICE,TRAFFIC_TYPE,COUNT");
        out.write(lineSep);
        rs.getWebDaily(new Visitor() {
          public boolean visit(Object o) throws StatisticsException {
            WebDaily u = (WebDaily)o;
            if(filter.isAllowed(u)) {
              try{
                out.write(u.getDate());
                out.write(comma);
                out.write(u.getRegion());
                out.write(comma);
                out.write(bundle.getString("smsx.service.SMS_SITE"));
                out.write(comma);
                out.write(bundle.getString(u.isMsc() ? "smsx.traffic.paid" : "smsx.traffic.free"));
                out.write(comma);
                out.write(Integer.toString(u.getCount()));
                out.write(lineSep);
              }catch (IOException e){
                throw new StatisticsException(e);
              }
            }
            return true;
          }
        });
      } catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
  }

  void writeWebRegions(final Writer out, int requestId, final SmsxWebFilter filter, Locale locale) throws StatisticsException{
    SmsxResults rs = requestManager.getSmsxResult(requestId);
    if(rs != null) {
      try {
        final String lineSep = System.getProperty("line.separator");
        final ResourceBundle bundle = ResourceBundle.getBundle("locales.messages", locale);
        final String comma = ",";
        out.write("REGION,SERVICE,TRAFFIC_TYPE,SRC_COUNT,DST_COUNT");
        out.write(lineSep);
        rs.getWebRegions(new Visitor() {
          public boolean visit(Object o) throws StatisticsException {
            WebRegion u = (WebRegion)o;
            if(filter.isAllowed(u)) {
              try{
                out.write(u.getRegion());
                out.write(comma);
                out.write(bundle.getString("smsx.service.SMS_SITE"));
                out.write(comma);
                out.write(bundle.getString(u.isMsc() ? "smsx.traffic.paid" : "smsx.traffic.free"));
                out.write(comma);
                out.write(Integer.toString(u.getSrcCount()));
                out.write(comma);
                out.write(Integer.toString(u.getDstCount()));
                out.write(lineSep);
              }catch (IOException e){
                throw new StatisticsException(e);
              }
            }
            return true;
          }
        });
      } catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
  }

  void writeTraffic(final Writer out, int requestId, final SmsxTrafficFilter filter, Locale locale) throws StatisticsException{
    SmsxResults rs = requestManager.getSmsxResult(requestId);
    if(rs != null) {
      try {
        final String lineSep = System.getProperty("line.separator");
        final ResourceBundle bundle = ResourceBundle.getBundle("locales.messages", locale);
        final String comma = ",";
        out.write("REGION,SERVICE_ID,SERVICE_NAME,TRAFFIC_TYPE,COUNT");
        out.write(lineSep);
        rs.getTraffic(new Visitor() {
          public boolean visit(Object o) throws StatisticsException {
            Traffic u = (Traffic)o;
            if(filter.isAllowed(u)) {
              try{
                out.write(u.getRegion());
                out.write(comma);
                out.write(Integer.toString(u.getServiceId()));
                out.write(comma);
                out.write(bundle.getString("smsx.service."+u.getServiceId()));
                out.write(comma);
                out.write(bundle.getString(u.isMsc() ? "smsx.traffic.paid" : "smsx.traffic.free"));
                out.write(comma);
                out.write(Integer.toString(u.getCount()));
                out.write(lineSep);
              }catch (IOException e){
                throw new StatisticsException(e);
              }
            }
            return true;
          }
        });
      } catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
  }



}
