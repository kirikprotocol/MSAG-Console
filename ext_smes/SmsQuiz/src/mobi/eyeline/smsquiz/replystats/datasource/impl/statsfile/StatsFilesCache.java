package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ConcurrentHashMap;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.File;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import org.apache.log4j.Logger;

public class StatsFilesCache {

    private long iterationPeriod;
    private long timeLimit;
    private String datePattern;
    private String timePattern;
    private String replyStatsDir;
    private static Logger logger = Logger.getLogger(StatsFilesCache.class);

    private ConcurrentHashMap<String,CachedStatsFile> filesMap;
    private SimpleDateFormat fileNameFormat;
    private ScheduledExecutorService s;



    public StatsFilesCache(final String configFile) throws FileStatsException{
        long delayFirst;
        String fileNamePattern = null;
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));

            PropertiesConfig config = new PropertiesConfig(c.getSection("replystats").toProperties("."));
            fileNamePattern = config.getString("statsFile.filename.pattern");
            timePattern = config.getString("statsFile.time.pattern.in.file","yyyyMMdd");
            datePattern = config.getString("statsFile.date.pattern.in.file","НН:mm");
            replyStatsDir = config.getString("statsFile.dir.name",null);
            if(replyStatsDir==null) {
                throw new FileStatsException("dir.name parameter missed in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
            File file = new File(replyStatsDir);
            if(!file.exists()){
                file.mkdirs();
            }

            delayFirst = config.getLong("fileCollector.time.first.delay");
            iterationPeriod = config.getLong("fileCollector.time.period");
            timeLimit = config.getLong("fileCollector.time.limit");
            if(fileNamePattern ==null) {
                throw new FileStatsException("statsFile.filename.pattern not found in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
        } catch (ConfigException e) {
            logger.error("Unable to init StatsFilesCache",e);
            throw new FileStatsException("Unable to init StatsFilesCache",e);
        }
        filesMap =  new ConcurrentHashMap<String, CachedStatsFile>();
        fileNameFormat = new SimpleDateFormat(fileNamePattern);

        s = Executors.newSingleThreadScheduledExecutor();
        s.scheduleAtFixedRate(new FileCollector(false), delayFirst,iterationPeriod, java.util.concurrent.TimeUnit.SECONDS);

    }
	 
    public Collection<StatsFile> getFiles(String da, Date from, Date till) throws FileStatsException {
        if((da==null)||(from==null)||(till==null)) {
            throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        StatsFile statsFile;
        Collection<StatsFile> files = new LinkedList<StatsFile>();

        File dir = new File(replyStatsDir+"/"+da);
        if(!dir.exists()) {
            return files;
        }

        Calendar calendar = Calendar.getInstance();
        calendar.setTime(from);
        resetTillDay(calendar);
        Date modifedFrom = calendar.getTime();
        calendar.setTime(till);
        resetTillDay(calendar);
        Date modifedTill = calendar.getTime();

        for(File f: dir.listFiles()) {
            if(f.isFile()) {
                String name = f.getName();
                try {
                    Date date = fileNameFormat.parse(name.substring(0,name.lastIndexOf(".")));
                    if((date.compareTo(modifedTill)<=0)&&(date.compareTo(modifedFrom)>=0)) {
                        if ((statsFile = lockupFile(da, calendar.getTime(),true))!=null) {
                            files.add(statsFile);
                        }
                    }
                } catch (ParseException e) {
                    logger.error("Can't parse filename",e);
                    throw new FileStatsException("Can't parse filename",e);
                }
            }
        }
        return files;
    }
    private void resetTillDay(Calendar calendar) {
        if(calendar!=null) {
            calendar.set(Calendar.HOUR_OF_DAY,0);
            calendar.set(Calendar.MINUTE,0);
            calendar.set(Calendar.SECOND,0);
            calendar.set(Calendar.MILLISECOND,0);
        }
    }
	 
    public StatsFile getFile(String da, Date date) throws FileStatsException {
        if((da==null)||(date==null)) {
            throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        return lockupFile(da,date,false);
	}
	 
	public void shutdown() {
        s.shutdown();
        new FileCollector(true).run();
    }

    private String buildKey(String da, Date date) {
        String result ="";
        result+=da+ '_' + fileNameFormat.format(date);
        return result;
    }

    private StatsFile lockupFile(final String dest, final Date date, boolean checkExist){
        CachedStatsFile file;
        String key = buildKey(dest, date);
        
        if( (file = filesMap.get(key)) == null) {
            String filePath = replyStatsDir + '/' +dest+ '/' +fileNameFormat.format(date)+".csv";
            if(checkExist) {
                File f = new File(filePath);
                if(!f.exists())
                    return null;
            }
            file = new CachedStatsFile(dest, filePath);
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

        public List<Reply> getReplies(String oa, Date from, Date till) throws FileStatsException {
            return statsFileImpl.getReplies(oa, from, till);
        }

        public void close() {
            lock.unlock();
        }

        private void closeExt() {
            statsFileImpl.close();
            isClosed = true;
        }


        public boolean exist() {
            File file = new File(filePath);
            return file.exists();
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

            for (Map.Entry<String, CachedStatsFile> e : filesMap.entrySet()) {

            }

            for(Map.Entry<String, CachedStatsFile> entry : filesMap.entrySet()) {
                CachedStatsFile file = entry.getValue();
                String key = entry.getKey();
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
                            filesMap.remove(key);
                        } catch (FileStatsException exc) {
                            logger.error("Error lock the cached file", exc);
                        }
                    }
                }
            }
            if(logger.isInfoEnabled()) {
                logger.info("FileCollector finishes.");
            }
        }
    }
    public int countOpenedFiles() {
        if(filesMap!=null) {
            return filesMap.values().size();
        }
        else {
            return 0;
        }
    }

    public long getIterationPeriod() {
        return iterationPeriod;
    }
    public String getReplyStatsDir() {
        return replyStatsDir.substring(0);
    }
}


 
