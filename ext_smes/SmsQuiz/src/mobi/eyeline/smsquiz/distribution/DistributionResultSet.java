package mobi.eyeline.smsquiz.distribution;

import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

public class DistributionResultSet implements ResultSet {

    private static Logger logger = Logger.getLogger(DistributionResultSet.class);

    private File currentFile;
    private String currentString;
    private Set<File> files;
    private BufferedReader reader;
    private final Date startDate;
    private final Date endDate;
    private String successStatus;
    private SimpleDateFormat dateFormat;

    public DistributionResultSet(final Collection<File> files, Date startDate, Date endDate, String successStatus, String dateInFilePattern) throws DistributionException {
        if((files==null)||(startDate==null)||(endDate==null)||(successStatus==null)||(dateInFilePattern==null)) {
            logger.error("Some arguments in constructor are null");
            throw new DistributionException("Some arguments in constructor are null", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        this.successStatus = successStatus;
        this.startDate = startDate;
        this.endDate = endDate;
        this.files = new HashSet<File>();
        for(File f:files) {
            this.files.add(f);
        }

        dateFormat = new SimpleDateFormat(dateInFilePattern);
    }

    public boolean next()  {
        try{
            String line = null;
            if(files.size()==0) {
                return false;
            }
            if(currentFile==null) {
                currentFile = files.iterator().next();
                reader = new BufferedReader(new FileReader(currentFile));
            }
            if((line=reader.readLine())!=null) {
                return parseLine(line, successStatus, startDate, endDate) || next();
            } else {
                reader.close();
                files.remove(currentFile);
                currentFile=null;
                return next();
            }
        } catch (Exception e) {
            logger.error("Error during getting line from file", e);
            if(currentFile!=null) {
                files.remove(currentFile);
                currentFile=null;
            }
            if(reader!=null) {
                try {
                    reader.close();
                } catch (IOException e1) {
                    logger.error("Can't close reader",e1);
                }
            }
            return next();
        }
    }

	public Object get() {
        if(currentString==null) {
            return null;
        }
        return currentString;
	}

	public void close() {
	    if(reader!=null) {
            try {
                reader.close();
            } catch (IOException e) {
                logger.error("Can't close reader",e);
            }
        }
        files.clear();
    }

    private boolean parseLine(String line, String successStatus, Date from, Date until){
        StringTokenizer tokenizer= new StringTokenizer(line,",");
        String status = tokenizer.nextToken();
        if(status.equals(successStatus)) {
            try {
                Date date = dateFormat.parse(tokenizer.nextToken());
                if((date.compareTo(until)<=0)&&(date.compareTo(from)>=0)) {
                    currentString = tokenizer.nextToken();
                    return true;    
                }
            } catch (ParseException e) {
                logger.error("Unable to parse date", e);
            }
         }
        return false;
    }
	 
}
 
