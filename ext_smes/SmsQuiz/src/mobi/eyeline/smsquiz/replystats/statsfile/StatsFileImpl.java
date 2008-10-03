package mobi.eyeline.smsquiz.replystats.statsfile;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.replystats.ReplyStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

public class StatsFileImpl implements StatsFile {
    private static String replyStatsDir;   //директория статистики
    private static Logger logger = Logger.getLogger(StatsFileImpl.class);
    private static String datePattern;
    private static String timePattern;

    private final String da;    // номер сервиса
    private BufferedReader reader;
    private PrintWriter writer;
    private SimpleDateFormat dateFormat;
    private SimpleDateFormat timeFormat;
    private SimpleDateFormat cvsDateFormat;
    private String filePath;

    public static void init(final String configFile) throws ReplyStatsException {
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));
            final PropertiesConfig config = new PropertiesConfig(c.getSection("statsFile").toProperties("."));
            replyStatsDir = config.getString("dir.name",null);
            if(replyStatsDir==null) {
                throw new ReplyStatsException("dir.name parameter missed in config file", ReplyStatsException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
            timePattern = config.getString("time.pattern","yyyyMMdd");
            datePattern = config.getString("date.pattern","НН:mm");

            File replyStatsDirFile = new File(replyStatsDir);
            if(!replyStatsDirFile.exists()) {
                logger.warn(replyStatsDir+" doesn't exist. Creating...");
                replyStatsDirFile.mkdirs();
            }
        } catch (ConfigException e) {
            logger.error("Unable to init StatsFile",e);
            throw new ReplyStatsException("Unable to init StatsFile",e);
        }
    }
    public StatsFileImpl(String da, String fileName) throws ReplyStatsException{
        this.da = da;
        filePath = replyStatsDir + "/" + da +"/"+fileName;

        dateFormat = new SimpleDateFormat(datePattern);
        timeFormat = new SimpleDateFormat(timePattern);
        cvsDateFormat = new SimpleDateFormat(datePattern+" "+timePattern);

        File currentFile = new File(filePath);
        if(!currentFile.exists()) {
            logger.error(filePath+ " doesn't exist!");
            throw new ReplyStatsException(filePath+ " doesn't exist!", ReplyStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
    }

    public void open() throws ReplyStatsException{
        try{
            writer = new PrintWriter(new BufferedWriter(new FileWriter(filePath,true)));
        } catch (IOException e) {
            logger.error("Can't create io stream",e);
            throw new ReplyStatsException("Can't create io stream",e);
        }
    }
	 

	public void add(Reply reply)  throws ReplyStatsException {
        if((reply.getDate()==null)||(reply.getOa()==null)||(reply.getText()==null)) {
            logger.error("Some arguments are missed");
            throw new ReplyStatsException("Some arguments are missed", ReplyStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        writer.print(dateFormat.format(reply.getDate())+",");
        writer.print(timeFormat.format(reply.getDate())+",");
        writer.print(reply.getOa()+",");
        writer.println(reply.getText());
        writer.flush();
    }
	 

	@SuppressWarnings({"unchecked"})
    public  void list(Date from, Date till, Collection result)  throws ReplyStatsException {
        if((result==null)||(from==null)||(till==null)) {
            logger.error("Some arguments are null!");
            throw new ReplyStatsException("Some arguments are null!", ReplyStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        String line=null;
        StringTokenizer tokenizer=null;
        Date date = null;
        Reply reply=  null;
        try {
            reader = new BufferedReader(new FileReader(filePath));
            while((line = reader.readLine())!=null) {
                tokenizer = new StringTokenizer(line,",");
                date = cvsDateFormat.parse(tokenizer.nextToken()+" "+tokenizer.nextToken());
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
        }
        catch (NoSuchElementException e) {
            logger.error("Unsupported file format",e);
            throw new ReplyStatsException("Unsupported file format",e);
        } catch (ParseException e) {
            logger.error("Unsupported file format",e);
            throw new ReplyStatsException("Unsupported file format",e);
        } catch (FileNotFoundException e) {
            logger.error("Unable to create file reader",e);
            throw new ReplyStatsException("Unable to create file reader",e);
        } catch (IOException e) {
            logger.error("IOException during reading file",e);
            throw new ReplyStatsException("IOException during reading file",e);
        }

    }

	public void close() {
        if(writer!=null) {
            writer.close();
        }
    }

    public static void main(String[] args) throws ReplyStatsException {
        init("conf/replystats.xml");
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
        statsFile.close();

    }
	 
}
 
