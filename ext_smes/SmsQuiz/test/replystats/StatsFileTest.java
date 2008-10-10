package replystats;

import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFileImpl;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Date;
import java.util.Collection;
import java.util.LinkedList;

/**
 * author: alkhal
 */
public class StatsFileTest {

    public StatsFile file;

    @Before
    public void init() {
        try {
            file = new StatsFileImpl("148","/home/alkhal/cvs/smsc/ext_smes/SmsQuiz/replyStats/148/20081007.csv");
            file.open();
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }


    @Test
    public void add() {
        try {
        Reply reply = new Reply();
        reply.setDate(new Date());
        reply.setOa("+7913testAdd");
        reply.setText("TestTestTest");
            file.add(reply);
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @Test
    public void list() {
        Collection<Reply> replies = new LinkedList<Reply>();
        try {
            file.list(new Date(0), new Date(System.currentTimeMillis()+2000), replies);
            for(Reply reply: replies) {
               System.out.println(reply);
            }
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @After
    public void close() {
        if(file!=null) {
            file.close();
        }
    }
}
