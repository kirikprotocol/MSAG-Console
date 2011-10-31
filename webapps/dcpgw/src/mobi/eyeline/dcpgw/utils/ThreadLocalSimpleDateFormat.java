package mobi.eyeline.dcpgw.utils;

import org.apache.log4j.Logger;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 31.10.11
 * Time: 15:10
 */
public class ThreadLocalSimpleDateFormat{

    private static Logger log = Logger.getLogger(ThreadLocalSimpleDateFormat.class);

    private static final ThreadLocal<SimpleDateFormat> formatter =
                                                     new ThreadLocal<SimpleDateFormat>() {
        protected SimpleDateFormat initialValue() {
            log.debug("Create new instance of simple date format for thread "+Thread.currentThread().getName());
            return new SimpleDateFormat("yyMMddHHmmssZ");
        }
    };

    public Date parse(String text) throws ParseException {
        return formatter.get().parse(text);
    }

    public String format(Date date)
    {
        return formatter.get().format(date);
    }
}
