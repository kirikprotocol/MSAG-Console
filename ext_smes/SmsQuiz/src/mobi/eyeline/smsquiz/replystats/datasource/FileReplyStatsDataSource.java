package mobi.eyeline.smsquiz.replystats.datasource;

import mobi.eyeline.smsquiz.replystats.statsfile.StatsFilesCache;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Collection;
import java.util.Date;

public class FileReplyStatsDataSource implements ReplyStatsDataSource {
 
	private StatsFilesCache statsFilesCache;
	 

	public void add(Reply reply) {
	 
	}
	 

	public Collection list(String da, Date from, Date till) {
		return null;
	}
	 

	public void shutdown() {
	 
	}
	 
}
 
