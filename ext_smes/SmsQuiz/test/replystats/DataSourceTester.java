package replystats;

import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertFalse;
import mobi.eyeline.smsquiz.replystats.statsfile.StatsFilesCache;
import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.FileReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Date;
import java.util.Calendar;
import java.io.File;

/**
 * author: alkhal
 */
public class DataSourceTester {

    private ReplyStatsDataSource dataSource;

    @Before
    public void init() {
        try {
            StatsFilesCache.init("conf/config.xml");
            dataSource = new FileReplyStatsDataSource();
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertFalse(true);
        }
    }

    @Test
    public void add() {
        Reply reply = new Reply();
        reply.setDate(new Date());
        reply.setOa("+7913testAdd");
        reply.setDa("148");
        reply.setText("addDatasource");
        try {
            dataSource.add(reply);
        } catch (ReplyDataSourceException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @Test
    public void list() {
        try {
            int flag=0;
            Calendar cal = Calendar.getInstance();
            cal.setTime(new Date(System.currentTimeMillis()-2000));
            cal.set(Calendar.SECOND, 0);
            cal.set(Calendar.MILLISECOND, 0);
            Date from = cal.getTime();
            cal.add(Calendar.MINUTE,1);
            Date till = cal.getTime();
            for (Reply reply:dataSource.list("148",from, till)){
                if(reply.getText().equals("addDatasource"))
                    if(reply.getOa().equals("+7913testAdd"))
                        if(reply.getDa().equals("148"))
                            flag = 1;
            }
            assertTrue(flag==1);
        } catch (ReplyDataSourceException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @After
    public void shutdown() {
        if(dataSource !=null)
            try {
                dataSource.shutdown();
            } catch (ReplyDataSourceException e) {
                e.printStackTrace();
                assertTrue(false);
            }
    }
}
