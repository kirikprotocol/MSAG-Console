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

}
