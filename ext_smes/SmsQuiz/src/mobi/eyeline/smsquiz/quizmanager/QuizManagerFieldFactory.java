package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.Impl.DistributionInfoSmeManager;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.datasource.impl.FileReplyStatsDataSource;

/**
 * author: alkhal
 */
public class QuizManagerFieldFactory {

    private static DistributionManager distributionManager;
    private static ReplyStatsDataSource replyStatsDataSource;

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
}
