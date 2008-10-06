package mobi.eyeline.smsquiz.replystats.statsfile;

import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.text.SimpleDateFormat;
import java.io.File;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import org.apache.log4j.Logger;

public class StatsFilesCache {

    private Map<String,CachedStatsFile> filesMap;
    private SimpleDateFormat fileNameFormat;
    private SimpleDateFormat dateToStringFormat;
    private static String fileNamePattern;
    private static Logger logger = Logger.getLogger(StatsFilesCache.class);
    private ScheduledExecutorService s;
    private static long delayFirst;
    private static long iterationPeriod;
    private static long timeLimit;

    public static void init(final String configFile) throws FileStatsException {
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));
            PropertiesConfig config = new PropertiesConfig(c.getSection("statsFile").toProperties("."));
            fileNamePattern = config.getString("filename.pattern");
            config = new PropertiesConfig(c.getSection("fileCollector").toProperties("."));
            delayFirst = config.getLong("time.first.delay");
            iterationPeriod = config.getLong("time.period");
            timeLimit = config.getLong("time.limit");

            if(fileNamePattern==null) {
                throw new FileStatsException("filename.pattern not found in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
        } catch (ConfigException e) {
            logger.error("Unable to init StatsFilesCache",e);
            throw new FileStatsException("Unable to init StatsFilesCache",e);
        }
    }

    public StatsFilesCache() {
        filesMap = new HashMap<String, CachedStatsFile>();      // todo
        fileNameFormat = new SimpleDateFormat(fileNamePattern);
        dateToStringFormat = new SimpleDateFormat("yyyyMMdd");

        s = Executors.newSingleThreadScheduledExecutor();
        s.scheduleAtFixedRate(new FileCollector(false),delayFirst,iterationPeriod, java.util.concurrent.TimeUnit.MINUTES);

    }
	 
    public Collection<StatsFile> getFiles(String da, Date from, Date till) throws FileStatsException {
        if((da==null)||(from==null)||(till==null)) {
            throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        StatsFile statsFile = null;
        Collection<StatsFile> files = new HashSet<StatsFile>();

        Calendar calendar = Calendar.getInstance();
        calendar.setTime(from);
        calendar.set(Calendar.HOUR_OF_DAY,0);
        calendar.set(Calendar.MINUTE,0);
        calendar.set(Calendar.SECOND,0);
        calendar.set(Calendar.MILLISECOND,0);

        while(calendar.getTime().before(till)) {
            if ((statsFile = lockupFile(da, calendar.getTime()))!=null) {
                files.add(statsFile);
            }
            calendar.add(Calendar.DAY_OF_MONTH,1);
        } 
        return files;
    }
	 
	public StatsFile getFile(String da, Date date) throws FileStatsException {
        if((da==null)||(date==null)) {
            throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        return lockupFile(da,date);
	}
	 
	public void shutdown() {
        s.shutdown();
        new Thread(new FileCollector(true)).run();
    }

    private String buildKey(String da, Date date) {
        String result ="";
        result+=da+"_" + dateToString(date);
        return result;
    }
    private String dateToString(Date date) {
        return dateToStringFormat.format(date);
    }

    private StatsFile lockupFile(final String dest, final Date date){
        CachedStatsFile file = null;
        String key = buildKey(dest, date);
        
        if( (file = filesMap.get(key)) == null) {
            file = new CachedStatsFile(dest, fileNameFormat.format(date));
            filesMap.put(key, file);
        }
        return file;
    }

    private class CachedStatsFile implements StatsFile{
        private StatsFileImpl statsFileImpl;
        private String fileName;
        private String da;
        private Lock lock = new ReentrantLock();
        private boolean isClosed = true;
        private Date lastUsageDate;

        public CachedStatsFile(String da, String fileName) {
            this.da = da;
            this.fileName = fileName;
            lastUsageDate = new Date();
        }

        public void open() throws FileStatsException {
            lock.lock();               // todo
            if(statsFileImpl==null) {
                statsFileImpl = new StatsFileImpl(da, fileName);
            }
            if(isClosed) {
                statsFileImpl.open();
                isClosed = false;
            }
            lastUsageDate = new Date();
        }

        public void add(Reply reply) throws FileStatsException {
            statsFileImpl.add(reply);
        }
        public void list(Date from, Date till, Collection result) throws FileStatsException {
            statsFileImpl.list(from, till, result);
        }

        public void close() {
            lock.unlock();
        }

        private void closeExt() {
            statsFileImpl.close();
            isClosed = false;
        }
    }

    private class FileCollector implements Runnable {

        boolean closeAll = false;
        FileCollector(boolean closeAll) {
            this.closeAll = closeAll;
        }

        public void run() {
            if(logger.isInfoEnabled()) {
                logger.info("FileCollector starts...");
            }
            for(CachedStatsFile file:filesMap.values()) {
                if(!file.isClosed) {
                    boolean close;
                    if(closeAll) {
                        close = closeAll;
                    }
                    else {
                        close = (new Date().getTime() - file.lastUsageDate.getTime())>timeLimit;  // todo
                    }
                    if(close) {
                        try {
                            file.open();
                            file.closeExt();
                            file.close();
                        } catch (FileStatsException e) {
                            logger.error("Error lock the cached file", e);
                        }
                    }
                }
            }
            if(logger.isInfoEnabled()) {
                logger.info("FileCollector finishes.");
            }
        }
    }
}


 
