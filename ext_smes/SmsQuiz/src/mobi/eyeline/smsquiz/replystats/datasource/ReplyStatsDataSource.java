package mobi.eyeline.smsquiz.replystats.datasource;

import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Collection;
import java.util.Date;
import java.util.Map;

public interface ReplyStatsDataSource {

  public void add(Reply reply) throws ReplyDataSourceException;

  public Reply getLastReply(String oa, String da, Date from, Date till) throws ReplyDataSourceException;

  public void shutdown();
}
 
