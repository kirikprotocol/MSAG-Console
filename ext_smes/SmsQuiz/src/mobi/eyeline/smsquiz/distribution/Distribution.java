package mobi.eyeline.smsquiz.distribution;

import java.util.*;

public class Distribution {

    public static enum WeekDays {MONDAY,TUESDAY,WEDNESDAY,THURSDAY,FRIDAY,SATURDAY,SUNDAY}

    private String filePath;
	 
	private Date dateBegin;
	 
	private Date dateEnd;
	 
	private Calendar timeBegin;
	 
	private Calendar timeEnd;
	 
	private Set<WeekDays> days;

    public Distribution() {
        days = new HashSet<WeekDays>();
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

    public Collection<WeekDays> getDays() {
        return days;
    }
}
 
