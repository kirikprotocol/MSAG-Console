package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.FileUtils;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.Reply;

class StatsFileImpl implements StatsFile {
    private static Logger logger = Logger.getLogger(StatsFileImpl.class);

    private final String da;
    private PrintWriter writer;
    private SimpleDateFormat dateFormat;
    private SimpleDateFormat timeFormat;
    private SimpleDateFormat csvDateFormat;
    private String filePath;


    public StatsFileImpl(final String da, final String filePath) throws FileStatsException{
        this(da,filePath, "HH:mm", "yyyyMMdd");
    }
    public StatsFileImpl(final String da, final String filePath, String timePattern, String datePattern) throws FileStatsException {
        this.da = da;
        this.filePath = filePath;

        dateFormat = new SimpleDateFormat(datePattern);
        timeFormat = new SimpleDateFormat(timePattern);
        csvDateFormat = new SimpleDateFormat(datePattern+ ' ' +timePattern);
        File currentFile = new File(filePath);
        File parent = currentFile.getParentFile();
        if (!parent.exists())
            parent.mkdirs();
        else {
            try {
                FileUtils.truncateFile(currentFile,"\n".getBytes()[0],10);
            } catch (IOException e) {
                logger.error("Unable to truncate file",e);
                throw new FileStatsException("Unable to truncate file",e);
            }
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
        writer.print(dateFormat.format(reply.getDate()));
        writer.print(",");
        writer.print(timeFormat.format(reply.getDate()));
        writer.print(",");
        writer.print(reply.getOa());
        writer.print(",");
        writer.println(reply.getText());
        writer.flush();
    }
	 

	@SuppressWarnings({"unchecked"})
    public  void list(Date from, Date till, Collection<Reply> result)  throws FileStatsException {
        if((result==null)||(from==null)||(till==null)) {
            logger.error("Some arguments are null!");
            throw new FileStatsException("Some arguments are null!", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        String line;
        StringTokenizer tokenizer;
        Date date;
        Reply reply;
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(filePath));
            while((line = reader.readLine())!=null) {
                tokenizer = new StringTokenizer(line,",");
                date = csvDateFormat.parse(tokenizer.nextToken()+" "+tokenizer.nextToken());
                if((date.compareTo(till)<=0)&&(date.compareTo(from)>=0)) {
                    reply = new Reply();
                    reply.setDa(da);
                    reply.setDate(date);
                    reply.setOa(tokenizer.nextToken());
                    reply.setText(tokenizer.nextToken());

                    result.add(reply);
                }
            }

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
        } finally {
            if(reader!=null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    logger.error("Can't close reader",e);
                    throw new FileStatsException("Can't close reader", e);
                }
            }
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

	 
}
 
