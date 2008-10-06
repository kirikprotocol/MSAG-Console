package mobi.eyeline.smsquiz.replystats.statsfile;

import mobi.eyeline.smsquiz.replystats.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.ReplyStatsException;
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

    public static void init(final String configFile) throws ReplyStatsException {
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));
            final PropertiesConfig config = new PropertiesConfig(c.getSection("statsFile").toProperties("."));
            fileNamePattern = config.getString("filename.pattern");
            if(fileNamePattern==null) {
                throw new ReplyStatsException("filename.pattern not found in config file", ReplyStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
        } catch (ConfigException e) {
            logger.error("Unable to init StatsFilesCache",e);
            throw new ReplyStatsException("Unable to init StatsFilesCache",e);
        }
    }

    public StatsFilesCache() {
        filesMap = new HashMap<String, CachedStatsFile>();
        fileNameFormat = new SimpleDateFormat(fileNamePattern);
        dateToStringFormat = new SimpleDateFormat("yyyyMMdd");

        s = Executors.newSingleThreadScheduledExecutor();
        s.scheduleAtFixedRate(new FileCollector(false),500000,50000, java.util.concurrent.TimeUnit.SECONDS); //todo

    }
	 
    public Collection<StatsFile> getFiles(String da, Date from, Date till) throws ReplyStatsException{
        if((da==null)||(from==null)||(till==null)) {
            throw new ReplyStatsException("Some arguments are null", ReplyStatsException.ErrorCode.ERROR_WRONG_REQUEST);
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
	 
	public StatsFile getFile(String da, Date date) throws ReplyStatsException{
        if((da==null)||(date==null)) {
            throw new ReplyStatsException("Some arguments are null", ReplyStatsException.ErrorCode.ERROR_WRONG_REQUEST);
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

        public void open() throws ReplyStatsException {
            lock.lock();
            if(statsFileImpl==null) {
                statsFileImpl = new StatsFileImpl(da, fileName);
            }
            if(isClosed) {
                statsFileImpl.open();
                isClosed = false;
            }
            lastUsageDate = new Date();
        }

        public void add(Reply reply) throws ReplyStatsException {
            statsFileImpl.add(reply);
        }
        public void list(Date from, Date till, Collection result) throws ReplyStatsException {
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
        final long timeLimit = 50000000; //todo
        boolean closeAll = false;

        FileCollector(boolean closeAll) {
            this.closeAll = closeAll;
        }

        public void run() {
            for(CachedStatsFile file:filesMap.values()) {
                boolean close;
                if(closeAll) {
                    close = closeAll;
                }
                else {
                    close = (new Date().getTime() - file.lastUsageDate.getTime())>timeLimit;
                }
                if((!file.isClosed)&&(close)) {
                    try {
                        file.open();
                        file.closeExt();
                        file.close();
                    } catch (ReplyStatsException e) {
                        logger.error("Error lock the cached file", e);
                    }
                }
            }
        }
    }
}


 
