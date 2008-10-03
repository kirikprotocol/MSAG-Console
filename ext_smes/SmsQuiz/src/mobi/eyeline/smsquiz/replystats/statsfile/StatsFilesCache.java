package mobi.eyeline.smsquiz.replystats.statsfile;

import mobi.eyeline.smsquiz.replystats.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.statsfile.CachedStatsFile;
import mobi.eyeline.smsquiz.replystats.datasource.FileReplyStatsDataSource;

import java.util.Date;
import java.util.Collection;

public class StatsFilesCache {
 
	private CachedStatsFile cachedStatsFile;
	 
	private FileReplyStatsDataSource fileReplyStatsDataSource;
	 
	public Collection getFiles(String da, Date from, Date till) {
		return null;
	}
	 
	public StatsFile getFile(String da, Date date) {
		return null;
	}
	 
	public void shutdown() {
	 
	}
	 
}
 
