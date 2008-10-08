package mobi.eyeline.smsquiz.distribution;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.FileUtils;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;

public class DistributionInfoSmeManager implements DistributionManager {

    private static String statsDir;
    private static String dirPattern;
    private static String filePattern;
    private static String dateInFilePattern;
    private static String succDeliveryStatus;
    private static Logger logger = Logger.getLogger(DistributionInfoSmeManager.class);

    private SimpleDateFormat dirFormat;
    private SimpleDateFormat fileFormat;
    private SimpleDateFormat dateInFileFormat;

    public static void init(final String configFile) throws DistributionException{
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));

            PropertiesConfig config = new PropertiesConfig(c.getSection("distribution").toProperties("."));

            statsDir = config.getString("info.sme.stats.dir",null);
            dirPattern = config.getString("info.sme.stats.date.dir.pattern",null);
            filePattern = config.getString("info.sme.stats.time.file.pattern",null);
            succDeliveryStatus = config.getString("info.sme.succ.delivery.status");
            dateInFilePattern = config.getString("info.sme.date.format.in.file");
            if(statsDir==null) {
               throw new DistributionException("dir.name parameter missed in config file", DistributionException.ErrorCode.ERROR_NOT_INITIALIZED);
            }
            File file = new File(statsDir);
            if(!file.exists()){
                file.mkdirs();
            }
        } catch (ConfigException e) {
            logger.error("Unable to init StatsFilesCache",e);
            throw new DistributionException("Unable to init StatsFilesCache",e);
        }
    }

    public DistributionInfoSmeManager() {
        dirFormat = new SimpleDateFormat(dirPattern);
        fileFormat = new SimpleDateFormat(filePattern);
        dateInFileFormat = new SimpleDateFormat(dateInFilePattern);
    }

    @SuppressWarnings({"ThrowFromFinallyBlock"})
    public String createDistribution(Distribution distr) throws DistributionException {
        if((distr==null)||(distr.getFilePath()==null)||(distr.getDateBegin()==null)||(distr.getDateEnd()==null)
                ||(distr.getDays()==null)||(distr.getTimeBegin()==null)|| (distr.getTimeEnd()==null)) {
            logger.error("Some fields of argument are empty");
            throw new DistributionException("Some fields of argument are empty", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        File file = new File(distr.getFilePath());
        if(!file.exists()) {
            throw new DistributionException("Distributions abonents list doesn't exist: "+distr.getFilePath(), DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
        }

        //todo Implementation
            BufferedReader reader = null;
            String str;
            String msisdn;
            List<String> msisdns = new LinkedList<String>();

            try {
                reader = new BufferedReader(new FileReader(file));
                while ((str=reader.readLine())!=null) {
                    StringTokenizer tokenizer = new StringTokenizer(str,"|");
                    if((msisdn =tokenizer.nextToken())!=null) {
                        msisdns.add(msisdn);
                    }
                }
            }catch (IOException e) {
                logger.error("Error during reading the file: "+distr.getFilePath(),e);
                throw new DistributionException("Error during reading the file: ", e);
            } finally {
                if(reader!=null) {
                    try {
                        reader.close();
                    } catch (IOException e) {
                        logger.error("Can't close file: "+distr.getFilePath(),e);
                        throw new DistributionException("Can't close file: "+distr.getFilePath(), e);
                    }
                }
            }

            createDeliveryFiles(msisdns,statsDir+"/"+"148");
            return "148";
        //todo Implementation
    }

    public ResultSet getStatistics(String id, Date startDate, Date endDate) throws DistributionException{
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(startDate);
        calendar.set(Calendar.MINUTE,0);
        calendar.set(Calendar.SECOND,0);
        calendar.set(Calendar.MILLISECOND,0);
        String path = statsDir+"/"+id;
        Date date;
        Set<File> files = new HashSet<File>();

        while(calendar.getTime().compareTo(endDate)<=0) {
            date = calendar.getTime();
            File file = new File(path +"/"+dirFormat.format(date)+"/"+fileFormat.format(date)+".csv");
            if(file.exists()) {
                try {
                    FileUtils.truncateFile(file,"\n".getBytes()[0],10);
                } catch (IOException e) {
                    logger.error("Unable to truncate file",e);
                    throw new DistributionException("Unable to truncate file",e);
                }
                files.add(file);
            }
            calendar.add(Calendar.HOUR_OF_DAY,1);
        }

        return new DistributionResultSet(files, startDate, endDate, succDeliveryStatus, dateInFilePattern);
    }

    //todo delete
    private void createDeliveryFiles(List<String> msisdns, String dir) throws DistributionException{
        if((msisdns==null)||(dir==null)) {
            logger.error("Some fields of argument are empty");
            throw new DistributionException("Some arguments are null", DistributionException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis());
        PrintWriter writer = null;
        try {
            String fileDir = dir+"/"+dirFormat.format(calendar.getTime());
            File file = new File(fileDir);
            if(!file.exists()) {
                file.mkdirs();
            }
            writer = new PrintWriter(new BufferedWriter(new FileWriter(fileDir+"/"+fileFormat.format(calendar.getTime())+".csv")));
            Random random = new Random();
            for(int i=0; i<msisdns.size()/2;i++) {
                writer.print(random.nextInt(10));
                writer.print(",");
                writer.print(dateInFileFormat.format(calendar.getTime()));
                writer.print(",");
                writer.println(msisdns.get(i));
            }
            writer.flush();
            writer.close();

            calendar.add(Calendar.HOUR_OF_DAY,2);
            fileDir = dir+"/"+dirFormat.format(calendar.getTime());
            file = new File(fileDir);
            if(!file.exists()) {
                file.mkdir();
            }
            writer = new PrintWriter(new BufferedWriter(new FileWriter(fileDir+"/"+fileFormat.format(calendar.getTime())+".csv")));
            for(int i=msisdns.size()/2; i<msisdns.size(); i++) {
                writer.print(random.nextInt(10));
                writer.print(",");
                writer.print(dateInFileFormat.format(calendar.getTime()));
                writer.print(",");
                writer.println(msisdns.get(i));
            }
            writer.flush();
            writer.close();
        } catch (IOException e) {
            logger.error("Unable to create delivery file", e);
            throw new DistributionException("Unable to create delivery file",e);
        } finally {
            if(writer!=null) {
                writer.close();
            }
        }
    }

	 
}
 
