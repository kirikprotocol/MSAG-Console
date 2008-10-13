package quizmanager;
import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertFalse;

import java.io.*;
import java.util.Random;

import mobi.eyeline.smsquiz.quizmanager.QuizManagerImpl;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.Notification;
import mobi.eyeline.smsquiz.quizmanager.QuizException;

/**
 * author: alkhal
 */
public class QuizManagerTest {

    private QuizManagerImpl quizManager;

    @Before
    public void init() {
        try {
            QuizManagerImpl.init("conf/config.xml");
            quizManager = (QuizManagerImpl)QuizManagerImpl.getInstanse();
            File file = new File("test_QuizManager");
            if(file.exists()) {
                removeAll(file);
            }
            file.mkdirs();
            createQuizFile("test_QuizManager/opros_test.xml");
            createAbFile("test_QuizManager/opros_test_ab.csv");
        } catch (Exception e) {
            e.printStackTrace();
            assertFalse(true);

        }
    }

    @Test
    public void initTest() {
        assertTrue(quizManager.countQuizes()==0);
    }


    @Test
    public void update() {
        try {
            Notification notification = new Notification("test_QuizManager/opros_test.xml", Notification.FileStatus.CREATED);
            quizManager.update(null,notification);
            assertTrue(quizManager.countQuizes()==1);
            File file = new File(quizManager.getStatusDir()+"/opros_test.status");
            assertTrue(file.exists());

            quizManager.update(null, notification);
            assertTrue(quizManager.countQuizes()==1);
            file.delete();

            notification = new Notification("test_QuizManager/opros_test.xml", Notification.FileStatus.MODIFIED);
            quizManager.update(null,notification);
            assertTrue(quizManager.countQuizes()==1);
            file = new File(quizManager.getStatusDir()+"/opros_test.status");
            assertTrue(file.exists());
            file.delete();
        } catch (QuizException e) {
            e.printStackTrace();
            assertFalse(true);
        }

    }


    @After
    public void stop() {
        quizManager.stop();;
    }

    private void removeAll(File dir) {
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

    private void createQuizFile(String fileName) {
        PrintWriter writer = null;
        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(fileName)));
            writer.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            writer.println("<opros> ");

            writer.println("    <general>");
            writer.println("        <status>active</status>");
            writer.println("        <date-begin>10.10.2008 09:00</date-begin>");
            writer.println("        <date-end>15.20.2008 22:00</date-end>");
            writer.println("        <question>Question</question>");
            writer.println("        <abonents-file>test_QuizManager/opros_test_ab.csv</abonents-file>");
            writer.println("    </general>");

            writer.println("    <distribution>");
            writer.println("        <source-address>148</source-address>");
            writer.println("        <time-begin>12:00</time-begin>");
            writer.println("        <time-end>20:00</time-end>");
            writer.println("        <days>");
            writer.println("            <day>Mon</day>");
            writer.println("            <day>Sun</day>");
            writer.println("        </days>");
            writer.println("        <txmode>transaction</txmode>");
            writer.println("    </distribution>");

            writer.println("    <replies>");
            writer.println("        <destination-address>148</destination-address>");
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
            writer.println("</opros> ");
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
    private void createAbFile(String distrList) {
        PrintWriter writer = null;
        File file = (new File(distrList)).getParentFile();
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



}
