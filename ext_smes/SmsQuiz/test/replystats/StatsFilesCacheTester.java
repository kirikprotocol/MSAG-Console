package replystats;

import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFilesCache;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Date;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Iterator;

/**
 * author: alkhal
 */
public class StatsFilesCacheTester {

    private StatsFilesCache filesCache;

    @Before
    public void init() {
        try {
            filesCache = new StatsFilesCache("conf/config.xml");
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
            reply.setText("getFileAddReplyCacheTest");
            file.open();
            file.add(reply);

            Collection<Reply> replies = new LinkedList<Reply>();
            file.list(new Date(0), new Date(new Date().getTime()+100000), replies);
            int flag=0;

            Iterator<Reply> iter = replies.iterator();
            while(iter.hasNext()) {
                Reply r = iter.next();
                if((!iter.hasNext())&&(r.getText().equals("getFileAddReplyCacheTest"))){
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
            files = filesCache.getFiles("148",new Date(System.currentTimeMillis()-1000000000), new Date(System.currentTimeMillis()+10000000));
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertFalse(true);
        }
        assertNotNull(files);
        for(StatsFile file : files) {
            try {
                file.open();
            } catch (FileStatsException e) {
                e.printStackTrace();
                assertFalse(true);
            } finally {
                file.close();
            }
        }
    }
    @Ignore
    @Test(timeout = 21000)
    public void openNotClose() {
        try {
            StatsFile file = filesCache.getFile("148",new Date());
            file.open();
            file.close();
            int count = filesCache.countOpenedFiles();
            System.out.println(count);
            Thread thread = new Thread() {
                public void run() {
                    try {
                        sleep(filesCache.getIterationPeriod()*2000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();}
                }
            };
            thread.start();
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            assertTrue(filesCache.countOpenedFiles() <= count - 1);
        } catch (FileStatsException e) {
            e.printStackTrace();
            assertFalse(true);
        }

    }
    @After
    public void shutdown() {
        if(filesCache!=null)
            filesCache.shutdown();
    }


}
