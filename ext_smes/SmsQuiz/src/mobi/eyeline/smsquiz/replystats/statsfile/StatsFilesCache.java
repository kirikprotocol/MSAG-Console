package mobi.eyeline.smsquiz.replystats.statsfile;

import mobi.eyeline.smsquiz.replystats.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.ReplyStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.text.SimpleDateFormat;
import java.io.File;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import org.apache.log4j.Logger;

public class StatsFilesCache {

    private Map<String,StatsFile> filesMap;
    private SimpleDateFormat fileNameFormat;
    private SimpleDateFormat dateToStringFormat;
    private static String fileNamePattern;
    private static Logger logger = Logger.getLogger(StatsFilesCache.class);

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
        filesMap = new HashMap<String, StatsFile>();
        fileNameFormat = new SimpleDateFormat(fileNamePattern);
        dateToStringFormat = new SimpleDateFormat("yyyyMMdd");
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
	//todo
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
        StatsFile file = null;
        String key = buildKey(dest, date);

        final Thread shutdowner = new Thread() {
                        public boolean shutdown = false;
                        public void run() {
                            shutdown = true;
                        }
                        public boolean getShutdown() {
                            return shutdown;
                        }
                    };
        if( (file = filesMap.get(key)) == null) {
            file = new StatsFile(){
                    private StatsFileImpl statsFileImpl;
                    private String filename = fileNameFormat.format(date);
                    private String da = dest;
                    private Lock lock = new ReentrantLock();
                    public void open() throws ReplyStatsException {
                        lock.lock();
                        if(statsFileImpl==null) {
                            statsFileImpl = new StatsFileImpl(da,filename);
                        }
                        if(statsFileImpl.isClosed()) {
                            statsFileImpl.open();
                        }
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
                    public boolean exist() {
                        File replyStatsDirFile = new File(StatsFileImpl.getReplyStatsDir()+"/"+da+"/"+filename);
                        return replyStatsDirFile.exists();
                    }
            };

            if(!file.exist()) {
                return null;
            }
            filesMap.put(key,file);
        }
        return file;
    }


	 
}
 
