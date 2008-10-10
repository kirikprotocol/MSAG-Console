package mobi.eyeline.smsquiz.replystats.datasource;

import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Collection;
import java.util.Date;

public interface ReplyStatsDataSource {
	 
	public void add(Reply reply) throws ReplyDataSourceException;
	public Collection<Reply> list(String da, Date from, Date till) throws ReplyDataSourceException;   
	public void shutdown() throws ReplyDataSourceException;
}
 
