package mobi.eyeline.smsquiz.replystats.datasource;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Date;

public interface ReplyStatsDataSource {

  public void add(Reply reply) throws ReplyDataSourceException;

  public Reply getLastReply(String oa, String da, Date from, Date till) throws ReplyDataSourceException;

  public void shutdown();

  public AbstractDynamicMBean getMonitor();
}
 
