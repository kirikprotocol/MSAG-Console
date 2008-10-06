package mobi.eyeline.smsquiz.replystats.statsfile;

import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Collection;
import java.util.Date;

public interface StatsFile {

    public void open() throws FileStatsException;
	public void add(Reply reply) throws FileStatsException;
	public void list(Date from, Date till, Collection result) throws FileStatsException;   // todo
	public void close();
}
 
