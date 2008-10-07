package mobi.eyeline.smsquiz.distribution;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

import java.util.Date;
import java.io.File;
import java.text.SimpleDateFormat;

import org.apache.log4j.Logger;

public class DistributionInfoSmeManager implements DistributionManager {

    private static String statsDir;
    private static String dirPattern;
    private static String filePattern;
    private static Logger logger = Logger.getLogger(DistributionInfoSmeManager.class);

    private SimpleDateFormat dirFormat;
    private SimpleDateFormat fileFormat;

    public static void init(final String configFile) throws DistributionException{
        try {
            final XmlConfig c = new XmlConfig();
            c.load(new File(configFile));

            PropertiesConfig config = new PropertiesConfig(c.getSection("replystats").toProperties("."));

            statsDir = config.getString("statsFile.dir.name",null);
            dirPattern = config.getString("info.sme.stats.date.dir.pattern",null);
            filePattern = config.getString("info.sme.stats.time.file.pattern",null);
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
    }

    public String createDistribution(Distribution distr) {
		return null;
	}

    public ResultSet getStatistics(String id, Date startDate, Date endDate) {
        String filePath;
        return null;
    }

	 
}
 
