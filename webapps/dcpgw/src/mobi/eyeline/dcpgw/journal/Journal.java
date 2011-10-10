package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
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

    private int max_journal_size_mb = 10;

    private File j1;
    private File j2;
    private File j2t;

    private final Object monitor = new Object();

    private Hashtable<String, Hashtable<Integer, Data>> connection_sn_data_store;
    private Hashtable<String, LinkedBlockingQueue<Data>> connection_data_queue_table;

    private ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    private int queue_limit;
    private int request_limit;

    public void init(File journal_dir, int max_journal_size_mb, int clean_timeout, int queue_limit, int request_limit) throws InitializationException{
        this.max_journal_size_mb = max_journal_size_mb;
        this.queue_limit = queue_limit;
        this.request_limit = request_limit;

        if (!journal_dir.exists()) {
            if (!journal_dir.mkdir()) throw new InitializationException("Couldn't create journal directory.");
        }

        j1 = new File(journal_dir, "j1.csv");
        j2 = new File(journal_dir, "j2.csv");
        j2t = new File(journal_dir, "j2.csv.tmp");

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


        scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                clean();
            }

        }, clean_timeout, clean_timeout, TimeUnit.MILLISECONDS);


        connection_sn_data_store = new Hashtable<String, Hashtable<Integer, Data>>();
        connection_data_queue_table = new Hashtable<String, LinkedBlockingQueue<Data>>();
    }

    public void load() throws CouldNotLoadJournalException {
        log.debug("Try to load journal to the memory ...");

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
                                    connection_data_queue_table.put(connection, new LinkedBlockingQueue<Data>(queue_limit));


                                try {
                                    connection_data_queue_table.get(connection).put(data);
                                } catch (InterruptedException e) {
                                    throw new CouldNotLoadJournalException(e);
                                }
                            } else {

                                Integer sequence_number = data.getSequenceNumber();

                                if (!connection_sn_data_store.containsKey(connection))
                                        connection_sn_data_store.put(connection, new Hashtable<Integer, Data>(request_limit));

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

        log.debug("Successfully load journal in memory.");
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
                isEnoughSpace = isEnoughSpace(s);
            } catch (UnsupportedEncodingException e) {
                throw new CouldNotWriteToJournalException(e);
            }

            if (isEnoughSpace){
                log.debug("Length of the journal after appending string will be less or equal than "+ max_journal_size_mb +" mb.");

                try {
                    BufferedWriter bw = new BufferedWriter(new FileWriter(j1, true));
                    bw.write(s+"\n");
                    bw.flush();
                    bw.close();
                    log.debug("Successfully write to the journal.");
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

    private boolean isEnoughSpace(String line) throws UnsupportedEncodingException {
        log.debug("Try to write to journal string: "+line);

        int byteCount;
        byte[] bytes = (line + "\n").getBytes("UTF-8");
        byteCount = bytes.length;

        long sum;
        if (!j2.exists()){
            sum = j1.length();
        } else {
            sum = j1.length() + j2.length();
        }

        log.debug("Length of the journal in bytes: "+sum);

        return sum+byteCount <= max_journal_size_mb*1024*1024;
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

                appendFile(j1, j2);

                if (j2t.createNewFile()) log.debug("Create file "+j2t.getName());

                pw = new PrintWriter(new FileWriter(j2t));

                Set<Long> finished_message_ids = new HashSet<Long>();
                Set<Integer> sequence_numbers = new HashSet<Integer>();
                Set<Long> processed_message_ids = new HashSet<Long>();

                buffReader1 = new BufferedReader (new FileReader(j2));
                String line;
                while((line = buffReader1.readLine()) != null){
                    String[] ar = line.split(sep);
                    long message_id = Long.parseLong(ar[3].trim());
                    Data.Status status = Data.Status.valueOf(ar[12].trim());
                    int sequence_number = Integer.parseInt(ar[4]);

                    if (status == Data.Status.DONE || status == Data.Status.EXPIRED_MAX_TIMEOUT) {
                        finished_message_ids.add(message_id);
                    } else if (status == Data.Status.EXPIRED_TIMEOUT){
                        sequence_numbers.add(sequence_number);
                    } else if (status == Data.Status.SEND || status == Data.Status.NOT_SEND){
                        processed_message_ids.add(message_id);
                    }

                }
                buffReader1.close();

                buffReader2 = new BufferedReader (new FileReader(j2));
                while((line = buffReader2.readLine()) != null){
                    String[] ar = line.split(sep);

                    long message_id = Long.parseLong(ar[3].trim());
                    Data.Status status = Data.Status.valueOf(ar[12].trim());
                    int sequence_number = Integer.parseInt(ar[4]);

                    if (status == Data.Status.INIT){
                        if (!processed_message_ids.contains(message_id)){
                            log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                            pw.println(line);
                            pw.flush();
                        }
                    } else {
                        if (!finished_message_ids.contains(message_id) && !sequence_numbers.contains(sequence_number)){
                            log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                            pw.println(line);
                            pw.flush();
                        }
                    }

                }
                buffReader2.close();
                pw.close();

                if (j2.delete()) log.debug("Delete file "+j2.getName());
                if (j2t.renameTo(j2)) log.debug("Rename file "+j2t.getName()+" to "+j2.getName());

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

    private void appendFile(File source, File target) throws IOException {
        synchronized (monitor){
            log.debug("Try to append file '"+source.getName()+"' to file '"+target.getName()+"'.");

            FileOutputStream fos = new FileOutputStream(target, true);
            BufferedOutputStream bufOut = new BufferedOutputStream(fos);

            FileInputStream fis = new FileInputStream(source);
            BufferedInputStream bufRead = new BufferedInputStream(fis);

            int n;

            while((n = bufRead.read()) != -1) {
                bufOut.write(n);
            }

            bufOut.flush();

            bufOut.close();

            bufRead.close();

            if (j1.delete()) log.debug("Delete file "+j1.getName());

            if (j1.createNewFile()) log.debug("Create file "+j1.getName());
            log.debug("Successfully append journal.");
        }
    }

    public Hashtable<Integer, Data> getDataTable(String connection_name){
        return connection_sn_data_store.get(connection_name);
    }

    public LinkedBlockingQueue<Data> getDataQueue(String connection_name){
        return connection_data_queue_table.get(connection_name);
    }

    public void shutdown(){
        scheduler.shutdown();
        log.debug("journal shutdown");
    }

}
