import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import subscription.DBDataSourceTester;
import subscription.SubscriptionServiceTest;
import subscription.SubscriptionManagerTest;
import replystats.StatsFilesCacheTester;
import replystats.DataSourceTester;
import distribution.DistributionManagerTester;
import quizmanager.QuizManagerTest;
import quizmanager.DirListenerTest;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;


@RunWith(value = Suite.class)
@Suite.SuiteClasses(value = {DBDataSourceTester.class, SubscriptionManagerTest.class,
    SubscriptionServiceTest.class, StatsFilesCacheTester.class, DataSourceTester.class,
    StatsFilesCacheTester.class, DistributionManagerTester.class, DirListenerTest.class,
    QuizManagerTest.class})
public class AllTests {
  public static void main(String[] args) {
    System.out.println("Tests begin");
  }

  public static void createQuizes(SubscriptionManager subscriptionManager) {
    QuizManagerTest.createQuizFile("test_QuizManager/short.xml", 4, "170", "170");
    QuizManagerTest.createQuizFile("test_QuizManager/medium.xml", 5, "180", "180");
    QuizManagerTest.createQuizFile("test_QuizManager/long.xml", 6, "190", "190");

    QuizManagerTest.createAbFile("test_QuizManager/short_ab.xml", 7911, 15000, subscriptionManager);
    QuizManagerTest.createAbFile("test_QuizManager/medium_ab.xml", 15000, 20000, subscriptionManager);
    QuizManagerTest.createAbFile("test_QuizManager/long_ab.xml", 20000, 27000, subscriptionManager);
  }
}
