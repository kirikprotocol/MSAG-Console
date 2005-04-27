package ru.novosoft.smsc.admin.smsstat;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.smsview.SmsSet;
import ru.novosoft.smsc.admin.smsview.SmsQuery;
import ru.novosoft.smsc.admin.smsview.DateConvertor;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 22.04.2005
 * Time: 15:32:13
 * To change this template use File | Settings | File Templates.
 */
public class SmsStat
{
  Category logger = Category.getInstance(SmsStat.class);
  private String statstorePath;
  private static final String SECTION_NAME_MessageStore = "MessageStore";
  private static final String PARAM_NAME_statisticsDir = "statisticsDir";
  private String DATE_FORMAT = "yyyy-MM-dd HH:mm:ss";
  private String DATE_DIR_FILE_FORMAT = "yyyy-MM-dd";
  private String DATE_DIR_INT_FORMAT = "yyyyMM";
  private String DATE_DIR_FORMAT = "yyyy-MM";
  private static final String PERIOD_DATE_FORMAT = "yyyyMMdd";
  private TreeMap selectedFiles=new TreeMap();
  SimpleDateFormat dateFormat=new SimpleDateFormat(DATE_FORMAT);
  private String dirPrefics = "/";//Solaris
  private Statistics stat = null;
  private Smsc smsc = null;
  protected RouteSubjectManager routeSubjectManager = null;
  protected ProviderManager providerManager = null;
  protected CategoryManager categoryManager = null;

  public void init(SMSCAppContext appContext) throws AdminException
  {
    Smsc smsc = appContext.getSmsc();
    String configPath = smsc.getConfigFolder().getAbsolutePath();
    //dirPrefics = "/";
    int len = configPath.lastIndexOf(dirPrefics) + 1; //Solaris
    if (len < 1) {
      dirPrefics = "\\";//Windows
      //  len = configPath.lastIndexOf(dirPrefics) + 1;//Windows
    }
    //   String absolutePath = configPath.substring(0, len);
    Config config = smsc.getSmscConfig();

    try {
      statstorePath = config.getString(SECTION_NAME_MessageStore + '.' + PARAM_NAME_statisticsDir);
    } catch (Config.ParamNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    } catch (Config.WrongParamTypeException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
    /*  if (statstorePath.indexOf(dirPrefics) != 0)
    statstorePath = absolutePath + statstorePath;
    */
  }
  public TreeMap getStatQueryDirs(StatQuery query) throws AdminException
  {
    dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    Date fromDate =new Date();
    Date tillDate =new Date();
    TreeMap selected=new TreeMap();
    if (query.isTillDateEnabled()) {
      tillDate=DateConvertor.convertLocalToGMT(query.getTillDate());
    }
    if (query.isFromDateEnabled()) {
      fromDate =  DateConvertor.convertLocalToGMT(query.getFromDate());
    }
    File filePath=new File(statstorePath);
    String[] dirNames=filePath.list();
    if (dirNames==null || dirNames.length==0) throw new AdminException("No statistics directories");
    for (int i = 0; i < dirNames.length; i++) {
      String dirName = dirNames[i];
      File[] dirFiles=(new File(statstorePath+dirPrefics+dirName)).listFiles();
      if (dirFiles==null || dirFiles.length==0) continue;
      dateFormat.applyPattern(DATE_DIR_FORMAT);
      try {
        Date dirDate=dateFormat.parse(dirName);
        dateFormat.applyPattern(DATE_DIR_INT_FORMAT);
        String dirDate1=dateFormat.format(dirDate);
        int dirDate2=Integer.parseInt(dirDate1);
        String fromDate1=dateFormat.format(fromDate);
        int fromDate2=Integer.parseInt(fromDate1);
        String tillDate1=dateFormat.format(tillDate);
        int tillDate2=Integer.parseInt(tillDate1);
        if (query.isFromDateEnabled() && fromDate2>dirDate2+1) continue;
        if (query.isTillDateEnabled() && tillDate2<dirDate2+1) continue;
        dateFormat.applyPattern(DATE_DIR_FILE_FORMAT);
        for (int j = 0; j < dirFiles.length; j++) {
          String fileName=dirFiles[j].getName();
          Date fileDate=dateFormat.parse(dirName+"-"+fileName.substring(0,2));
          if (query.isFromDateEnabled() && fromDate.getTime()>fileDate.getTime()+Constants.Day) continue;
          if (query.isTillDateEnabled() && tillDate.getTime()<fileDate.getTime()+Constants.Day) continue;
          selected.put(fileDate,statstorePath+dirPrefics+dirName+dirPrefics+fileName);
        }
      } catch (ParseException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        throw new AdminException(e.getMessage());
      }
    }
    return selected;
  }
  public InputStream openStream(String path) throws AdminException
  {
    InputStream in = null;
    try {
      return new FileInputStream(path);
    } catch (FileNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      throw new AdminException(e.getMessage());
    }
  }
  public void closeStream(InputStream input) throws AdminException
  {
    try {
      if (input != null) input.close();
      input = null;
    } catch (Exception exc) {
    }
  }
  public Statistics getStatistics(StatQuery query) throws AdminException
  {
    selectedFiles=getStatQueryDirs(query);
    stat = new Statistics();
    DateCountersSet dateCounters = null; int nextDate;
    HashMap countersForSme = new HashMap();
    HashMap countersForRoute = new HashMap();
    HashSet warnRoutId=new HashSet();
    HashMap warnProvId=new HashMap();
    HashMap warnCatId=new HashMap();
    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
    formatter.setTimeZone(TimeZone.getDefault());
    for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();) {
      Date dateFile = (Date) iterator.next();
      String path=(String)selectedFiles.get(dateFile);
      InputStream input = null;
      try {
        input = new FileInputStream(path);
        StatFileReader record;
        String dateFileLOCAL=formatter.format(dateFile);
        int dateFileInt=Integer.parseInt(dateFileLOCAL);
        if (dateCounters != null && dateCounters.getDateFile()!=dateFileInt) {
          stat.addDateStat(dateCounters);  dateCounters = null;}
        String FileName = StatFileReader.readString(input, 9);
        if (FileName == null || !FileName.equals("SMSC.STAT"))
          throw new AdminException("unsupported header of file (support only SMSC.STAT file )");
        int version = (int) StatFileReader.readUInt16(input);
        record = new StatFileReader(routeSubjectManager,providerManager,
                categoryManager, warnRoutId,warnProvId,warnCatId,stat,
                countersForSme,countersForRoute,dateCounters);
        stat=record.receiveStat(input,dateFile);
        dateCounters=record.getDateCounters();
        countersForSme=record.getCountersForSme();
        countersForRoute=record.getCountersForRoute();
        warnRoutId=record.getWarnRoutId();
        warnProvId=record.getWarnProvId();
        warnCatId=record.getWarnCatId();
        if (!iterator.hasNext()) stat.addDateStat(dateCounters);
      } catch (IOException e) {
        throw new AdminException(e.getMessage());
      }
      finally {
        try { if (input != null) input.close(); } catch(Throwable th) { th.printStackTrace(); }
      }
    } // for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();)
    Collection countersSme = countersForSme.values();
    if (countersSme != null ) stat.addSmeIdCollection(countersSme);
    Collection countersRoute = countersForRoute.values();
    if (countersRoute != null ) stat.addRouteIdCollection(countersRoute);


    return stat;
  }

  public CategoryManager getCategoryManager()
  {
    return categoryManager;
  }

  public void setCategoryManager(CategoryManager categoryManager)
  {
    this.categoryManager = categoryManager;
  }

  public ProviderManager getProviderManager()
  {
    return providerManager;
  }

  public void setProviderManager(ProviderManager providerManager)
  {
    this.providerManager = providerManager;
  }

  public RouteSubjectManager getRouteSubjectManager()
  {
    return routeSubjectManager;
  }

  public void setRouteSubjectManager(RouteSubjectManager routeSubjectManager)
  {
    this.routeSubjectManager = routeSubjectManager;
  }

  public Smsc getSmsc()
  {
    return smsc;
  }

  public void setSmsc(Smsc smsc)
  {
    this.smsc = smsc;
  }

}
