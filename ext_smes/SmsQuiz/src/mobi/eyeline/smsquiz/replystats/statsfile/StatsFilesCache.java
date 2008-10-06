package mobi.eyeline.smsquiz.replystats.statsfile;

import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ConcurrentHashMap;
import java.text.SimpleDateFormat;
import java.io.File;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import org.apache.log4j.Logger;

public class StatsFilesCache {

    private static long delayFirst;
    private static long iterationPeriod;
    private static long timeLimit;
    private static String datePattern;
    private static String timePattern;
    private static String replyStatsDir;
    private static String fileNamePattern;
    private static Logger logger = Logger.getLogger(StatsFilesCache.class);

    private ConcurrentHashMap<String,CachedStatsFile> filesMap;
    private SimpleDateFormat fileNameFormat;
    private ScheduledExecutorService s;

    public static void init(final String configFile) throws FileStatsException {
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));

            PropertiesConfig config = new PropertiesConfig(c.getSection("statsFile").toProperties("."));
            fileNamePattern = config.getString("filename.pattern");
            timePattern = config.getString("time.pattern.in.file","yyyyMMdd");
            datePattern = config.getString("date.pattern.in.file","НН:mm");
            replyStatsDir = config.getString("dir.name",null);
            if(replyStatsDir==null) {
                throw new FileStatsException("dir.name parameter missed in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
            File file = new File(replyStatsDir);
            if(!file.exists()){
                file.mkdirs();
            }

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
        filesMap =  new ConcurrentHashMap<String, CachedStatsFile>();
        fileNameFormat = new SimpleDateFormat(fileNamePattern);

        s = Executors.newSingleThreadScheduledExecutor();
        s.scheduleAtFixedRate(new FileCollector(false),delayFirst,iterationPeriod, java.util.concurrent.TimeUnit.SECONDS);

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
        new FileCollector(true).run();
    }

    private String buildKey(String da, Date date) {
        String result ="";
        result+=da+"_" + date.getTime();
        return result;
    }

    private StatsFile lockupFile(final String dest, final Date date){
        CachedStatsFile file = null;
        String key = buildKey(dest, date);
        
        if( (file = filesMap.get(key)) == null) {
            file = new CachedStatsFile(dest, replyStatsDir +"/"+dest+"/"+fileNameFormat.format(date)+".csv");
            file.setMapKey(key);
            CachedStatsFile f1 = filesMap.putIfAbsent(key, file);
            if (f1 != null)
                file = f1;
        }
        return file;
    }

    private class CachedStatsFile implements StatsFile{
        private StatsFileImpl statsFileImpl;
        private String filePath;
        private String da;
        private Lock lock = new ReentrantLock();
        private boolean isClosed = true;
        private Date lastUsageDate;
        private String mapKey;

        public CachedStatsFile(String da, String filePath) {
            this.da = da;
            this.filePath = filePath;
            lastUsageDate = new Date();
        }

        public void open() throws FileStatsException{
            try{
                lock.lock();
                if(statsFileImpl==null) {
                    statsFileImpl = new StatsFileImpl(da, filePath, timePattern,datePattern);
                }
                if(isClosed) {
                    statsFileImpl.open();
                    isClosed = false;
                }
            } catch (FileStatsException e) {
                lock.unlock();
                throw new FileStatsException(e);
            }
            lastUsageDate = new Date();

        }

        public void add(Reply reply) throws FileStatsException {
            statsFileImpl.add(reply);
        }
        public void list(Date from, Date till, Collection<Reply> result) throws FileStatsException {
            statsFileImpl.list(from, till, result);
        }

        public void close() {
            lock.unlock();
        }

        private void closeExt() {
            statsFileImpl.close();
            isClosed = false;
        }

        public String getMapKey() {
            return mapKey;
        }

        public void setMapKey(String mapKey) {
            this.mapKey = mapKey;
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
                        close = (System.currentTimeMillis() - file.lastUsageDate.getTime())>timeLimit;
                    }
                    if(close) {
                        try {
                            file.open();
                            file.closeExt();
                            file.close();
                            filesMap.remove(file.getMapKey());
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


 
