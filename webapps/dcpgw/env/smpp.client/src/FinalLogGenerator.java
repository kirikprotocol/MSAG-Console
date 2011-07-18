import org.apache.log4j.Logger;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * Created by IntelliJ IDEA.
 * User: stepanov
 * Date: 13.07.11
 * Time: 10:11
 */
public class FinalLogGenerator {

    private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");

    private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

    private static Logger log = Logger.getLogger(FinalLogGenerator.class);

    private File final_log_dir;
    private Calendar cal;

    public FinalLogGenerator(File final_log_dir){
        this.final_log_dir = final_log_dir;
        cal = Calendar.getInstance();
        cal.setTimeZone(STAT_TIMEZONE);
    }

    public void writeFinalState(long message_id){
        log.debug("Try to write final state for message "+message_id);
        cal.setTimeInMillis(System.currentTimeMillis());
        Date date = cal.getTime();
        SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmm");

        SimpleDateFormat sdf_sec = new SimpleDateFormat("ss");

        File current_file = new File(final_log_dir, sdf.format(date)+".csv");
        if (!current_file.exists()){
            try {
                if (!current_file.createNewFile()) log.error("Couldn't create file "+current_file.getName());
            } catch (IOException e) {
                log.debug(e);
            }
        }

        String line = sdf_sec.format(date)+",3,a,0,"+System.currentTimeMillis()+",D,0,+79139138729,1,id="+message_id;

        BufferedWriter bw = null;

        try {
            bw = new BufferedWriter(new FileWriter(current_file, true));
            bw.write(line + "\n");
        } catch (IOException e) {
            log.error(e);
        } finally {
            try{
                if (bw != null) {
                    bw.close();
                }
            }catch(IOException ioe1){
                log.error(ioe1);
            }
        }



    }


}
