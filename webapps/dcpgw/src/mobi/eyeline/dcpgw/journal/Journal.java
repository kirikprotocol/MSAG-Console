package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.exeptions.CouldNotCleanJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotGetNextFileException;
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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.05.11
 * Time: 16:58
 */
public class Journal {

    private static Logger log = Logger.getLogger(Journal.class);

    private static final String sep=",";

    private int max_number_of_files = 10;
    private int max_file_size_mb = 10;

    private File current_file;
    private File journal_dir;

    private DateFormat df;
    private Calendar cal;

    private long clean_journal_timeout = 60000;

    private FileFilter csv_file_filter;

    public Journal(){
        String userDir = System.getProperty("user.dir");
        String filename = userDir+"/config/dcpgw.properties";

        Properties prop = new Properties();

        try{
            prop.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.error(e);
            System.exit(1);
        }

        String s = prop.getProperty("max.number.of.files");
        if (s != null && !s.isEmpty()){
            max_number_of_files = Integer.parseInt(s);
            log.debug("Set: max_number_of_files="+max_number_of_files);
        } else {
            log.warn("Couldn't find property max.number.of.files, set default value " + max_number_of_files);
        }

        s = prop.getProperty("max.file.size.mb");
        if (s != null && !s.isEmpty()){
            max_file_size_mb = Integer.parseInt(s);
            log.debug("Set: max_file_size_mb="+max_file_size_mb);
        } else {
            log.warn("Couldn't find property max_file_size_mb, set default value " + max_file_size_mb);
        }

        s = prop.getProperty("clean.journal.timeout.msl");
        if (s != null && !s.isEmpty()){
            clean_journal_timeout = Long.parseLong(s);
            log.debug("Set: clean_journal_time="+clean_journal_timeout);
        } else {
            log.warn("Couldn't find clean journal timeout, set default value to "+clean_journal_timeout);
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

        try {
            loadJournal();
        } catch (CouldNotLoadJournalException e) {
            log.error(e);
            System.exit(1);
        }

        String file_name = "journal_0.csv";
        current_file = new File(journal_dir, file_name);
        if (!current_file.exists()){
            log.debug("Detected that file journal_0.csv doesn't exist.");
            try {
                if (current_file.createNewFile()){
                    log.debug("Successfully create file "+file_name);
                } else {
                    log.debug("Couldn't create file "+file_name);
                }
            } catch (IOException e) {
                log.error(e);
                System.exit(1);
            }
        }

        df = DateFormat.getDateTimeInstance();
        cal = Calendar.getInstance();

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

        csv_file_filter = new FileFilter() {
            @Override
            public boolean accept(File file) {
                return file.isFile() && file.getName().toLowerCase().endsWith(".csv");
            }

        };

    }

    public void write(int sequence_number, Data data, Status status) throws CouldNotWriteToJournalException {
        cal.setTimeInMillis(data.getTime());
        Date date = cal.getTime();
        String s = df.format(date)+sep+ data.getMessageId()+sep+sequence_number+sep+status;

        log.debug("Try to write to journal string: "+s);

            int byteCount;
            try {
                byte[] bytes = (s+"\n").getBytes("UTF-8");
                byteCount = bytes.length;
                log.debug("Byte count "+byteCount);
            } catch (UnsupportedEncodingException e) {
                log.error(e);
                throw new CouldNotWriteToJournalException(e);
            }

            long file_length = current_file.length();
            log.debug("Length of the current file in bytes: "+file_length);

            if (file_length+byteCount <= max_file_size_mb*1024*1024){
                log.debug("Length of the current file after appending string will be less or equal than "+max_file_size_mb+" mb.");
            } else {
                log.debug("Length of the current file after appending string will be more than "+max_file_size_mb+" mb.");
                try {
                    current_file = getNextFile();
                } catch (CouldNotGetNextFileException e) {
                    throw new CouldNotWriteToJournalException("Couldn't write to journal. ",e);
                }
            }

            try {
                BufferedWriter out = new BufferedWriter(new FileWriter(current_file, true));
                out.write(s+"\n");
                out.close();
                log.debug("Successfully write to the journal");
            } catch (IOException e) {
                log.error("Could not append a string to the file "+current_file.getName(), e);
                throw new CouldNotWriteToJournalException(e);
            }



    }

    synchronized private File getNextFile() throws CouldNotGetNextFileException {

        File[] journals = journal_dir.listFiles(csv_file_filter);

        int number_of_journals = journals.length;

        if (number_of_journals <= max_number_of_files){
            log.debug("Number of journal files is "+number_of_journals+" and it's less than max number "+max_number_of_files+".");

            NavigableMap<Integer, File> files_map = new TreeMap<Integer, File>();
            for(File f:journals){
                String name = f.getName();
                Pattern p = Pattern.compile("journal_(\\d*)\\.csv");
                Matcher m = p.matcher(name);
                if (m.matches()){
                    int number_of_file = Integer.parseInt(m.group(1));
                    files_map.put(number_of_file, f);
                    log.debug(number_of_file + " --> " + name);
                } else {
                    log.warn("Found the file name does not match the pattern "+name+".");
                }
            }

            NavigableMap<Integer, File> descending_files_map = files_map.descendingMap();
            for (Map.Entry<Integer, File> entry : descending_files_map.entrySet()) {
                int number = entry.getKey();
                File file = entry.getValue();
                String old_name = file.getName();
                String new_name = "journal_" + (number + 1)+".csv";
                boolean renamed = file.renameTo(new File(journal_dir, new_name));
                if (renamed) {
                    log.debug("Successfully rename file " + old_name + " to file " + new_name);
                } else {
                    log.error("Couldn't rename file " + old_name + " to file " + new_name+".");
                    throw new CouldNotGetNextFileException("Couldn't rename file " + old_name + " to file " + new_name+".");
                }
            }

            String new_file_name = "journal_0.csv";
            File file = new File(journal_dir, new_file_name);
            try {
                if (file.createNewFile()){
                    log.debug("New file "+new_file_name+" was successfully created.");
                } else {
                    log.error("Couldn't create new file with name "+new_file_name+".");
                    throw new CouldNotGetNextFileException("Couldn't create new file with name "+new_file_name+".");
                }

                return file;
            } catch (IOException e) {
                log.debug("Couldn't create new file "+file.getName(), e);
                throw new CouldNotGetNextFileException("Couldn't create new file "+file.getName(), e);
            }

        } else {
            log.error("Number of journal files is " + number_of_journals + " and it's more than max number " + max_number_of_files + ".");
            throw new CouldNotGetNextFileException("Number of journal files is "+number_of_journals+" and it's more than max number "+max_number_of_files+".");
        }

    }

    synchronized public void cleanJournal() throws CouldNotCleanJournalException {

        log.debug("Try to clean journal ... ");

        File[] journals = journal_dir.listFiles(csv_file_filter);

        if (journals.length > 0){

            TreeMap<String, File> m = new TreeMap<String, File>();

            for(File f: journals) {
                String name = f.getName();
                if ( !name.equals("journal_0.csv") ) {
                    m.put(name, f);
                    log.debug("put file with name "+f.getName());
                }
            }

            NavigableMap<String, File> dm = m.descendingMap();

            Set<Long> message_ids = new HashSet<Long>();

            Vector<File> temp_journals = new Vector<File>();
            int temp_journal_number = 1;
            File temp_journal = new File(journal_dir,"journal_"+temp_journal_number+ ".csv.tmp");
            try {
                if (temp_journal.createNewFile()) log.debug("Create new temporary file: "+temp_journal.getName() );
            } catch (IOException e) {
                log.debug(e);
                throw new CouldNotCleanJournalException(e);
            }

            temp_journals.add(temp_journal);

            PrintWriter pw;
            try {
                pw = new PrintWriter(new FileWriter(temp_journal));
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotCleanJournalException(e);
            }

            for (Map.Entry<String, File> entry : dm.entrySet()) {
                File f = entry.getValue();
                log.debug("Cleaning journal file: " + f.getName());
                Scanner scanner;
                try {
                    scanner = new Scanner(f);
                } catch (FileNotFoundException e) {
                    log.error(e);
                    throw new CouldNotCleanJournalException(e);
                }

                scanner.useDelimiter(sep);
                while (scanner.hasNextLine()){
                    String line = scanner.nextLine();

                    String[] ar = line.split(sep);

                    long message_id = Long.parseLong(ar[1].trim());
                    String status = ar[3].trim();

                    if (status.equals(Status.DONE.toString())) {
                        message_ids.add(message_id);
                        log.debug(message_id+"_message has DONE status, remember it.");
                    } else {

                        if (!message_ids.contains(message_id)){

                            log.debug(message_id+"_message has "+status+" status, so try to write it to the temporary journal file "+temp_journal.getName());

                            int byte_count;
                            try {
                                byte[] bytes = (line+"\n").getBytes("UTF-8");
                                byte_count = bytes.length;
                            } catch (UnsupportedEncodingException e) {
                                log.error(e);
                                throw new CouldNotCleanJournalException(e);
                            }

                            long temp_file_length = temp_journal.length();

                            if (temp_file_length+byte_count > max_file_size_mb*1024*1024){

                                log.debug("Temporary file has byte length "+temp_file_length);

                                pw.close();

                                temp_journal_number++;

                                String next_temp_file_name = "journal_"+temp_journal_number+".csv.tmp";

                                log.debug("Try to create next temporary journal file "+next_temp_file_name+" ...");

                                temp_journal = new File(journal_dir, next_temp_file_name);

                                temp_journals.add(temp_journal);

                                try {
                                    pw = new PrintWriter(new FileWriter(temp_journal));
                                } catch (IOException e) {
                                    log.error(e);
                                    throw new CouldNotCleanJournalException(e);
                                }


                            }

                            pw.println(line);
                            pw.flush();

                        }

                    }

                }

                scanner.close();
            }

            pw.close();

            for(File f: journals){
                if (!f.delete()){
                    throw new CouldNotCleanJournalException("Could not delete old journal file.");
                }
            }

            for(File f: temp_journals){
                String name = f.getName();
                Pattern p2 = Pattern.compile("(journal_\\d*\\.csv)\\.tmp");
                Matcher m2 = p2.matcher(name);
                if (m2.matches()){
                    String new_name = m2.group(1);
                    File new_file = new File(journal_dir, new_name);
                    if (!f.renameTo(new_file)){
                        throw new CouldNotCleanJournalException("Could not rename temporary journal file.");
                    }
                }
            }
            log.debug("Successfully clean journal.");

        } else {
            log.debug("Did not found any of journal.");
        }
    }

    public Hashtable<Integer, Data> loadJournal() throws CouldNotLoadJournalException {
        log.debug("Try to load journal to the memory ...");

        Hashtable<Integer, Data> table = new Hashtable<Integer, Data>();

        File[] journals = journal_dir.listFiles(csv_file_filter);

        if (journals.length > 0){

            for(File f: journals){
                log.debug("Read file "+f.getName());
                try {
                    Scanner scanner = new Scanner(f);
                    scanner.useDelimiter(sep);
                    while (scanner.hasNextLine()){
                        String s = scanner.next();
                        long time;
                        try {
                            Date date = df.parse(s);
                            cal.setTime(date);
                            time = cal.getTimeInMillis();
                        } catch (ParseException e) {
                            log.error(e);
                            throw new CouldNotLoadJournalException(e);
                        }
                        long message_id = scanner.nextLong();
                        int sequence_number = scanner.nextInt();
                        String status = scanner.next();

                        if (Status.valueOf(status) == Status.DONE){
                            table.remove(sequence_number);
                            log.debug("Remove from memory: "+sequence_number);
                        } else {
                            Data data = new Data(message_id, time);
                            table.put(sequence_number, data);
                            log.debug("Write in memory: "+sequence_number+" --> "+ data.toString());
                        }

                    }

                } catch (FileNotFoundException e) {
                    log.error(e);
                    throw new CouldNotLoadJournalException(e);
                }
            }
            log.debug("Successfully load journal in memory.");
        } else {
            log.debug("Did not found any of journal.");
        }
        return table;
    }

}
