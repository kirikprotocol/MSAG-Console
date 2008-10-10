package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Collection;
import java.util.Date;

public interface StatsFile {

    public void open() throws FileStatsException;
	public void add(Reply reply) throws FileStatsException;
	public void list(Date from, Date till, Collection<Reply> result) throws FileStatsException;   
	public void close();
}
 
