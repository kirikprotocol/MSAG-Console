package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.dcpgw.Utils;
import mobi.eyeline.dcpgw.exeptions.CouldNotCleanJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.05.11
 * Time: 16:58
 */
public class Journal {

    private static Logger log = Logger.getLogger(Journal.class);

    private static final String sep=";";

    private int max_journal_size_mb = 10;

    private File dir, j1, j2, j2t;

    private DateFormat df;
    private Calendar cal;

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmss");

    private Hashtable<Long, Data> message_id_receipt_table;
    private Hashtable<Integer, Long> sequence_number_message_id_table;


    public Journal(Hashtable<Long, Data> message_id_receipt_table, Hashtable<Integer, Long> sequence_number_message_id_table) throws InitializationException{
        this.message_id_receipt_table = message_id_receipt_table;
        this.sequence_number_message_id_table = sequence_number_message_id_table;

        String userDir = System.getProperty("user.dir");
        String filename = userDir+File.separator+"conf"+File.separator+"config.properties";

        Properties prop = new Properties();

        try{
            prop.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.error(e);
            System.exit(1);
        }

        max_journal_size_mb = Utils.getProperty(prop, "max.journal.size.mb", 10);

        String journal_dir_str = Utils.getProperty(prop, "journal.dir", userDir+File.separator+"journal");

        dir = new File(journal_dir_str);
        j1 = new File(dir, "j1.csv");
        j2 = new File(dir, "j2.csv");
        j2t = new File(dir, "j2.csv.tmp");

        if (!dir.exists()){
            log.debug("Detected that journal directory doesn't exist.");
            if (dir.mkdir()){
                log.debug("Successfully create journal directory.");
            } else {
                log.error("Couldn't create journal directory, check permissions.");
                throw new InitializationException("Couldn't create journal directory, check permissions.");
            }
        } else {
            log.debug("Detected that journal directory already exists.");
        }

        df = DateFormat.getDateTimeInstance();
        cal = Calendar.getInstance();
    }

    private void appendFile(File source, File target) throws IOException {
        log.debug("Try to append file '"+source.getName()+"' to file '"+target.getName()+"'.");
        BufferedOutputStream bufOut = new BufferedOutputStream(new FileOutputStream(target, true));

        BufferedInputStream bufRead = new BufferedInputStream(new FileInputStream(source));

        int n;

        while((n = bufRead.read()) != -1) {
            bufOut.write(n);
        }

        bufOut.flush();

        bufOut.close();

        bufRead.close();
        log.debug("Successfully append journal file.");

    }

    /**
     *
     * first_sending_date; last_resending_date; message_id; sequence_number; source_address; dest_address; connection_name; submit_date; done_date; final_message_state; nsms; status
     * @param data
     * @throws CouldNotWriteToJournalException
     */

    public void write(Data data) throws CouldNotWriteToJournalException {
        cal.setTimeInMillis(data.getFirstSendingTime());
        Date first_sending_date = cal.getTime();
        cal.setTimeInMillis(data.getLastResendTime());
        Date last_resending_time = cal.getTime();
        String s = df.format(first_sending_date) + sep +
                   df.format(last_resending_time) + sep +
                   data.getMessageId() + sep +
                   data.getSequenceNumber() + sep +
                   data.getSourceAddress().getAddress() + sep +
                   data.getDestinationAddress().getAddress()+ sep +
                   data.getConnectionName() + sep +
                   sdf.format(data.getSubmitDate()) + sep +
                   sdf.format(data.getDoneDate()) + sep +
                   data.getFinalMessageState() + sep +
                   data.getNsms() + sep +
                   data.getStatus();

        log.debug("Try to write to journal string: "+s);

        int byteCount;
        try {
            byte[] bytes = (s+"\n").getBytes("UTF-8");
            byteCount = bytes.length;
        } catch (UnsupportedEncodingException e) {
            log.error(e);
            throw new CouldNotWriteToJournalException(e);
        }

        try {
            if (j1.createNewFile()){
                log.debug("Create journal "+j1.getName());
            }
        } catch (IOException e) {
            log.error(e);
            throw new CouldNotWriteToJournalException(e);
        }

        long sum;
        if (!j2.exists()){
            sum = j1.length();
        } else {
            sum = j1.length() + j2.length();
        }

        log.debug("Length of the journal in bytes: "+sum);

        if (sum+byteCount <= max_journal_size_mb*1024*1024){
            log.debug("Length of the journal after appending string will be less or equal than "+ max_journal_size_mb +" mb.");

            try {
                BufferedWriter out = new BufferedWriter(new FileWriter(j1, true));
                out.write(s+"\n");
                out.close();
                log.debug("Successfully write to the journal.");
            } catch (IOException e) {
                log.error("Could not append a string to the file "+ j1.getName(), e);
                throw new CouldNotWriteToJournalException(e);
            }

        } else {
            log.error("Size of the journal after appending string will be more than maximum allowed juornal size "+ max_journal_size_mb +" mb.");
            throw new CouldNotWriteToJournalException("Size of the journal after appending string will be more than maximum allowed juornal size "+ max_journal_size_mb +" mb.");
        }

    }

    public void clean() throws CouldNotCleanJournalException {
        log.debug("Try to clean journal ... ");

        if (j1.exists()){

            j2 = new File(dir, "journal_1.csv");

            try {
                if (j2.createNewFile()){
                    log.debug("Create journal "+ j2.getName()+".");
                } else {
                    log.warn("Journal "+ j2.getName()+" already exists.");
                }
            } catch (IOException e) {
                log.debug("Couldn't create cleaned file '"+ j2.getName()+"'.");
                throw new CouldNotCleanJournalException(e);
            }

            try {
                appendFile(j1, j2);
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotCleanJournalException("Couldn't append file '"+ j1.getName()+"' to another file '"+ j2.getName()+"'.");
            }

            if (j1.delete()){
                log.debug("Delete journal "+ j1.getName()+" .");
            } else {
                log.error("Couldn't delete journal "+ j1.getName()+" .");
                throw new CouldNotCleanJournalException("Couldn't delete journal "+ j1.getName()+".");
            }

            File temp_journal = new File(dir, j2.getName()+".tmp");
            try {
                if (temp_journal.createNewFile()){
                    log.debug("Successfully create temporary cleaned journal file "+temp_journal.getName()+"'.");
                } else {
                    log.warn("Couldn't create temporary cleaned journal file "+temp_journal.getName()+" because it already exists.");
                }
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotCleanJournalException("Couldn't create temporary cleaned journal file.", e);
            }

            PrintWriter pw;
            try {
                pw = new PrintWriter(new FileWriter(temp_journal));
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotCleanJournalException(e);
            }

            Set<Long> message_ids = new HashSet<Long>();

            BufferedReader buffReader;

            try{
                buffReader = new BufferedReader (new FileReader(j2));
                String line;
                while((line = buffReader.readLine()) != null){
                    String[] ar = line.split(sep);
                    long message_id = Long.parseLong(ar[2].trim());
                    String status = ar[11].trim();

                    if (status.equals(Status.DONE.toString())) {
                        if (message_ids.add(message_id)){
                            log.debug(message_id+"_message has DONE status, remember it.");
                        } else {
                            log.warn("Couldn't remember message with 'DONE' status because it's already added to the map. ");
                        }
                    }
                }
                buffReader.close();
            } catch (IOException ioe){
                log.error(ioe);
                throw new CouldNotCleanJournalException(ioe);
            }

            int counter = 0;
            try{
                buffReader = new BufferedReader (new FileReader(j2));
                String line;
                while((line = buffReader.readLine()) != null){
                    log.debug("line: "+line);
                    String[] ar = line.split(sep);

                    long message_id = Long.parseLong(ar[2].trim());
                    String status = ar[11].trim();
                    log.debug(message_id+"_message has "+status+" status, so try to write it to the temporary journal file "+temp_journal.getName());

                    if (!message_ids.contains(message_id)){
                        pw.println(line);
                        pw.flush();
                        counter++;
                    }

                }
                buffReader.close();
            } catch (IOException ioe){
                log.error(ioe);
                throw new CouldNotCleanJournalException(ioe);
            } finally {
                pw.close();
            }

            if (j2.delete()){
                log.debug("Delete journal "+ j2.getName()+" .");
            } else {
                log.error("Couldn't delete journal "+ j2.getName()+" .");
                throw new CouldNotCleanJournalException("Couldn't delete journal "+ j2.getName()+" .");
            }

            if (counter>0){
                log.debug("Detected that temporary file isn't empty.");
                if (temp_journal.renameTo(j2)){
                    log.debug("Rename temporary journal "+temp_journal.getName()+" to journal "+ j2.getName()+" .");
                } else {
                    log.error("Couldn't rename temporary journal "+temp_journal.getName()+" to journal "+ j2.getName()+".");
                    throw new CouldNotCleanJournalException("Couldn't rename temporary journal "+temp_journal.getName()+" to journal "+ j2.getName()+".");
                }
            } else {
                log.debug("Detected that cleaned file is empty.");
                if (temp_journal.delete()){
                    log.debug("Successfully delete temporary file '"+temp_journal.getName());
                } else {
                    log.debug("Couldn't delete temporary file '"+temp_journal.getName());
                }
            }

        } else {
            log.debug("There is not file "+ j1.getName()+", so nothing to clean.");
        }

        log.debug("Successfully clean journal.");
    }

    public Hashtable<Long, Data> load() throws CouldNotLoadJournalException {
        log.debug("Try to load journal to the memory ...");

        Hashtable<Long, Data> table = new Hashtable<Long, Data>();

        if (j2t.exists()){
            log.debug("Detected that file '"+j2t.getName()+"' exist.");
            if (j2.exists()){
                log.debug("Detected that file '"+j2.getName()+"' exist.");
                if (j2t.delete()){
                    log.debug("Successfully delete file '"+j2t+"'.");
                } else {
                    log.debug("Couldn't delete file '"+j2t+"'.");
                    throw new CouldNotLoadJournalException("Couldn't delete file '"+j2t+"'.");
                }
            } else {
                log.debug("Detected that file '"+j2.getName()+"' doesn't exist.");
                if (j2t.renameTo(j2)){
                    log.debug("Successfully rename file '"+j2t.getName()+"' to the file '"+j2.getName()+"'.");
                } else {
                    log.debug("Couldn't rename file '"+j2t.getName()+"' to the file '"+j2.getName()+"'.");
                    throw new CouldNotLoadJournalException("Couldn't rename file '"+j2t.getName()+"' to the file '"+j2.getName()+"'.");
                }
            }
        }

        File[] journals = {j2, j1};

        for(File f: journals){
            if (f.exists()){
                log.debug("Read file "+f.getName());
                try {
                    Scanner scanner = new Scanner(f);
                    scanner.useDelimiter(sep);
                    while (scanner.hasNextLine()){
                        String line = scanner.nextLine();
                        if (!line.isEmpty()){
                            String[] ar = line.split(sep);

                            long first_sending_time;
                            try {
                                Date date = df.parse(ar[0]);
                                cal.setTime(date);
                                first_sending_time = cal.getTimeInMillis();
                            } catch (ParseException e) {
                                log.error(e);
                                throw new CouldNotLoadJournalException(e);
                            }

                            long last_resending_time;
                            try {
                                Date date = df.parse(ar[1]);
                                cal.setTime(date);
                                last_resending_time = cal.getTimeInMillis();
                            } catch (ParseException e){
                                log.error(e);
                                throw new CouldNotLoadJournalException(e);
                            }

                            long message_id = Long.parseLong(ar[2]);
                            int sequence_number = Integer.parseInt(ar[3]);

                            Address source_address, destination_address;
                            try {
                                source_address = new Address(ar[4]);
                                destination_address = new Address(ar[5]);
                            } catch (InvalidAddressFormatException e) {
                                throw new CouldNotLoadJournalException(e);
                            }

                            String connection_name = ar[6];
                            Date submit_date, done_date;
                            try {
                                submit_date = sdf.parse(ar[7]);
                                done_date = sdf.parse(ar[8]);
                            } catch (ParseException e) {
                                 throw new CouldNotLoadJournalException(e);
                            }

                            FinalMessageState finalMessageState =  FinalMessageState.valueOf(ar[9]);

                            int nsms = Integer.parseInt(ar[10]);

                            String status_str = ar[11];

                            if (status_str.equals(Status.DONE.toString())){
                                message_id_receipt_table.remove(message_id);
                                log.debug("Remove from delivery receipt data with message id "+sequence_number+" .");
                            } else {
                                Data data = new Data();
                                data.setMessageId(message_id);
                                data.setSourceAddress(source_address);
                                data.setDestinationAddress(destination_address);
                                data.setFirstSendingTime(first_sending_time);
                                data.setLastResendTime(last_resending_time);
                                data.setSubmitDate(submit_date);
                                data.setDoneDate(done_date);
                                data.setNsms(nsms);
                                data.setFinalMessageState(finalMessageState);
                                data.setConnectionName(connection_name);
                                message_id_receipt_table.put(message_id, data);
                                log.debug("Write in memory delivery receipt data with system id "+message_id+" --> "+ data.toString());

                                sequence_number_message_id_table.put(sequence_number, message_id);
                                log.debug("Put sn --> message_id: "+sequence_number+" --> "+message_id);
                            }
                        }
                    }
                } catch (FileNotFoundException e) {
                    log.error(e);
                    throw new CouldNotLoadJournalException(e);
                }
            }
        }

        log.debug("Successfully load journal in memory.");
        return table;
    }

}
