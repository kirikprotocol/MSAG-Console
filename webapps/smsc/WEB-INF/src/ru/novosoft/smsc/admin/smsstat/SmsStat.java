package ru.novosoft.smsc.admin.smsstat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.smsview.DateConvertor;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.*;
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
    org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmsStat.class);

    private final static String PARAM_NAME_STAT_DIR = "MessageStore.statisticsDir";

    private final static String DIR_SEPARATOR = "/";
    private final static String DATE_DIR_FORMAT = "yyyy-MM";
    private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + DIR_SEPARATOR + "dd";
    private final static String DATE_FILE_EXTENSION = ".rts";

    private String statstorePath;

    Calendar calendar = new GregorianCalendar();
    //SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT);
    SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
    SimpleDateFormat dateDirFileFormat = new SimpleDateFormat(DATE_DIR_FILE_FORMAT);
    Date fromQueryDate = null; // TODO: move to getStatistics ???
    Date tillQueryDate = null;

    private Statistics stat = null;
    private Smsc smsc = null;

    protected RouteSubjectManager routeSubjectManager = null;
    protected ProviderManager providerManager = null;
    protected CategoryManager categoryManager = null;

    public void init(SMSCAppContext appContext) throws AdminException
    {
        Smsc smsc = appContext.getSmsc();
        Config config = smsc.getSmscConfig();

        try { statstorePath = config.getString(PARAM_NAME_STAT_DIR); }
        catch (Exception e) {
            throw new AdminException("Failed to obtain statistics dir. Details: "+e.getMessage());
        }

        dateDirFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        dateDirFileFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    }

    private TreeMap getStatQueryDirs() throws AdminException
    {
        File filePath=new File(statstorePath);
        String[] dirNames=filePath.list();
        if (dirNames==null || dirNames.length==0)
            throw new AdminException("No statistics directories");

        TreeMap selected = new TreeMap();
        for (int i = 0; i<dirNames.length; i++)
        {
            String dirName = dirNames[i];
            if (dirName == null || dirName.length() <= 0) continue;
            String currentDir = statstorePath + DIR_SEPARATOR + dirName;

            File[] dirFiles = (new File(currentDir)).listFiles();
            if (dirFiles==null || dirFiles.length==0) continue;

            Date dirDate = null;
            try { dirDate = dateDirFormat.parse(dirName); }
            catch(ParseException exc) { continue; }
            calendar.setTime(dirDate); calendar.add(Calendar.MONTH, 1);
            Date fDate = calendar.getTime();
            if (fromQueryDate != null && fDate.getTime() < fromQueryDate.getTime()) continue;
            if (tillQueryDate != null && fDate.getTime() > tillQueryDate.getTime()) continue;

            for (int j = 0; j < dirFiles.length; j++)
            {
                String fileName = dirFiles[j].getName();
                if (fileName == null || fileName.length() <= 0 ||
                        !fileName.toLowerCase().endsWith(DATE_FILE_EXTENSION)) continue;

                String dirFileName = dirName+DIR_SEPARATOR+fileName;
                Date fileDate = null;
                try { fileDate = dateDirFileFormat.parse(dirFileName); }
                catch (ParseException exc) { continue; }
                if (fromQueryDate != null && fileDate.getTime()+Constants.Day < fromQueryDate.getTime()) continue;
                if (tillQueryDate != null && fileDate.getTime()+Constants.Day > tillQueryDate.getTime()) continue;

                selected.put(fileDate, statstorePath + DIR_SEPARATOR + dirFileName);
            }
        }
        return selected;
    }

    private void initQueryPeriod(StatQuery query)
    {
        if (query.isFromDateEnabled()) fromQueryDate = query.getFromDate();
        if (query.isTillDateEnabled()) tillQueryDate = query.getTillDate();
        if (fromQueryDate != null) tillQueryDate = DateConvertor.convertLocalToGMT(query.getTillDate());
        if (tillQueryDate != null) fromQueryDate = DateConvertor.convertLocalToGMT(query.getFromDate());
    }

    private void scanCounters(CountersSet set, InputStream is) throws IOException
    {
        int accepted = (int)readUInt32(is);
        int rejected = (int)readUInt32(is);
        int delivered = (int)readUInt32(is);
        int failed  = (int)readUInt32(is);
        int rescheduled = (int)readUInt32(is);
        int temporal = (int)readUInt32(is);
        int peak_i = (int)readUInt32(is);
        int peak_o = (int)readUInt32(is);
        set.increment(accepted, rejected, delivered, failed, rescheduled,
                      temporal, peak_i, peak_o);
    }
    private void scanErrors(ExtendedCountersSet set, InputStream is) throws IOException
    {
        int counter = (int)readUInt32(is);
        while ( counter-- > 0)
        {
          int errcode = (int)readUInt32(is);
          int count = (int)readUInt32(is);
          set.incError(errcode, count);
        }
    }
    private void scanSmes(HashMap map, InputStream is) throws IOException
    {
        int counter = (int)readUInt32(is);
        while ( counter-- > 0)
        {
            int sme_id_len = (int)readUInt8(is);
            String smeId = readString(is, sme_id_len);
            SmeIdCountersSet set = (SmeIdCountersSet)map.get(smeId);
            if (set == null) {
                set = new SmeIdCountersSet(smeId);
                map.put(smeId, set);
            }
            scanCounters(set, is);
            scanErrors(set, is);
        }
    }

    private HashSet warnRoutId = null;
    private HashMap warnProvId = null;
    private HashMap warnCatId  = null;

    private void scanRoutes(HashMap map, InputStream is) throws IOException
    {
        int counter = (int)readUInt32(is);
        while ( counter-- > 0)
        {
            int route_id_len = (int)readUInt8(is);
            String routeId = readString(is, route_id_len);

            long providerId = readInt64(is);
            long categoryId = readInt64(is);
            /* TODO: !!!
            Provider provider = null; Category category = null;
            final Route r =  routeSubjectManager.getRoutes().get(routeId);
            if (r == null) {
              warnRoutId.add(routeId);// logger.warn("Route '"+routeId+"' is undefined");
              if (categoryId != -1) category = categoryManager.getCategory(new Long(categoryId));
              if (providerId != -1) provider = providerManager.getProvider(new Long(providerId));
            }
            else {
              if (providerId == -1) provider = null;
              else if (r.getProviderId() != providerId) {
                warnProvId.put(routeId,new Long(providerId));//logger.warn("ProviderId='"+providerId+"' doesn't match for route '"+routeId+"'  ");
                provider = new Provider(providerId, "undefined ("+providerId+")");
              }
              else provider = providerManager.getProvider(new Long(providerId));
              if (categoryId == -1) category = null;
              else if (r.getCategoryId() != categoryId) {
                warnCatId.put(routeId,new Long(categoryId));//logger.warn("CategoryId='"+providerId+"' doesn't match for route '"+routeId+"'  ");
                category = new Category(categoryId, "undefined ("+categoryId+")");
              }
              else category = categoryManager.getCategory(new Long(categoryId));
            }
            if (provider == null) provider = new Provider(-1, "");
            if (category == null) category = new Category(-1, "");*/

            RouteIdCountersSet set = (RouteIdCountersSet)map.get(routeId);
            if (set == null) {
                set = new RouteIdCountersSet(routeId);
                map.put(routeId, set);
            }
            // TODO: add provider & category logic
            //set.setProvider(provider);
            //set.setCategory(category);

            scanCounters(set, is);
            scanErrors(set, is);
        }
    }

    public Statistics getStatistics(StatQuery query) throws AdminException
    {
        stat = new Statistics();

        warnRoutId = new HashSet();
        warnProvId = new HashMap();
        warnCatId  = new HashMap();

        initQueryPeriod(query);
        TreeMap selectedFiles = getStatQueryDirs();
        if (selectedFiles == null || selectedFiles.size() <= 0) return stat;

        TreeMap statByHours = new TreeMap(); // to add lastHourCounter to it
        CountersSet lastHourCounter = new CountersSet();
        ExtendedCountersSet totalCounter = new ExtendedCountersSet();
        HashMap countersForSme = new HashMap(); // contains SmeIdCountersSet
        HashMap countersForRoute = new HashMap(); // contains RouteIdCountersSet

        Date lastDate = null;
        for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();)
        {
            Date fileDate = (Date)iterator.next(); // GMT
            String path = (String)selectedFiles.get(fileDate);
            InputStream input = null;
            try
            {
                input = new FileInputStream(path);
                String fileStamp = readString(input, 9);
                if (fileStamp == null || !fileStamp.equals("SMSC.STAT"))
                    throw new AdminException("unsupported header of file (support only SMSC.STAT file )");
                readUInt16(input); // read version for support reasons

                while (true) // iterate file records (by minutes)
                {
                    try
                    {
                        int rs1 = (int)readUInt32(input);
                        int hour = (int)readUInt8(input); int min = (int)readUInt8(input);
                        calendar.setTime(fileDate);
                        calendar.add(Calendar.HOUR, hour); calendar.add(Calendar.MINUTE, min);
                        Date curDate = calendar.getTime();

                        boolean skipRecord =
                         ((fromQueryDate != null && curDate.getTime() < fromQueryDate.getTime()) ||
                          (tillQueryDate != null && curDate.getTime() > tillQueryDate.getTime()));
                        if (skipRecord || lastDate == null ||
                           (curDate.getTime()-lastDate.getTime() >= Constants.Hour) )
                        {
                            lastDate = curDate;
                            statByHours.put(lastDate, lastHourCounter);
                            lastHourCounter.reset();
                        }

                        // TODO: records skipping !!!
                        if (skipRecord) readString(input, rs1 - 2);
                        else {
                            scanCounters(lastHourCounter, input);
                            scanErrors(stat, input);

                            scanSmes(countersForSme, input);
                            scanRoutes(countersForRoute, input);
                        }

                        int rs2 = (int)readUInt32(input);
                        if (rs1 != rs2)
                            throw new IOException("Invalid file format rs1="+rs1+", rs2="+rs2);
                    }
                    catch (EOFException exc) { break; }
                }

            } catch (IOException e) {
                throw new AdminException(e.getMessage());
            }
            finally {
                try { if (input != null) input.close(); } catch(Throwable th) { th.printStackTrace(); }
            }
        } // for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();)


        DateCountersSet dateCounters = null;
        Calendar localCaledar = new GregorianCalendar(TimeZone.getDefault());
        for (Iterator it = statByHours.keySet().iterator(); it.hasNext();)
        {
            Date fileDate = (Date)it.next();
            CountersSet hour = (CountersSet)statByHours.get(fileDate);
            localCaledar.setTime(fileDate);
            /*localCaledar.

            if (dateCounters == null || ) {
                dateCounters = new DateCountersSet(fileDate);
            }*/
        }

        Collection countersSme = countersForSme.values();
        if (countersSme != null ) stat.addSmeIdCollection(countersSme);
        Collection countersRoute = countersForRoute.values();
        if (countersRoute != null ) stat.addRouteIdCollection(countersRoute);

        // TODO: warnings
        return stat;
    }

    public static int readUInt8(InputStream is) throws IOException
    {
        int b = is.read();
        if (b == -1) throw new EOFException();
        return b;
    }
    public static int readUInt16(InputStream is) throws IOException
    {
        return (readUInt8(is) << 8 | readUInt8(is));
    }
    public static long readUInt32(InputStream is) throws IOException
    {
        return ((long) readUInt8(is) << 24) | ((long) readUInt8(is) << 16) |
                ((long) readUInt8(is) << 8) | ((long) readUInt8(is));
    }
    public static long readInt64(InputStream is) throws IOException
    {
        return (readUInt32(is) << 32) | readUInt32(is);
    }
    public static String readString(InputStream is, int size) throws IOException
    {
        if (size <= 0) return "";
        byte buff[] = new byte[size];
        int pos = 0;
        int cnt = 0;
        while (pos < size) {
            cnt = is.read(buff, pos, size - pos);
            if (cnt == -1) throw new EOFException();
            pos += cnt;
        }
        return new String(buff);
    }

    public void setCategoryManager(CategoryManager categoryManager) {
        this.categoryManager = categoryManager;
    }
    public void setProviderManager(ProviderManager providerManager) {
        this.providerManager = providerManager;
    }
    public void setRouteSubjectManager(RouteSubjectManager routeSubjectManager) {
        this.routeSubjectManager = routeSubjectManager;
    }
    public void setSmsc(Smsc smsc)  {
        this.smsc = smsc;
    }
    public Smsc getSmsc() {
        return smsc;
    }

}
