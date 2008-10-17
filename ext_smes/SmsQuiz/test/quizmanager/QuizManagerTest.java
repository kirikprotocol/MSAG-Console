package quizmanager;
import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;

import java.io.*;
import java.util.Calendar;
import java.util.Date;
import java.util.StringTokenizer;
import java.util.Random;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.quizmanager.QuizManagerImpl;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import mobi.eyeline.smsquiz.quizmanager.Result;
import mobi.eyeline.smsquiz.quizmanager.service.ReplySMPPService;
import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
import mobi.eyeline.smsquiz.subscription.SubManagerException;
import com.eyeline.sme.handler.config.ServicesConfig;
import com.eyeline.sme.handler.config.ServicesConfigReader;
import com.eyeline.sme.handler.RequestToServiceMap;
import com.eyeline.sme.handler.SMPPService;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.IncomingObject;
import com.eyeline.utils.tree.radix.StringsRTree;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;

/**
 * author: alkhal
 */
public class QuizManagerTest {

    public static QuizManagerImpl quizManager;
    private static SubscriptionManager subscriptionManager;
    private static Date dateBegin;
    private static Date dateEnd;
    private static SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");

    private static String number1="+7909";
    private static String number2="+7910";
    private static String number3="+8000";
    private static int divider = 1000;
    private static String oa = "148";
    private static String da="148";
    private static long initWait = 10000;
    private static long abonents = 15000;
    private static int minutes = 3;
    private static String[] answers = {"yes","no","dsasddssdds"};

    @BeforeClass
    public static void init() {
        try {
            ConnectionPoolFactory.init("conf/config.xml");
            subscriptionManager = SubscriptionManager.getInstance();
            QuizManagerImpl.init("conf/config.xml");
            quizManager = (QuizManagerImpl)QuizManagerImpl.getInstanse();
            File file = new File("test_QuizManager");
            if(file.exists()) {
                removeAll(file);
            }
            file.mkdirs();
            createQuizFile("test_QuizManager/opros_test.xml");
            createAbFile("test_QuizManager/opros_test_ab.csv");
            quizManager.start();
            waiting(initWait);
        } catch (Exception e) {
            e.printStackTrace();
            assertFalse(true);

        }
    }


    @Test
    public void handleSms() {
       try{
           Result result = quizManager.handleSms(da,number1,"y");
           assertNotNull(result);
           assertTrue(result.getReplyRull().equals(Result.ReplyRull.OK));
           for(int i=0;i<10;i++) {
                quizManager.handleSms(da,number1,"asfaf");
           }
           assertNull(quizManager.handleSms(da,number1,"asfaf"));
           File file = new File(quizManager.getStatusDir()+"/opros_test.status");
           assertTrue(file.exists());
           file.delete();
           waiting(25000);
       } catch (QuizException e) {
           e.printStackTrace();
           assertTrue(false);
       }
    }

    @Test
    public void testRequestMapping() {
        try{
            ServicesConfig config = ServicesConfigReader.readConfig("conf/services_reply.xml");
            RequestToServiceMap requestToServiceMap = new RequestToServiceMap(config,new OutgoingQueue(20));
            IncomingObject incObj = createIncObj("+7",da,"dsads");
            RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
            assertTrue(e.getService().getClass().getName().equals(ReplySMPPService.class.getName()));
        } catch (Exception e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @Test
    public void testService() {
        try{
            ServicesConfig config = ServicesConfigReader.readConfig("conf/services_reply.xml");
            RequestToServiceMap requestToServiceMap = new RequestToServiceMap(config,new OutgoingQueue(20));
            IncomingObject incObj = createIncObj(number2,da,"no");
            RequestToServiceMap.Entry e = requestToServiceMap.getEntry(incObj);
            SMPPService service = e.getService();
            assertNotNull(service);
            assertTrue(service.serve(e.getPattern().createRequest(incObj)));

            incObj = createIncObj(number3,da,"dsd");
            e = requestToServiceMap.getEntry(incObj);
            service = e.getService();
            assertNotNull(service);
            for(int i=0;i<10;i++) {
                service.serve(e.getPattern().createRequest(incObj));
            }
        } catch (Exception e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }



    @Test
    public void perfTest() {
       try{
           Random random = new Random();
           int i = Integer.parseInt(number2.substring(1,number2.length()))+1;
           for(;i<abonents;i++) {
                quizManager.handleSms(da, "+"+Integer.toString(i),answers[random.nextInt(3)]);
                quizManager.handleSms(da, "+"+Integer.toString(i),answers[random.nextInt(3)]);
           }
       } catch (QuizException e) {
           e.printStackTrace();
           assertTrue(false);
       }
    }

    @Test
    public void stats() {
        waiting(dateEnd.getTime() - dateBegin.getTime());
        SimpleDateFormat dateFormat = new SimpleDateFormat("ddMMyy_HHmmss");
        String fileName = quizManager.getDirResult()+"/"+da+"."+dateFormat.format(dateBegin)+"-"+dateFormat.format(dateEnd)+".res";
        File file = new File(fileName);
        assertTrue(file.exists());
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(file));
            String line = null;
            StringTokenizer tokenizer= null;
            int flag=0;
            while((line = reader.readLine())!=null) {
                flag=1;
                tokenizer = new StringTokenizer(line,",");
                String msisdn = tokenizer.nextToken();
                if(msisdn.equals(number1)) {
                    tokenizer.nextToken();tokenizer.nextToken();
                    assertTrue(tokenizer.nextToken().equals("Да"));
                    assertTrue(tokenizer.nextToken().equals("asfaf"));
                    continue;
                }
                if(msisdn.equals(number2)) {
                    tokenizer.nextToken();tokenizer.nextToken();
                    assertTrue(tokenizer.nextToken().equals("Нет"));
                    assertTrue(tokenizer.nextToken().equals("no"));
                    continue;
                }
                if((Integer.parseInt(msisdn.substring(1,msisdn.length()))%divider)==0){
                  //  tokenizer.nextToken();tokenizer.nextToken();
                  // assertTrue(tokenizer.nextToken().equals("Да"));
                    continue;
                }
                assertTrue(false);
            }
            assertTrue(flag==1);
        } catch (Exception e) {
            e.printStackTrace();
            assertFalse(true);
        } finally {
            if(reader!=null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

    }

    @AfterClass
    public static void stop() {
        quizManager.stop();
    }



    /* --------------------------------- Utils --------------------------------------------- */

    private static void removeAll(File dir) {
        if((dir==null)||(!dir.exists())) {
            return;
        }
        if(dir.isDirectory()) {
            for(File f:dir.listFiles()) {
                removeAll(f);
            }
        }
        if(dir.isFile()) {
            dir.delete();
        }
    }

    private static void createQuizFile(String fileName) {
        Calendar cal = Calendar.getInstance();
        cal.setTimeInMillis(System.currentTimeMillis());
        cal.set(Calendar.SECOND,0);
        dateBegin = cal.getTime();
        cal.add(Calendar.MINUTE,minutes);
        dateEnd = cal.getTime();

        PrintWriter writer = null;
        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(fileName)));
            writer.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            writer.println("<opros>");

            writer.println("    <general>");
            writer.print("        <date-begin>");writer.print(dateFormat.format(dateBegin)); writer.println("</date-begin>");
            writer.print("        <date-end>"); writer.print(dateFormat.format(dateEnd));writer.println("</date-end>");
            writer.println("        <question>Question</question>");
            writer.println("        <abonents-file>test_QuizManager/opros_test_ab.csv</abonents-file>");
            writer.println("    </general>");

            writer.println("    <distribution>");
            writer.print("        <source-address>");writer.print(oa);writer.println("</source-address>");
            writer.println("        <time-begin>12:00</time-begin>");
            writer.println("        <time-end>20:00</time-end>");
            writer.println("        <days>");
            writer.println("            <day>Mon</day>");
            writer.println("            <day>Sun</day>");
            writer.println("        </days>");
            writer.println("        <txmode>transaction</txmode>");
            writer.println("    </distribution>");

            writer.println("    <replies>");
            writer.print("        <destination-address>");writer.print(da);writer.println("</destination-address>");
            writer.println("        <max-repeat>3</max-repeat>");

            writer.println("        <reply>");
            writer.println("            <category>Да</category>");
            writer.println("            <pattern>(yes|y|1|да|д)</pattern>");
            writer.println("            <answer>Thanks</answer>");
            writer.println("        </reply>");

            writer.println("        <reply>");
            writer.println("            <category>Нет</category>");
            writer.println("            <pattern>(no|n|0|нет|н)</pattern>");
            writer.println("            <answer>Thanks</answer>");
            writer.println("        </reply>");

            writer.println("        <default>Да</default>");

            writer.println("    </replies>");
            writer.println("</opros>");
            writer.flush();
        } catch (IOException e) {
            e.printStackTrace();
            assertTrue(false);
        }  finally {
            if(writer!=null) {
                writer.close();
            }
        }
    }
    private static void createAbFile(String distrList) {
        PrintWriter writer = null;
        File file = (new File(distrList)).getParentFile();
        if(!file.exists()) {
            file.mkdirs();
        }

        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(distrList)));
            try {
                subscriptionManager.subscribe(number1);
                subscriptionManager.subscribe(number2);
            } catch (SubManagerException e) {
                e.printStackTrace();
            }
                String abonent = number1;
                writer.print(abonent);
                writer.print("|");

                StringBuilder strBuilder = new StringBuilder(30);
                int aCode = (int)'a';
                for (int j=0;j<20;j++){
                    strBuilder.append( (char)( aCode + 26*Math.random() ) );
                }
                writer.println(strBuilder.substring(0));
                abonent = number2;
                writer.print(abonent);
                writer.print("|");

                strBuilder = new StringBuilder(20);
                for (int j=0;j<20;j++){
                    strBuilder.append( (char)( aCode + 26*Math.random() ) );
                }
                writer.println(strBuilder.substring(0));
            for(int i=7911; i<abonents;i++) {
                abonent = "+"+ i;
                writer.print(abonent);
                writer.print("|");
                if((i%divider)==0) {
                    try {
                        subscriptionManager.subscribe(abonent);
                    } catch (SubManagerException e) {
                        e.printStackTrace();
                    }
                }
                strBuilder = new StringBuilder(20);
                aCode = (int)'a';
                for (int j=0;j<20;j++){
                    strBuilder.append( (char)( aCode + 26*Math.random() ) );
                }
                writer.println(strBuilder.substring(0));
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

    private static void waiting(final long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private class TestIncomingObject implements IncomingObject {
        private Message message;
        public TestIncomingObject(Message message) {
            this.message = message;
        }
        public Message getMessage() {
            return message;
        }

        public boolean isResponded() {
            return false;
        }

        public void respond(int status) throws SMPPException {
        }

    }
    private IncomingObject createIncObj(String oa, String da, String text) {
        Message message = new Message();
        message.setDestinationAddress(da);
        message.setSourceAddress(oa);
        message.setMessageString(text);
        return new TestIncomingObject(message);
    }


}
