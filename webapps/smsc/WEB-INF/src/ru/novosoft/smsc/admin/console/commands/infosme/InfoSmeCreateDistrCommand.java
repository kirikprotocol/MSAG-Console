package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.console.CommandContext;

import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 *
 * User: alkhal
 * Date: 22.10.2008
 *
 */

public class InfoSmeCreateDistrCommand extends CommandClass {


  private SimpleDateFormat dateFormat;

  private String timeFormatDelim;

  private String sourceAddress;

  private String file;

  private String dateBeginStr;

  private String dateEndStr;

  private String timeBeginStr;

  private String timeEndStr;

  private String dayStr;

  private final String[] weekDays = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

  private String txmode;
    // infosme create distr "/home/alkhal/file.txt" "10.08.2010 10:56" "10.08.2010 12:56" "10:45" "10:57" "Mon,Tue" "rx" "144"

  public InfoSmeCreateDistrCommand() {
    super();
    dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
    timeFormatDelim = ":";
  }

  public void process(CommandContext ctx) {
    try {
      dateFormat.parse(dateBeginStr);
      dateFormat.parse(dateEndStr);
      validateTime(timeBeginStr);
      validateTime(timeEndStr);
      validateDays();
      Boolean bolMod = Boolean.valueOf(txmode);

      final InfoSmeCreateDistr cmd = (InfoSmeCreateDistr)Class.forName("ru.novosoft.smsc.infosme.backend.commands.InfoSmeCreateDistrImpl").newInstance();

      HashMap params = new HashMap(7);
      params.put("dateBegin", dateBeginStr);
      params.put("dateEnd", dateEndStr);
      params.put("timeBegin", timeBeginStr);
      params.put("timeEnd", timeEndStr);
      params.put("days",dayStr);
      params.put("txmode",bolMod);
      params.put("address", sourceAddress);

      cmd.createDistribution(ctx, file, dateBeginStr, dateEndStr,
          timeBeginStr, timeEndStr, dayStr, Boolean.valueOf(txmode), sourceAddress);
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
      ctx.setMessage("Can't find module InfoSme");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    } catch (IllegalAccessException e) {
      e.printStackTrace();
      ctx.setMessage("Can't find module InfoSme");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    } catch (InstantiationException e) {
      e.printStackTrace();
      ctx.setMessage("Can't find module InfoSme");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    } catch (Exception e) {
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      ctx.setMessage(e.getMessage());
    }


  }

  public String getId() {
    return "INFOSME_CREATE_DISTR";
  }

  private void validateDays() throws ParseException, NoSuchElementException{
    StringTokenizer tokenizer = new StringTokenizer(dayStr,",");
    while(tokenizer.hasMoreTokens()){
      String token = tokenizer.nextToken();
      if((token==null)||(token.equals(""))) {
        throw new NoSuchElementException("Some day is null");
      }
      int fl=0;
      for(int i=0;i<weekDays.length;i++) {
        if(weekDays[i].equals(token)) {
          fl=1;
          break;
        }
      }
      if(fl==0) {
        throw new ParseException("Unsupported day format: "+token,0);
      }
    }
  }

  private void validateTime(String str) throws ParseException, NoSuchElementException {
    StringTokenizer tokenizer = new StringTokenizer(str,timeFormatDelim);
    String token = tokenizer.nextToken();
    if((token==null)||(token.equals(""))) {
      throw new NoSuchElementException("Hour is null");
    }
    Integer.parseInt(token);
    if((token=tokenizer.nextToken())==null) {
      throw new NoSuchElementException("Minutes is null");
    }
    Integer.parseInt(token);
  }



  public void setTxmode(String txmode) {
    this.txmode = txmode;
  }

  public String getTxmode() {
    return txmode;
  }


  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public String getDateBeginStr() {
    return dateBeginStr;
  }

  public void setDateBeginStr(String dateBeginStr) {
    this.dateBeginStr = dateBeginStr;
  }

  public String getDateEndStr() {
    return dateEndStr;
  }

  public void setDateEndStr(String dateEndStr) {
    this.dateEndStr = dateEndStr;
  }

  public String getTimeBeginStr() {
    return timeBeginStr;
  }

  public void setTimeBeginStr(String timeBeginStr) {
    this.timeBeginStr = timeBeginStr;
  }

  public String getTimeEndStr() {
    return timeEndStr;
  }

  public void setTimeEndStr(String timeEndStr) {
    this.timeEndStr = timeEndStr;
  }

  public String getDayStr() {
    return dayStr;
  }

  public void setDayStr(String dayStr) {
    this.dayStr = dayStr;
  }

  public String getFile() {
    return file;
  }

  public void setFile(String file) {
    this.file = file;
  }
}
