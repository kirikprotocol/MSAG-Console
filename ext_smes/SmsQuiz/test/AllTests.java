import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import subscription.DBDataSourceTester;
import subscription.SmsQuizTest;
import subscription.SubscriptionManagerTest;
import replystats.StatsFilesCacheTester;
import replystats.DataSourceTester;
import distribution.DistributionManagerTester;

import java.util.Calendar;

/**
 * Created by IntelliJ IDEA.
 * User: alkhal
 * Date: 30.09.2008
 * Time: 9:30:24
 * To change this template use File | Settings | File Templates.
 */

@RunWith(value= Suite.class)
@Suite.SuiteClasses(value={DBDataSourceTester.class, SubscriptionManagerTest.class, SmsQuizTest.class, StatsFilesCacheTester.class ,DataSourceTester.class, DistributionManagerTester.class})
public class AllTests {
    public static void main(String[] args) {
        System.out.println("Tests begin");
    }
}
