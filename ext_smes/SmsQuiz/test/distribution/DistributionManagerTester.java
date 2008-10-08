package distribution;
import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import mobi.eyeline.smsquiz.distribution.*;
import mobi.eyeline.smsquiz.distribution.Impl.DistributionInfoSmeManager;

import java.io.*;
import java.util.Random;
import java.util.Calendar;
import java.util.Date;
import java.util.HashSet;

/**
 * author: alkhal
 */
public class DistributionManagerTester {

    private static DistributionManager distributionManager;
    private static String distrList = "/home/alkhal/cvs/smsc/ext_smes/SmsQuiz/infoSme/distribution.csv";   // todo
    private static String id = null;
    private static HashSet<String> abonents;

    @BeforeClass
    public static void init() {
        abonents = new HashSet<String>();
        try {
            DistributionInfoSmeManager.init("conf/config.xml");
            distributionManager = new DistributionInfoSmeManager();
        } catch (DistributionException e) {
            e.printStackTrace();
            assertFalse(true);
        }
        PrintWriter writer = null;
        File file = (new File(distrList)).getParentFile();
        deleteFiles(file);
        if(!file.exists()) {
            file.mkdirs();
        }

        Random random = new Random();
        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(distrList)));
            for(int i=0; i<40;i++) {
                String abonent = "+7"+Math.abs(random.nextInt());
                writer.print(abonent);
                writer.print("|");

                StringBuilder strBuilder = new StringBuilder(20);
                int aCode = (int)'a';
                for (int j=0;j<20;j++){
                    strBuilder.append( (char)( aCode + 26*Math.random() ) );
                }
                writer.println(strBuilder.substring(0));
                abonents.add(abonent);
            }
            writer.flush();
        } catch (IOException e) {
            e.printStackTrace();
            assertTrue(false);
        } finally {
            if(writer!=null) {
                writer.close();
            }
        }

    }

    @Test
    public void createDistribution() {
        Distribution distribution = new Distribution();
        Calendar calTime1 = Calendar.getInstance();
        Calendar calTime2 = Calendar.getInstance();
        calTime1.set(Calendar.HOUR_OF_DAY,12);
        calTime2.set(Calendar.HOUR_OF_DAY,20);
        calTime1.set(Calendar.MINUTE,0);
        calTime2.set(Calendar.MINUTE,0);

        Calendar calDate = Calendar.getInstance();
        calDate.setTimeInMillis(System.currentTimeMillis());
        calDate.add(Calendar.DAY_OF_MONTH,10);

        distribution.setDateBegin(calDate.getTime());
        calDate.add(Calendar.DAY_OF_MONTH,4);

        distribution.setDateEnd(calDate.getTime());
        distribution.setFilePath(distrList);

        distribution.addDay(Distribution.WeekDays.FRIDAY);
        distribution.addDay(Distribution.WeekDays.SUNDAY);

        distribution.setTimeBegin(calTime1);
        distribution.setTimeEnd(calTime2);
        try {
            id = distributionManager.createDistribution(distribution);
            assertNotNull(id);            
        } catch (DistributionException e) {
            e.printStackTrace();
            assertFalse(true);
        }
    }

    @Test
    public void getReply() {
        Calendar cal = Calendar.getInstance();
        cal.setTimeInMillis(System.currentTimeMillis());
        cal.add(Calendar.DAY_OF_MONTH,-3);
        Date begin = cal.getTime();
        cal.add(Calendar.DAY_OF_MONTH,3);
        cal.add(Calendar.MINUTE,30);
        Date end = cal.getTime();
        ResultSet resultSet = null;
        try {
            resultSet = distributionManager.getStatistics(id, begin, end);
            while(resultSet.next()) {
                assertTrue(abonents.contains(resultSet.get().toString()));
                System.out.println(resultSet.get().toString());
            }
        } catch (DistributionException e) {
            e.printStackTrace();
            assertFalse(true);
        }  finally {
            if(resultSet!=null) {
                resultSet.close();
            }
        }
    }
    private static void deleteFiles(File file) {
        if(file.isDirectory()) {
            for(File f:file.listFiles()) {
                deleteFiles(f);
            }
        }
        if(file.isFile()) {
            file.delete();
        }
    }
    
}
