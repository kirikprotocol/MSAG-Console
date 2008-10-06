package mobi.eyeline.smsquiz.replystats.statsfile;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.FileUtils;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

public class StatsFileImpl implements StatsFile {
    private static Logger logger = Logger.getLogger(StatsFileImpl.class);

    private static String replyStatsDir;   //директория статистики
    private static String datePattern;
    private static String timePattern;

    private final String da;    // номер сервиса
    private PrintWriter writer;
    private SimpleDateFormat dateFormat;
    private SimpleDateFormat timeFormat;
    private SimpleDateFormat csvDateFormat;
    private String filePath;

    public static void init(final String configFile) throws FileStatsException {
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));
            final PropertiesConfig config = new PropertiesConfig(c.getSection("statsFile").toProperties("."));
            replyStatsDir = config.getString("dir.name",null);
            if(replyStatsDir==null) {
                throw new FileStatsException("dir.name parameter missed in config file", FileStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
            timePattern = config.getString("time.pattern.in.file","yyyyMMdd");
            datePattern = config.getString("date.pattern.in.file","НН:mm");
        } catch (ConfigException e) {
            logger.error("Unable to init StatsFile",e);
            throw new FileStatsException("Unable to init StatsFile",e);
        }
    }
    public StatsFileImpl(final String da, final String fileName) throws FileStatsException {
        this.da = da;
        filePath = replyStatsDir + "/" + da +"/"+fileName;

        dateFormat = new SimpleDateFormat(datePattern);
        timeFormat = new SimpleDateFormat(timePattern);
        csvDateFormat = new SimpleDateFormat(datePattern+" "+timePattern);
        File currentFile = new File(filePath);
        try {
            FileUtils.truncateFile(currentFile,"\n".getBytes()[0],10);
        } catch (IOException e) {
            logger.error("Unable to truncate file",e);
            throw new FileStatsException("Unable to truncate file",e);
        }
    }

    public void open() throws FileStatsException {
        if(logger.isInfoEnabled()) {
            logger.info("File: "+filePath+" opened");
        }
        try{
            writer = new PrintWriter(new BufferedWriter(new FileWriter(filePath,true)));
        } catch (IOException e) {
            logger.error("Can't create io stream",e);
            throw new FileStatsException("Can't create io stream",e);
        }
    }
	 

	public void add(Reply reply)  throws FileStatsException {
        if((reply.getDate()==null)||(reply.getOa()==null)||(reply.getText()==null)) {
            logger.error("Some arguments are missed");
            throw new FileStatsException("Some arguments are missed", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        writer.print(dateFormat.format(reply.getDate())+",");
        writer.print(timeFormat.format(reply.getDate())+",");
        writer.print(reply.getOa()+",");
        writer.println(reply.getText());
        writer.flush();
    }
	 

	@SuppressWarnings({"unchecked"})
    public  void list(Date from, Date till, Collection result)  throws FileStatsException {
        if((result==null)||(from==null)||(till==null)) {
            logger.error("Some arguments are null!");
            throw new FileStatsException("Some arguments are null!", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        String line=null;
        StringTokenizer tokenizer=null;
        Date date = null;
        Reply reply=  null;
        try {
            BufferedReader reader = new BufferedReader(new FileReader(filePath));
            while((line = reader.readLine())!=null) {
                tokenizer = new StringTokenizer(line,",");
                date = csvDateFormat.parse(tokenizer.nextToken()+" "+tokenizer.nextToken());
                if((date.before(till))&&(date.after(from))) {
                    reply = new Reply();
                    reply.setDa(da);
                    reply.setDate(date);
                    reply.setOa(tokenizer.nextToken());
                    reply.setText(tokenizer.nextToken());
                    
                    result.add(reply);
                }
            }
            reader.close();

        } catch (FileNotFoundException e) {
            logger.info("Unable to create file reader, maybe file doesn't exist",e);
        }
        catch (NoSuchElementException e) {
            logger.error("Unsupported file format",e);
            throw new FileStatsException("Unsupported file format",e);
        } catch (ParseException e) {
            logger.error("Unsupported file format",e);
            throw new FileStatsException("Unsupported file format",e);
        } catch (IOException e) {
            logger.error("IOException during reading file",e);
            throw new FileStatsException("IOException during reading file",e);
        }

    }

	public void close() {
        if(logger.isInfoEnabled()) {
            logger.info("File: "+filePath+" closed");
        }
        if(writer!=null) {
            writer.close();
        }
    }

    public boolean exist() {
        File file = new File(filePath);
        return file.exists();
    }

    public static String getReplyStatsDir() {
        return replyStatsDir;   
    }

    public static void main(String[] args) throws FileStatsException {
        /*init("conf/replystats.xml");
        StatsFileImpl statsFile = new StatsFileImpl("148","test.txt");
        statsFile.open();
        Reply reply = new Reply();
        reply.setDa("148");
        reply.setOa("+7913924924249");
        reply.setText("Test");
        reply.setDate(new Date());
        statsFile.add(reply);
        ArrayList list = new ArrayList();
        statsFile.list(new Date(0),new Date(),list );
        for(Object obj:list) {
            System.out.println(obj);
        }
        statsFile.close(); */
        System.out.println();
    }

	 
}
 
