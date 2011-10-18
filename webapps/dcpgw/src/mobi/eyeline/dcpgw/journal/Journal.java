package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.05.11
 * Time: 16:58
 */
public class Journal {

    private static Journal instance = new Journal();

    public static Journal getInstance(){
        return instance;
    }

    private static Logger log = Logger.getLogger(Journal.class);

    private static final String sep=";";

    private int max_journal_size_mb;
    private int max_submit_date_journal_size_mb;


    private File j1, j2, j2t;
    private BufferedWriter bw, sdbw;

    private final Object monitor = new Object();
    private final Object monitor2 = new Object();

    private Hashtable<String, Hashtable<Integer, Data>> connection_sn_data_store;
    private Hashtable<String, LinkedBlockingQueue<Data>> connection_data_queue_table;
    private Hashtable<String, Hashtable<Long, Date>> connection_message_id_date_store;

    private ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    // Sumbit date journal
    private File sdj1;
    private File sdj2;
    private File sdj2t;

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");

    public void init(File journal_dir, int max_journal_size_mb, int max_submit_date_journal_size_mb, int clean_timeout) throws InitializationException{
        log.debug("Try to initialize journal ...");
        this.max_journal_size_mb = max_journal_size_mb;
        this.max_submit_date_journal_size_mb = max_submit_date_journal_size_mb;

        if (!journal_dir.exists()) {
            if (!journal_dir.mkdir()) throw new InitializationException("Couldn't create journal directory.");
        }

        j1 = new File(journal_dir, "j1.csv");
        j2 = new File(journal_dir, "j2.csv");
        j2t = new File(journal_dir, "j2.csv.tmp");

        File sddir = new File(journal_dir.getPath() + File.separator + "sd");
        sdj1 = new File(journal_dir.getPath()+File.separator+"sd"+File.separator+"sdj1.csv");
        sdj2 = new File(journal_dir.getPath()+File.separator+"sd"+File.separator+"sdj2.csv");
        sdj2t = new File(journal_dir.getPath()+File.separator+"sd"+File.separator+"sdj2t.csv");

        if (!journal_dir.exists()){
            log.debug("Detected that journal directory doesn't exist.");
            if (journal_dir.mkdir()){
                log.debug("Successfully create journal directory.");
            } else {
                log.error("Couldn't create journal directory, check permissions.");
                throw new InitializationException("Couldn't create journal directory, check permissions.");
            }
        } else {
            log.debug("Detected that journal directory already exists.");
        }

        if (!sddir.exists()){
            log.debug("Detected that journal directory doesn't exist.");
            if (sddir.mkdir()){
                log.debug("Successfully create submit date journal directory.");
            } else {
                log.error("Couldn't create journal directory, check permissions.");
                throw new InitializationException("Couldn't create journal directory, check permissions.");
            }
        } else {
            log.debug("Detected that journal directory already exists.");
        }


        scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                clean();
            }

        }, clean_timeout, clean_timeout, TimeUnit.MILLISECONDS);


        connection_sn_data_store = new Hashtable<String, Hashtable<Integer, Data>>();
        connection_data_queue_table = new Hashtable<String, LinkedBlockingQueue<Data>>();
        connection_message_id_date_store = new Hashtable<String, Hashtable<Long, Date>>();

        log.debug("Initialize journal.");
    }

    public void load() throws CouldNotLoadJournalException {
        log.debug("Try to load journal ...");

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
                        if (!line.isEmpty() && !line.startsWith("#")){

                            Data data;
                            try {
                                data = Data.parse(line);
                            } catch (ParseException e) {
                                throw new CouldNotLoadJournalException(e);
                            } catch (InvalidAddressFormatException e) {
                                throw new CouldNotLoadJournalException(e);
                            }

                            Data.Status status = data.getStatus();

                            String connection = data.getConnectionName();
                            if (status == Data.Status.INIT){

                                if (!connection_data_queue_table.containsKey(connection))
                                    connection_data_queue_table.put(connection, new LinkedBlockingQueue<Data>());


                                try {
                                    connection_data_queue_table.get(connection).put(data);
                                } catch (InterruptedException e) {
                                    throw new CouldNotLoadJournalException(e);
                                }
                            } else {

                                Integer sequence_number = data.getSequenceNumber();

                                if (!connection_sn_data_store.containsKey(connection))
                                        connection_sn_data_store.put(connection, new Hashtable<Integer, Data>());

                                if (status == Data.Status.DONE
                                        || status == Data.Status.EXPIRED_MAX_TIMEOUT
                                            ||  status == Data.Status.EXPIRED_TIMEOUT
                                                || status == Data.Status.DELETED){

                                    connection_sn_data_store.get(connection).remove(sequence_number);
                                    log.debug("remove from memory data: con="+connection+", sn="+sequence_number);

                                } else {

                                    connection_sn_data_store.get(connection).put(sequence_number, data);

                                    log.debug("add to memory: " + data.toString());
                                }
                            }
                        }
                    }
                    scanner.close();
                } catch (FileNotFoundException e) {
                    log.error(e);
                    throw new CouldNotLoadJournalException(e);
                }
            }
        }

        // Load submit date journal
        if (sdj2t.exists()){
            log.debug("Detected that file '"+sdj2t.getName()+"' exist.");

            if (sdj2.exists()){
                log.debug("Detected that file '"+sdj2.getName()+"' exist.");

                if (sdj2t.delete()){
                    log.debug("Successfully delete file '"+sdj2t+"'.");
                } else {
                    log.debug("Couldn't delete file '"+sdj2t+"'.");
                    throw new CouldNotLoadJournalException("Couldn't delete file '"+sdj2t+"'.");
                }
            } else {
                log.debug("Detected that file '"+sdj2.getName()+"' doesn't exist.");

                if (sdj2t.renameTo(sdj2)){
                    log.debug("Successfully rename file '"+sdj2t.getName()+"' to the file '"+sdj2.getName()+"'.");
                } else {
                    log.debug("Couldn't rename file '"+sdj2t.getName()+"' to the file '"+sdj2.getName()+"'.");
                    throw new CouldNotLoadJournalException("Couldn't rename file '"+sdj2t.getName()+"' to the file '"+sdj2.getName()+"'.");
                }
            }
        }

        File[] sd_journals = {sdj2, sdj1};

        for(File f: sd_journals){
            if (f.exists()){
                log.debug("Read file "+f.getName());
                try {
                    Scanner scanner = new Scanner(f);
                    scanner.useDelimiter(sep);
                    while (scanner.hasNextLine()){
                        String line = scanner.nextLine();
                        if (!line.isEmpty() && !line.startsWith("#")){

                            String ar[] = line.split(sep);
                            if (ar.length == 4) {
                                long message_id = Long.parseLong(ar[0].trim());
                                String connection_name = ar[1].trim();
                                Date date;
                                try {
                                    date = sdf.parse(ar[2].trim());
                                } catch (ParseException e) {
                                    throw new CouldNotLoadJournalException(e);
                                }
                                boolean receives_receipt = Boolean.parseBoolean(ar[3].trim());

                                if (!connection_message_id_date_store.containsKey(connection_name)){
                                     connection_message_id_date_store.put(connection_name, new Hashtable<Long, Date>());
                                }

                                Hashtable<Long, Date> message_id_date = connection_message_id_date_store.get(connection_name);

                                if (!receives_receipt){
                                    message_id_date.put(message_id, date);
                                } else {
                                    message_id_date.remove(message_id);
                                }
                            }

                        }
                    }
                    scanner.close();
                } catch (FileNotFoundException e) {
                    log.error(e);
                    throw new CouldNotLoadJournalException(e);
                }
            }
        }

        log.debug("Successfully load journal.");
    }

    public void write(Data data) throws CouldNotWriteToJournalException {

        synchronized (monitor){

            try {
                if (j1.createNewFile()){
                    log.debug("Create journal "+j1.getName());
                }
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotWriteToJournalException(e);
            }

            String s = Data.format(data);

            boolean isEnoughSpace;
            try {
                isEnoughSpace = isJournalHasEnoughSpace(s);
            } catch (UnsupportedEncodingException e) {
                throw new CouldNotWriteToJournalException(e);
            }

            if (isEnoughSpace){
                //log.debug("Length of the journal after appending string will be less or equal than "+ max_journal_size_mb +" mb.");

                try {
                    if (bw == null) {
                        bw = new BufferedWriter(new FileWriter(j1, true));
                        log.debug("Initialize buffered writer for journal "+j1.getName());
                    }
                    bw.write(s+"\n");
                    bw.flush();
                    log.debug("write: "+s);
                } catch (IOException e) {
                    log.error("Could not append a string to the file "+ j1.getName(), e);
                    throw new CouldNotWriteToJournalException(e);
                }

            } else {
                log.error("Size of the journal after appending string will be more than maximum allowed journal size "+ max_journal_size_mb +" mb.");
                throw new CouldNotWriteToJournalException("Size of the journal after appending string will be more than maximum allowed juornal size "+ max_journal_size_mb +" mb.");
            }

        }

    }

    private boolean isJournalHasEnoughSpace(String line) throws UnsupportedEncodingException {
        //log.debug("Try to write to journal string: "+line);

        int byteCount;
        byte[] bytes = (line + "\n").getBytes("UTF-8");
        byteCount = bytes.length;

        long sum;
        if (!j2.exists()){
            sum = j1.length();
        } else {
            sum = j1.length() + j2.length();
        }

        //log.debug("Length of the journal in bytes: "+sum);

        return sum+byteCount <= max_journal_size_mb*1024*1024;
    }

    private boolean isSubmitDateJournalHasEnoughSpace(String line) throws UnsupportedEncodingException {
        //log.debug("Try to write to journal string: "+line);

        int byteCount;
        byte[] bytes = (line + "\n").getBytes("UTF-8");
        byteCount = bytes.length;

        long sum;
        if (!sdj2.exists()){
            sum = sdj1.length();
        } else {
            sum = sdj1.length() + sdj2.length();
        }

        //log.debug("Length of the journal in bytes: "+sum);

        return sum+byteCount <= max_submit_date_journal_size_mb*1024*1024;
    }

    public void clean(){
        log.debug("Try to clean journal ... ");

        long t = System.currentTimeMillis();

        if (j1.exists() && j1.length() > 0){
            BufferedReader buffReader1 = null;
            BufferedReader buffReader2 = null;
            PrintWriter pw = null;
            try {
                if (j2.createNewFile()) log.debug("Create file "+j2.getName());

                synchronized (monitor){
                    log.debug("Try to append file '"+j1.getName()+"' to file '"+j2.getName()+"'.");

                    bw.close();
                    log.debug("Close buffered writer for journal "+j1.getName());

                    FileOutputStream fos = new FileOutputStream(j2, true);
                    BufferedOutputStream bufOut = new BufferedOutputStream(fos);

                    FileInputStream fis = new FileInputStream(j1);
                    BufferedInputStream bufRead = new BufferedInputStream(fis);

                    int n;

                    while((n = bufRead.read()) != -1) {
                        bufOut.write(n);
                    }

                    bufOut.flush();

                    bufOut.close();

                    bufRead.close();

                    boolean b = j1.delete();
                    log.debug("Delete file "+j1.getName()+": "+b);

                    b = j1.createNewFile();
                    log.debug("Create file "+j1.getName()+": "+b);

                    bw = new BufferedWriter(new FileWriter(j1, true));
                    log.debug("Initialize buffered writer for journal "+j1.getName());

                    log.debug("Successfully append journal.");
                }

                if (j2t.createNewFile()) log.debug("Create file "+j2t.getName());

                pw = new PrintWriter(new FileWriter(j2t));


                Set<Long> done_message_ids = new HashSet<Long>();
                Set<Long> perm_errors_message_ids = new HashSet<Long>();
                Set<Long> expired_max_messages_ids = new HashSet<Long>();
                Set<Integer> sequence_numbers = new HashSet<Integer>();
                Set<Long> processed_message_ids = new HashSet<Long>();
                Set<Long> deleted_messages_ids = new HashSet<Long>();


                buffReader1 = new BufferedReader (new FileReader(j2));

                String line;
                while((line = buffReader1.readLine()) != null){
                    String[] ar = line.split(sep);
                    long message_id = Long.parseLong(ar[3].trim());
                    Data.Status status = Data.Status.valueOf(ar[12].trim());

                    if (status == Data.Status.DONE || status == Data.Status.EXPIRED_MAX_TIMEOUT) {
                        done_message_ids.add(message_id);
                        //log.debug("finished: message_id="+message_id);
                    } else if (status == Data.Status.PERM_ERROR){
                        perm_errors_message_ids.add(message_id);
                        //log.debug("perm error: message_id="+message_id);
                    } else if (status == Data.Status.DELETED){
                        deleted_messages_ids.add(message_id);
                        //log.debug("deleted: message_id="+message_id);
                    } else if (status == Data.Status.EXPIRED_MAX_TIMEOUT){
                        expired_max_messages_ids.add(message_id);
                        //log.debug("expired_max: message_id="+message_id);
                    } else if (status == Data.Status.EXPIRED_TIMEOUT){
                        int sequence_number = Integer.parseInt(ar[4]);
                        sequence_numbers.add(sequence_number);
                        //log.debug("expired: sn="+sequence_number);
                    } else if (status == Data.Status.SEND || status == Data.Status.NOT_SEND){
                        processed_message_ids.add(message_id);
                        //log.debug("processed: message_id="+message_id);
                    }

                }
                buffReader1.close();
                log.debug("Read file "+j2);

                int counter = 0;
                buffReader2 = new BufferedReader(new FileReader(j2));

                while((line = buffReader2.readLine()) != null){
                    String[] ar = line.split(sep);

                    long message_id = Long.parseLong(ar[3].trim());
                    Data.Status status = Data.Status.valueOf(ar[12].trim());

                    if (status == Data.Status.INIT){
                        if (!processed_message_ids.contains(message_id) && !deleted_messages_ids.contains(message_id)){
                            //log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                            pw.println(line);
                            pw.flush();
                            counter++;
                        }

                    } else {
                        int sequence_number = Integer.parseInt(ar[4]);
                        if (!done_message_ids.contains(message_id)
                                && !sequence_numbers.contains(sequence_number)
                                    && !deleted_messages_ids.contains(message_id)
                                        && !expired_max_messages_ids.contains(message_id)
                                            && !perm_errors_message_ids.contains(message_id)){
                            //log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                            pw.println(line);
                            pw.flush();
                            counter++;
                        }
                    }

                }
                buffReader2.close();
                pw.close();
                log.debug("Write to file "+j2t);

                boolean b;
                if (counter>0){

                    b = j2.delete();
                    log.debug("Delete file "+j2.getName()+": "+b);
                    b = j2t.renameTo(j2);
                    log.debug("Rename file "+j2t.getName()+" to "+j2.getName()+": "+b);

                } else {

                    b = j2t.delete();
                    log.debug("Delete file "+j2t.getName()+": "+b);
                    b = j2.delete();
                    log.debug("Delete file "+j2.getName()+": "+b);
                    b = j2.createNewFile();
                    log.debug("Create file "+j2.getName()+": "+b);

                }

            } catch (IOException e) {
                log.error(e);
            } finally {
                if (pw != null) pw.close();
                try{
                    if (buffReader1 != null) buffReader1.close();
                    if (buffReader2 != null) buffReader2.close();
                } catch (IOException e2){
                    log.error(e2);
                }
            }

        }

        // Clean submit date journal

        if (sdj1.exists() && sdj1.length() > 0){
            BufferedReader buffReader1 = null;
            BufferedReader buffReader2 = null;
            PrintWriter pw = null;
            try {
                if (sdj2.createNewFile()) log.debug("Create file "+sdj2.getName());

                synchronized (monitor2){
                    log.debug("Try to append file '"+sdj1.getName()+"' to file '"+sdj2.getName()+"'.");

                    sdbw.close();
                    log.debug("Close buffered writer for journal "+sdj1.getName());

                    FileOutputStream fos = new FileOutputStream(sdj2, true);
                    BufferedOutputStream bufOut = new BufferedOutputStream(fos);

                    FileInputStream fis = new FileInputStream(sdj1);
                    BufferedInputStream bufRead = new BufferedInputStream(fis);

                    int n;

                    while((n = bufRead.read()) != -1) {
                        bufOut.write(n);
                    }

                    bufOut.flush();

                    bufOut.close();

                    bufRead.close();

                    boolean b = sdj1.delete();
                    log.debug("Delete file "+sdj1.getName()+": "+b);

                    b = sdj1.createNewFile();
                    log.debug("Create file "+sdj1.getName()+": "+b);

                    sdbw = new BufferedWriter(new FileWriter(sdj1, true));
                    log.debug("Initialize buffered writer for journal "+sdj1.getName());

                    log.debug("Successfully append submit date journal.");
                }

                if (sdj2t.createNewFile()) log.debug("Create file "+sdj2t.getName());

                pw = new PrintWriter(new FileWriter(sdj2t));

                Set<Long> receives_receipt_message_ids = new HashSet<Long>();

                buffReader1 = new BufferedReader (new FileReader(sdj2));

                String line;
                while((line = buffReader1.readLine()) != null){
                    String[] ar = line.split(sep);
                    long message_id = Long.parseLong(ar[0].trim());
                    boolean receives_receipts = Boolean.parseBoolean(ar[3].trim());
                    if (receives_receipts){
                        receives_receipt_message_ids.add(message_id);
                    }
                }
                buffReader1.close();
                log.debug("Read file "+sdj2);

                int counter = 0;
                buffReader2 = new BufferedReader(new FileReader(sdj2));

                while((line = buffReader2.readLine()) != null){
                    String[] ar = line.split(sep);

                    long message_id = Long.parseLong(ar[0].trim());
                    boolean receives_receipts = Boolean.parseBoolean(ar[3].trim());

                    if (!receives_receipts){
                        if (!receives_receipt_message_ids.contains(message_id)){
                            //log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                            pw.println(line);
                            pw.flush();
                            counter++;
                        }
                    }

                }
                buffReader2.close();
                pw.close();
                log.debug("Write to file "+sdj2t);

                boolean b;
                if (counter>0){

                    b = sdj2.delete();
                    log.debug("Delete file "+sdj2.getName()+": "+b);
                    b = sdj2t.renameTo(sdj2);
                    log.debug("Rename file "+sdj2t.getName()+" to "+sdj2.getName()+": "+b);

                } else {

                    b = sdj2t.delete();
                    log.debug("Delete file "+sdj2t.getName()+": "+b);
                    b = sdj2.delete();
                    log.debug("Delete file "+sdj2.getName()+": "+b);
                    b = sdj2.createNewFile();
                    log.debug("Create file "+sdj2.getName()+": "+b);

                }

            } catch (IOException e) {
                log.error(e);
            } finally {
                if (pw != null) pw.close();
                try{
                    if (buffReader1 != null) buffReader1.close();
                    if (buffReader2 != null) buffReader2.close();
                } catch (IOException e2){
                    log.error(e2);
                }
            }

        }

        long d = System.currentTimeMillis() - t;

        log.debug("Journal cleaned, "+d+" mls.");
    }

    public Hashtable<Integer, Data> getDataTable(String connection_name){
        return connection_sn_data_store.get(connection_name);
    }

    public LinkedBlockingQueue<Data> getDataQueue(String connection_name){
        return connection_data_queue_table.get(connection_name);
    }

    public Hashtable<Long, Date> getSubmitDateTable(String connection_name){
        return connection_message_id_date_store.get(connection_name);
    }

    public void shutdown(){
        scheduler.shutdown();
        log.debug("journal shutdown");
    }

    public void writeSubmitDate(long message_id, String connection_name, Date date, boolean receives_receipt)throws CouldNotWriteToJournalException {

        synchronized (monitor2){

            try {
                if (sdj1.createNewFile()){
                    log.debug("Create journal "+sdj1.getName());
                }
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotWriteToJournalException(e);
            }

            String s = message_id + sep + connection_name + sep + sdf.format(date) + sep + receives_receipt;

            boolean isEnoughSpace;
            try {
                isEnoughSpace = isSubmitDateJournalHasEnoughSpace(s);
            } catch (UnsupportedEncodingException e) {
                throw new CouldNotWriteToJournalException(e);
            }

            if (isEnoughSpace){
                //log.debug("Length of the journal after appending string will be less or equal than "+ max_journal_size_mb +" mb.");

                try {
                    if (sdbw == null) {
                        sdbw = new BufferedWriter(new FileWriter(sdj1, true));
                        log.debug("Initialize buffered writer for journal "+sdj1.getName());
                    }
                    sdbw.write(s+"\n");
                    sdbw.flush();
                    log.debug("write: "+s);
                } catch (IOException e) {
                    log.error("Could not append a string to the file "+ sdj1.getName(), e);
                    throw new CouldNotWriteToJournalException(e);
                }

            } else {
                log.error("Size of the journal after appending string will be more than maximum allowed journal size "+ max_journal_size_mb +" mb.");
                throw new CouldNotWriteToJournalException("Size of the journal after appending string will be more than maximum allowed juornal size "+ max_journal_size_mb +" mb.");
            }

        }
    }



}
