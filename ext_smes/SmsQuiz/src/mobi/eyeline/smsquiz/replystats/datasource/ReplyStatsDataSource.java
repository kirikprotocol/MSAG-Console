package mobi.eyeline.smsquiz.replystats.datasource;

import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Collection;
import java.util.Date;

public interface ReplyStatsDataSource {
	 
	public void add(Reply reply);
	public Collection list(String da, Date from, Date till);
	public abstract void shutdown();
}
 
