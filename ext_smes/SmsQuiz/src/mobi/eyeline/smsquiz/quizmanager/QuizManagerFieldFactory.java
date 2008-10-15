package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.Impl.DistributionInfoSmeManager;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.datasource.impl.FileReplyStatsDataSource;
import mobi.eyeline.smsquiz.subscription.datasource.SubscriptionDataSource;
import mobi.eyeline.smsquiz.subscription.datasource.impl.DBSubscriptionDataSource;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.storage.ConnectionPoolFactory;

/**
 * author: alkhal
 */
public class QuizManagerFieldFactory {

    private static DistributionManager distributionManager;
    private static ReplyStatsDataSource replyStatsDataSource;
    private static SubscriptionDataSource subscriptionDataSource;

    public static synchronized DistributionManager getDistributionManager(final String configFile) throws DistributionException {
        if(distributionManager==null) {
            distributionManager = new DistributionInfoSmeManager(configFile);
        }
        return distributionManager;
    }
    public static synchronized ReplyStatsDataSource getReplyStatsDataSource(final String configFile) throws ReplyDataSourceException {
        if(replyStatsDataSource==null) {
            replyStatsDataSource = new FileReplyStatsDataSource(configFile);
        }
        return replyStatsDataSource;
    }
    public static synchronized SubscriptionDataSource getSubscriptionDataSource(final String configFile) throws StorageException {
        if(subscriptionDataSource==null) {
            ConnectionPoolFactory.init(configFile);
            subscriptionDataSource = new DBSubscriptionDataSource();
        }
        return subscriptionDataSource;
    }
}
