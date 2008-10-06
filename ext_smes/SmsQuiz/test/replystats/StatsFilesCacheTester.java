package replystats;

import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import mobi.eyeline.smsquiz.replystats.statsfile.StatsFilesCache;
import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.statsfile.StatsFileImpl;
import mobi.eyeline.smsquiz.replystats.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Date;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Iterator;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * author: alkhal
 */
public class StatsFilesCacheTester {

    private StatsFilesCache filesCache;

    @Before
    public void init() {
        try {
            StatsFilesCache.init("conf/replystats.xml");
            filesCache = new StatsFilesCache();
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertFalse(true);
        }
    }

    
    @Test
    public void getFileAddReply() {
        StatsFile file = null;
        try {
            file = filesCache.getFile("148",new Date());
            Reply reply = new Reply();
            reply.setDate(new Date());
            reply.setOa("+7913testAdd");
            reply.setText("getFileAddReply");
            file.open();
            file.add(reply);

            Collection<Reply> replies = new LinkedList<Reply>();
            file.list(new Date(0), new Date(new Date().getTime()+1000), replies);
            int flag=0;

            Iterator<Reply> iter = replies.iterator();
            while(iter.hasNext()) {
                Reply r = iter.next();
                if((!iter.hasNext())&&(r.getText().equals("getFileAddReply"))){
                    flag = 1;
                    break;
                }
            }
            assertTrue(flag==1);
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertFalse(true);
        } finally {
            if(file!=null) {
                file.close();
            }
        }
    }


    @Test
    public void getFiles() {
        Collection<StatsFile> files = null;
        try {
            files = filesCache.getFiles("148",new SimpleDateFormat("ddMMyyy").parse("06102008"), new SimpleDateFormat("ddMMyyy").parse("07102008"));
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertFalse(true);
        } catch (ParseException e) {
            e.printStackTrace();
            assertFalse(true);
        }
        assertNotNull(files);
        for(StatsFile file : files) {
            try {
                file.open();
            } catch (FileStatsException e) {
                e.printStackTrace();
                file.close();
                assertFalse(true);
            }
        }
    }
    @Test
    public void openNotClose() {

    }
    @After
    public void shutdown() {
        if(filesCache!=null)
            filesCache.shutdown();
    }


}
