package mobi.eyeline.smsquiz.replystats;

import java.util.Date;

public class Reply {
 
	private Date date;
	 
	private String oa;
	 
	private String da;
	 
	private String text;

    public Date getDate() {
        return date;
    }

    public String getOa() {
        return oa;
    }

    public String getDa() {
        return da;
    }

    public String getText() {
        return text;
    }

    public void setDate(Date date) {
        this.date = date;
    }

    public void setOa(String oa) {
        this.oa = oa;
    }

    public void setDa(String da) {
        this.da = da;
    }

    public void setText(String text) {
        this.text = text;
    }
    public String toString() {
        String result = "";
        if(date!=null) {
            result+=date+" ";
        }
        if(oa!=null) {
            result+=oa+" ";
        }
        if(da!=null) {
            result+=da+" ";
        }
        if(text!=null) {
            result+=text;
        }
        return result;
    }
}
 
