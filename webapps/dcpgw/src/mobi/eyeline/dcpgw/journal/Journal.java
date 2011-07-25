package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.exeptions.CouldNotCleanJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.DateFormat;
import java.text.ParseException;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.05.11
 * Time: 16:58
 */
public class Journal {

    private static Logger log = Logger.getLogger(Journal.class);

    private static final String sep=",";

    private int max_journal_size_mb = 10;

    private File journal_dir, current_file, cleaned_file;

    private DateFormat df;
    private Calendar cal;

    public Journal(){
        String userDir = System.getProperty("user.dir");
        String filename = userDir+"/conf/dcpgw.properties";

        Properties prop = new Properties();

        try{
            prop.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.error(e);
            System.exit(1);
        }

        String s = prop.getProperty("max.journal.size.mb");
        if (s != null && !s.isEmpty()){
            max_journal_size_mb = Integer.parseInt(s);
            log.debug("Set: max_journal_size_mb="+ max_journal_size_mb);
        } else {
            log.warn("Couldn't find property max_journal_size_mb, set default value " + max_journal_size_mb);
        }

        s = prop.getProperty("clean.journal.timeout.msl");
        long clean_journal_timeout = 60000;
        if (s != null && !s.isEmpty()){
            clean_journal_timeout = Long.parseLong(s);
            log.debug("Set: clean_journal_time="+ clean_journal_timeout);
        } else {
            log.warn("Couldn't find clean journal timeout, set default value to "+ clean_journal_timeout);
        }

        journal_dir = new File(userDir+File.separator+"journal");
        if (!journal_dir.exists()){
            log.debug("Detected that journal directory doesn't exist.");
            if (journal_dir.mkdir()){
                log.debug("Successfully create journal directory.");
            } else {
                log.error("Couldn't create journal directory, check permissions.");
                System.exit(1);
            }
        } else {
            log.debug("Detected that journal directory already exists.");
        }

        df = DateFormat.getDateTimeInstance();
        cal = Calendar.getInstance();

        try {
            loadJournal();
        } catch (CouldNotLoadJournalException e) {
            log.error(e);
            System.exit(1);
        }

        ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                try {
                    cleanJournal();
                } catch (CouldNotCleanJournalException e) {
                    log.error(e);
                    // todo ?
                }
            }

        }, clean_journal_timeout, clean_journal_timeout, TimeUnit.MILLISECONDS);

    }

    private void appendFile(File source, File target) throws IOException {

        BufferedOutputStream bufOut = new BufferedOutputStream(new FileOutputStream(target, true));

        BufferedInputStream bufRead = new BufferedInputStream(new FileInputStream(source));

        int n;

        while((n = bufRead.read()) != -1) {
            bufOut.write(n);
        }

        bufOut.flush();

        bufOut.close();

        bufRead.close();

    }

    public void write(int sequence_number, Data data, Status status) throws CouldNotWriteToJournalException {
        cal.setTimeInMillis(data.getFirstSendingTime());
        Date first_sending_date = cal.getTime();
        cal.setTimeInMillis(data.getLastResendTime());
        Date last_resending_time = cal.getTime();
        String s = df.format(first_sending_date) + sep + df.format(last_resending_time) + sep + data.getMessageId() +
                   sep + sequence_number + sep + status;

        log.debug("Try to write to journal string: "+s);

        int byteCount;
        try {
            byte[] bytes = (s+"\n").getBytes("UTF-8");
            byteCount = bytes.length;
            //log.debug("Byte count "+byteCount);
        } catch (UnsupportedEncodingException e) {
            log.error(e);
            throw new CouldNotWriteToJournalException(e);
        }

        String file_name = "journal_0.csv";
        current_file = new File(journal_dir, file_name);
        if (!current_file.exists()){
            log.debug("Detected that file 'journal_0.csv' doesn't exist.");
            try {
                if (current_file.createNewFile()){
                    log.debug("Successfully create file "+file_name);
                } else {
                    log.debug("Couldn't create file "+file_name);
                }
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotWriteToJournalException(e);
            }
        }

        cleaned_file = new File(journal_dir, "journal_1.csv");

        long sum;
        if (!cleaned_file.exists()){
            sum = current_file.length();
        } else {
            sum = current_file.length() + cleaned_file.length();
        }

        log.debug("Length of the journal in bytes: "+sum);

        if (sum+byteCount <= max_journal_size_mb*1024/**1024*/){
            log.debug("Length of the journal after appending string will be less or equal than "+ max_journal_size_mb +" mb.");

            try {
                BufferedWriter out = new BufferedWriter(new FileWriter(current_file, true));
                out.write(s+"\n");
                out.close();
                log.debug("Successfully write to the journal.");
            } catch (IOException e) {
                log.error("Could not append a string to the file "+current_file.getName(), e);
                throw new CouldNotWriteToJournalException(e);
            }

        } else {
            log.error("Size of the journal after appending string will be more than maximum allowed juornal size "+ max_journal_size_mb +" mb.");
            throw new CouldNotWriteToJournalException("Size of the journal after appending string will be more than maximum allowed juornal size "+ max_journal_size_mb +" mb.");
        }

    }

    public void cleanJournal() throws CouldNotCleanJournalException {
        log.debug("Try to clean journal ... ");

        if (current_file.exists()){

            cleaned_file = new File(journal_dir, "journal_1.csv");
            if (!cleaned_file.exists()){

                log.debug("Detected that cleaned journal file "+cleaned_file.getName()+" doesn't exist.");
                try {
                    if (cleaned_file.createNewFile()){
                        log.debug("Successfully create cleaned journal file "+cleaned_file.getName());
                    } else {
                        log.warn("Couldn't create cleaned journal file "+cleaned_file.getName()+" because it already exists.");
                    }
                } catch (IOException e) {
                    log.debug("Couldn't create cleaned file '"+cleaned_file.getName()+"'.");
                    throw new CouldNotCleanJournalException(e);
                }

            }

            try {
                appendFile(current_file, cleaned_file);
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotCleanJournalException("Couldn't append file '"+current_file.getName()+"' to another file '"+cleaned_file.getName()+"'.");
            }

            if (current_file.delete()){
                log.debug("Successfully delete current file "+current_file.getName()+"'.");
            } else {
                log.error("Couldn't delete current journal file "+current_file.getName()+"'.");
                throw new CouldNotCleanJournalException("Couldn't delete current journal file "+current_file.getName()+"'.");
            }

            File temp_journal = new File(journal_dir, cleaned_file.getName()+".tmp");
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
                buffReader = new BufferedReader (new FileReader(cleaned_file));
                String line;
                while((line = buffReader.readLine()) != null){
                    String[] ar = line.split(sep);
                    long message_id = Long.parseLong(ar[2].trim());
                    String status = ar[4].trim();

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
                buffReader = new BufferedReader (new FileReader(cleaned_file));
                String line;
                while((line = buffReader.readLine()) != null){
                    log.debug("line: "+line);
                    String[] ar = line.split(sep);

                    long message_id = Long.parseLong(ar[2].trim());
                    String status = ar[4].trim();
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

            if (cleaned_file.delete()){
                log.debug("Successfully delete cleaned file "+cleaned_file.getName()+"'.");
            } else {
                log.error("Couldn't delete cleaned journal file "+cleaned_file.getName()+"'.");
                throw new CouldNotCleanJournalException("Couldn't delete cleaned journal file "+cleaned_file.getName()+"'.");
            }

            if (counter>0){
                log.debug("Detected that temporary file isn't empty.");
                if (temp_journal.renameTo(cleaned_file)){
                    log.debug("Successfully rename temporary cleaned journal file '"+temp_journal.getName()+"' to file '"+cleaned_file.getName()+"'.");
                } else {
                    log.error("Couldn't rename temporary cleaned journal file '"+temp_journal.getName()+"' to file '"+cleaned_file.getName()+"'.");
                    throw new CouldNotCleanJournalException("Couldn't rename temporary cleaned journal file '"+temp_journal.getName()+"' to file '"+cleaned_file.getName()+"'.");
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
            log.debug("There is not file "+current_file.getName()+", so nothing to clean.");
        }

        log.debug("Successfully clean journal.");
    }

    public Hashtable<Integer, Data> loadJournal() throws CouldNotLoadJournalException {
        log.debug("Try to load journal to the memory ...");

        Hashtable<Integer, Data> table = new Hashtable<Integer, Data>();

        File j0 = new File(journal_dir, "journal_0.csv");
        File j1 = new File(journal_dir, "journal_1.csv");

        File j1_tmp = new File(journal_dir, "journal_1.csv.tmp");
        if (j1_tmp.exists()){
            log.debug("Detected that file '"+j1_tmp.getName()+"' exist.");
            if (j1.exists()){
                log.debug("Detected that file '"+j1.getName()+"' exist.");
                if (j1_tmp.delete()){
                    log.debug("Successfully delete file '"+j1_tmp+"'.");
                } else {
                    log.debug("Couldn't delete file '"+j1_tmp+"'.");
                    throw new CouldNotLoadJournalException("Couldn't delete file '"+j1_tmp+"'.");
                }
            } else {
                log.debug("Detected that file '"+j1.getName()+"' doesn't exist.");
                if (j1_tmp.renameTo(j1)){
                    log.debug("Successfully rename file '"+j1_tmp.getName()+"' to the file '"+j1.getName()+"'.");
                } else {
                    log.debug("Couldn't rename file '"+j1_tmp.getName()+"' to the file '"+j1.getName()+"'.");
                    throw new CouldNotLoadJournalException("Couldn't rename file '"+j1_tmp.getName()+"' to the file '"+j1.getName()+"'.");
                }
            }
        }



        File[] journals = {j1, j0};

        for(File f: journals){
            if (f.exists()){
                log.debug("Read file "+f.getName());
                try {
                    Scanner scanner = new Scanner(f);
                    scanner.useDelimiter(sep);
                    while (scanner.hasNextLine()){
                        String s = scanner.next();
                        long first_sending_time;
                        try {
                            Date date = df.parse(s);
                            cal.setTime(date);
                            first_sending_time = cal.getTimeInMillis();
                        } catch (ParseException e) {
                            log.error(e);
                            throw new CouldNotLoadJournalException(e);
                        }

                        s = scanner.next();
                        long last_resending_time;
                        try {
                            Date date = df.parse(s);
                            cal.setTime(date);
                            last_resending_time = cal.getTimeInMillis();
                        } catch (ParseException e){
                            log.error(e);
                            throw new CouldNotLoadJournalException(e);
                        }

                        long message_id = scanner.nextLong();
                        int sequence_number = scanner.nextInt();
                        String status_str = scanner.next();

                        if (status_str.equals(Status.DONE.toString())){
                            table.remove(sequence_number);
                            log.debug("Remove from memory: "+sequence_number);
                        } else {
                            Data data = new Data(message_id, first_sending_time, last_resending_time);
                            table.put(sequence_number, data);
                            log.debug("Write in memory: "+sequence_number+" --> "+ data.toString());
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
