package mobi.eyeline.smsquiz.replystats.statsfile;

import mobi.eyeline.smsquiz.replystats.ReplyStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Collection;
import java.util.Date;

public interface StatsFile {

    public void open() throws ReplyStatsException;
	public void add(Reply reply) throws ReplyStatsException;
	public void list(Date from, Date till, Collection result) throws ReplyStatsException;
	public void close();
    public boolean exist();
}
 
