package mobi.eyeline.smsquiz.distribution;

import java.util.*;

public class Distribution {
    public String getSourceAddress() {
        return sourceAddress;
    }

    public void setSourceAddress(String sourceAddress) {
        this.sourceAddress = sourceAddress;
    }

    public static enum WeekDays {MON,TUE,WED,THU,FRI,SAT,SUN}

    private String sourceAddress;

    private String filePath;
	 
	private Date dateBegin;
	 
	private Date dateEnd;
	 
	private Calendar timeBegin;
	 
	private Calendar timeEnd;
	 
	private EnumSet<WeekDays> days;

    private String txmode;

    public Distribution() {
        days = EnumSet.allOf(WeekDays.class);
    }

    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    public Date getDateBegin() {
        return dateBegin;
    }

    public void setDateBegin(Date dateBegin) {
        this.dateBegin = dateBegin;
    }

    public Date getDateEnd() {
        return dateEnd;
    }

    public void setDateEnd(Date dateEnd) {
        this.dateEnd = dateEnd;
    }

    public Calendar getTimeBegin() {
        return timeBegin;
    }

    public void setTimeBegin(Calendar timeBegin) {
        this.timeBegin = timeBegin;
    }

    public Calendar getTimeEnd() {
        return timeEnd;
    }

    public void setTimeEnd(Calendar timeEnd) {
        this.timeEnd = timeEnd;
    }

    public void addDay(WeekDays weekDays) {
        days.add(weekDays);
    }

    public EnumSet<WeekDays> getDays() {
        return EnumSet.copyOf(days);
    }
    public String getTxmode() {
        return txmode;
    }
    public void setTxmode(String txmode) {
        this.txmode = txmode;
    }
}
 
