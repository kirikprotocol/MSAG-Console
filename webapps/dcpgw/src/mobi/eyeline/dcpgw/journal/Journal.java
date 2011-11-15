package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.smpp.Connection;
import mobi.eyeline.dcpgw.smpp.Server;
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

    private int max_journal_size_mb;
    private int max_submit_date_journal_size_mb;


    private File j1, j2, j2t;
    private BufferedWriter bw, bwt, sdbw, sdbwt;

    private final Object monitor = new Object();
    private final Object monitor2 = new Object();

    private Hashtable<String, Hashtable<Integer, DeliveryData>> connection_sn_data_store;
    private Hashtable<String, LinkedBlockingQueue<DeliveryData>> connection_data_queue_table;
    private Hashtable<String, Hashtable<Long, SubmitData>> connection_message_id_sdata_store;

    private ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    // Submit date journal
    private File sdj1;
    private File sdj2;
    private File sdj2t;

    private static final int buffer_size = 8192;

    public void init(File journal_dir, int max_journal_size_mb, int max_submit_date_journal_size_mb, int clean_timeout) throws InitializationException{
        log.debug("Try to initialize the journal ...");
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
            log.debug("Detected that delivery journal directory doesn't exist.");
            if (journal_dir.mkdir()){
                log.debug("Create delivery journal directory.");
            } else {
                log.error("Couldn't create delivery journal directory, check permissions.");
                throw new InitializationException("Couldn't create delivery journal directory, check permissions.");
            }
        } else {
            log.debug("Detected that delivery journal directory already exists.");
        }

        if (!sddir.exists()){
            log.debug("Detected that submit journal directory doesn't exist.");
            if (sddir.mkdir()){
                log.debug("Create submit journal directory.");
            } else {
                log.error("Couldn't create submit journal directory, check permissions.");
                throw new InitializationException("Couldn't create submit journal directory, check permissions.");
            }
        } else {
            log.debug("Detected that submit journal directory already exists.");
        }


        scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                try{
                    cleanDeliveryJournal();
                } catch (Throwable e){
                    log.error("Couldn't clean delivery journal.", e);
                }
                try{
                    cleanSubmitJournal();
                } catch (Throwable e){
                    log.error("Couldn't clean submit journal.", e);
                }
            }

        }, clean_timeout, clean_timeout, TimeUnit.MILLISECONDS);


        connection_sn_data_store = new Hashtable<String, Hashtable<Integer, DeliveryData>>();
        connection_data_queue_table = new Hashtable<String, LinkedBlockingQueue<DeliveryData>>();
        connection_message_id_sdata_store = new Hashtable<String, Hashtable<Long, SubmitData>>();

        log.debug("Journal is initialized.");
    }

    public void load() throws CouldNotLoadJournalException {
        log.debug("Try to load the journal ...");

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

        Hashtable<String, LinkedHashMap<Long, DeliveryData>> connection_message_id_data_table = new Hashtable<String, LinkedHashMap<Long, DeliveryData>>();

        for(File f: journals){
            if (f.exists()){
                log.debug("Read file "+f.getName());
                try {
                    Scanner scanner = new Scanner(f);
                    scanner.useDelimiter(sep);
                    while (scanner.hasNextLine()){
                        String line = scanner.nextLine();
                        if (!line.isEmpty() && !line.startsWith("#")){

                            DeliveryData data;
                            try {
                                data = DeliveryData.parse(line);
                            } catch (ParseException e) {
                                throw new CouldNotLoadJournalException(e);
                            } catch (InvalidAddressFormatException e) {
                                throw new CouldNotLoadJournalException(e);
                            }

                            DeliveryData.Status status = data.getStatus();
                            long message_id = data.getMessageId();
                            String connection = data.getConnectionName();

                            // In memory states: SEND, INIT, RESEND.

                            if (status == DeliveryData.Status.INIT || status == DeliveryData.Status.RESEND){

                                if (!connection_data_queue_table.containsKey(connection))
                                    connection_data_queue_table.put(connection, new LinkedBlockingQueue<DeliveryData>());

                                if (!connection_message_id_data_table.containsKey(connection))
                                    connection_message_id_data_table.put(connection, new LinkedHashMap<Long, DeliveryData>());

                                LinkedHashMap<Long, DeliveryData> message_id_data_map = connection_message_id_data_table.get(connection);
                                message_id_data_map.put(message_id, data);

                            } else if (status == DeliveryData.Status.SEND) {

                                Integer sequence_number = data.getSequenceNumber();

                                if (!connection_sn_data_store.containsKey(connection))
                                        connection_sn_data_store.put(connection, new Hashtable<Integer, DeliveryData>());

                                if (status == DeliveryData.Status.DONE
                                        || status == DeliveryData.Status.EXPIRED_MAX_TIMEOUT
                                        || status == DeliveryData.Status.EXPIRED_TIMEOUT
                                        || status == DeliveryData.Status.TEMP_ERROR
                                        || status == DeliveryData.Status.PERM_ERROR
                                        || status == DeliveryData.Status.DELETED              ){

                                    connection_sn_data_store.get(connection).remove(sequence_number);
                                    log.debug("remove from memory delivery data: con="+connection+", sn="+sequence_number);

                                } else if (status == DeliveryData.Status.SEND) {

                                    connection_sn_data_store.get(connection).put(sequence_number, data);

                                    log.debug("add to the table: " + data.toString());
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

        // Add last delivery data with INIT and RESEND status to the queues.
        for(String connection: connection_message_id_data_table.keySet()){
            LinkedHashMap<Long, DeliveryData> message_id_data_map = connection_message_id_data_table.get(connection);
            LinkedBlockingQueue<DeliveryData> queue = connection_data_queue_table.get(connection);
            for(Long message_id: message_id_data_map.keySet()){
                DeliveryData data = message_id_data_map.get(message_id);
                queue.add(data);
                log.debug("add to queue: "+data);
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

                            SubmitData sdata;
                            try {
                                sdata = SubmitData.parse(line);
                            } catch (ParseException e) {
                                log.debug(e);
                                throw new CouldNotLoadJournalException(e);
                            }


                            long message_id = sdata.getMessageId();
                            String connection_name = sdata.getConnectionName();
                            SubmitData.Status status = sdata.getStatus();

                            if (!connection_message_id_sdata_store.containsKey(connection_name)){
                                connection_message_id_sdata_store.put(connection_name, new Hashtable<Long, SubmitData>());
                            }

                            Hashtable<Long, SubmitData> message_id_sdata = connection_message_id_sdata_store.get(connection_name);

                            if (status != SubmitData.Status.RECEIVE_DELIVERY_RECEIPT){
                                message_id_sdata.put(message_id, sdata);
                                log.debug("remove from memory submit data: con="+connection_name+", message_id="+message_id);
                            } else {
                                message_id_sdata.remove(message_id);
                                log.debug("add to memory: "+sdata);
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

    public void write(DeliveryData data) throws CouldNotWriteToJournalException {

        synchronized (monitor){

            try {
                if (j1.createNewFile()){
                    log.debug("Create journal "+j1.getName());
                }
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotWriteToJournalException(e);
            }

            String s = DeliveryData.format(data);

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
                        bw = new BufferedWriter(new FileWriter(j1, true), buffer_size);
                        log.debug("Initialize buffered writer for journal "+j1.getName());
                    }
                    bw.write(s + "\n");
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
            sum = j1.length() + buffer_size;
        } else {
            sum = j1.length() + j2.length() + buffer_size;
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
            sum = sdj1.length() + buffer_size;
        } else {
            sum = sdj1.length() + sdj2.length() + buffer_size;
        }

        //log.debug("Length of the journal in bytes: "+sum);

        return sum+byteCount <= max_submit_date_journal_size_mb*1024*1024;
    }

    public void cleanDeliveryJournal(){
        log.debug("Try to clean delivery journal ...");
        long t = System.currentTimeMillis();

        if (!j1.exists()){
            log.debug("Delivery journal "+j1.getName()+" doesn't exist.");
            return;
        }

        BufferedReader buffReader1 = null;
        BufferedReader buffReader2 = null;
        try {
            if (j2.createNewFile()) log.debug("Create file "+j2.getName());

            synchronized (monitor){
                bw.flush();

                if (j1.length() > 0){

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

                    log.debug("Successfully append delivery journal.");

                    bw = new BufferedWriter(new FileWriter(j1, true), buffer_size);
                    log.debug("Initialize buffered writer for journal "+j1.getName());

                }

                if (j2.length() == 0){
                    log.debug("Delivery journal "+j2.getName()+ " is empty.");
                    return;
                }
            }

            if (j2t.createNewFile()) log.debug("Create file "+j2t.getName());

            bwt = new BufferedWriter(new FileWriter(j2t));

            Set<Long> done_message_ids = new HashSet<Long>();
            Set<Long> perm_errors_message_ids = new HashSet<Long>();
            Set<Long> expired_max_messages_ids = new HashSet<Long>();
            Set<Integer> expired_sequence_numbers = new HashSet<Integer>();
            Set<Integer> temp_error_sequence_numbers = new HashSet<Integer>();
            Set<Long> sended_message_ids = new HashSet<Long>();
            Set<Long> deleted_messages_ids = new HashSet<Long>();
            Hashtable<Long, Long> resended_message_id_last_send_time_table = new Hashtable<Long, Long>();

            buffReader1 = new BufferedReader (new FileReader(j2));

            Set<Long> expired_send_receipt_max_time = new HashSet<Long>();

            String line;
            while((line = buffReader1.readLine()) != null){

                if (line.isEmpty()){
                    log.error("Delivery journal contains empty line!!!");
                    continue;
                }

                DeliveryData data;
                try {
                    data = DeliveryData.parse(line);
                    //log.debug("read data:"+data);
                } catch (Exception e) {
                    log.debug("Couldn't parse journal line: "+line, e);
                    continue;
                }

                long message_id = data.getMessageId();
                DeliveryData.Status status = data.getStatus();


                long init_time = data.getInitTime();
                String connaction_name = data.getConnectionName();

                Connection connection = Server.getInstance().getConnection(connaction_name);
                int send_receipt_max_time = connection.getSendReceiptMaxTime();

                if (System.currentTimeMillis() - init_time > send_receipt_max_time * 60 * 1000){
                    log.warn("Journal record with message id "+message_id+" expired.");
                    expired_send_receipt_max_time.add(message_id);
                }


                if (status == DeliveryData.Status.DONE) {
                    done_message_ids.add(message_id);
                } else if (status == DeliveryData.Status.PERM_ERROR){
                    perm_errors_message_ids.add(message_id);
                } else if (status == DeliveryData.Status.DELETED){
                    deleted_messages_ids.add(message_id);
                } else if (status == DeliveryData.Status.EXPIRED_MAX_TIMEOUT){
                    expired_max_messages_ids.add(message_id);
                } else if (status == DeliveryData.Status.EXPIRED_TIMEOUT){
                    int sequence_number = data.getSequenceNumber();
                    expired_sequence_numbers.add(sequence_number);
                } else if (status == DeliveryData.Status.TEMP_ERROR){
                    int sequence_number = data.getSequenceNumber();
                    temp_error_sequence_numbers.add(sequence_number);
                } else if (status == DeliveryData.Status.SEND ){
                    sended_message_ids.add(message_id);
                } else if (status == DeliveryData.Status.RESEND){
                    long last_send_time = data.getLastSendTime();
                    resended_message_id_last_send_time_table.put(message_id, last_send_time);
                }

            }
            buffReader1.close();
            log.debug("Read file "+j2);

            int counter = 0;
            buffReader2 = new BufferedReader(new FileReader(j2));

            while((line = buffReader2.readLine()) != null){

                if (line.isEmpty()){
                    log.error("Delivery journal contains empty line!!!");
                    continue;
                }

                DeliveryData data;
                try {
                    data = DeliveryData.parse(line);
                    //log.debug("read data:"+data);
                } catch (Exception e) {
                    log.debug("Couldn't parse journal line: "+line, e);
                    continue;
                }

                long message_id = data.getMessageId();
                DeliveryData.Status status = data.getStatus();

                if (status == DeliveryData.Status.INIT){

                    if (!sended_message_ids.contains(message_id)
                            && !deleted_messages_ids.contains(message_id)
                            && !resended_message_id_last_send_time_table.containsKey(message_id)
                            && !expired_max_messages_ids.contains(message_id)
                            && !expired_send_receipt_max_time.contains(message_id)){
                        //log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                        bwt.write(line+"\n");
                        counter++;
                    }

                } else if (status == DeliveryData.Status.RESEND){

                    long last_send_time = data.getLastSendTime();

                    if (last_send_time == resended_message_id_last_send_time_table.get(message_id)
                            && !sended_message_ids.contains(message_id)
                            && !deleted_messages_ids.contains(message_id)
                            && !expired_max_messages_ids.contains(message_id)
                            && !expired_send_receipt_max_time.contains(message_id)){
                        bwt.write(line+"\n");
                        counter++;
                    }

                } else if (status == DeliveryData.Status.SEND) {

                    int sequence_number = data.getSequenceNumber();

                    if (!done_message_ids.contains(message_id)
                        && !deleted_messages_ids.contains(message_id)
                        && !expired_max_messages_ids.contains(message_id)
                        && !perm_errors_message_ids.contains(message_id)

                        && !expired_sequence_numbers.contains(sequence_number)
                        && !temp_error_sequence_numbers.contains(sequence_number)
                        && !expired_send_receipt_max_time.contains(message_id) ){
                        //log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                        bwt.write(line+"\n");
                        counter++;
                    }
                }

            }

            buffReader2.close();
            bwt.close();
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
            try{
                if (buffReader1 != null) buffReader1.close();
                if (buffReader2 != null) buffReader2.close();
                if (bwt != null) bwt.close();
            } catch (IOException e){
                log.error(e);
            }
        }

        long d = System.currentTimeMillis() - t;
        log.debug("Delivery journal cleaned, "+d+" mls.");
    }

    public void cleanSubmitJournal(){

        long t = System.currentTimeMillis();

        if (!sdj1.exists()){
            log.debug("Submit journal "+sdj1.getName()+" doesn't exist.");
            return;
        }

        BufferedReader buffReader1 = null;
        BufferedReader buffReader2 = null;
        try {
            if (sdj2.createNewFile()) log.debug("Create file "+sdj2.getName());

            synchronized (monitor2){

                sdbw.flush();

                if (sdj1.length() > 0){

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

                    log.debug("Successfully append submit journal.");

                    sdbw = new BufferedWriter(new FileWriter(sdj1, true), buffer_size);
                    log.debug("Initialize buffered writer for submit journal "+sdj1.getName());
                }

                if (sdj2.length() == 0){
                    log.debug("Submit journal "+sdj2.getName()+ " is empty.");
                    return;
                }
            }

            if (sdj2t.createNewFile()) log.debug("Create file "+sdj2t.getName());

            sdbwt = new BufferedWriter(new FileWriter(sdj2t));

            Set<Long> receives_receipt_message_ids = new HashSet<Long>();

            buffReader1 = new BufferedReader (new FileReader(sdj2));

            String line;
            while((line = buffReader1.readLine()) != null){

                if (line.isEmpty()){
                    log.error("Submit journal contains empty line!!!");
                    continue;
                }

                SubmitData sdata;
                try {
                    sdata = SubmitData.parse(line);
                } catch (ParseException e) {
                    log.error("Couldn't parse submit journal string.", e);
                    continue;
                }

                long message_id = sdata.getMessageId();
                SubmitData.Status status = sdata.getStatus();

                if (status == SubmitData.Status.RECEIVE_DELIVERY_RECEIPT){
                    receives_receipt_message_ids.add(message_id);
                }
            }

            buffReader1.close();
            log.debug("Read file "+sdj2);

            int counter = 0;
            buffReader2 = new BufferedReader(new FileReader(sdj2));

            while((line = buffReader2.readLine()) != null){

                if (line.isEmpty()){
                    log.error("Submit journal contains empty line!!!");
                    continue;
                }

                SubmitData sdata;
                try {
                    sdata = SubmitData.parse(line);
                } catch (ParseException e) {
                    log.error("Couldn't parse submit journal string.", e);
                    continue;
                }

                long message_id = sdata.getMessageId();
                SubmitData.Status status = sdata.getStatus();

                if (status != SubmitData.Status.RECEIVE_DELIVERY_RECEIPT){
                    if (!receives_receipt_message_ids.contains(message_id)){
                        sdbwt.write(line+"\n");
                        counter++;
                    }
                }

            }
            buffReader2.close();
            sdbwt.close();
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
            try{
                if (buffReader1 != null) buffReader1.close();
                if (buffReader2 != null) buffReader2.close();
                if (sdbwt != null) sdbwt.close();
            } catch (IOException e2){
                log.error(e2);
            }
        }

        long d = System.currentTimeMillis() - t;
        log.debug("Submit journal cleaned, "+d+" mls.");
    }

    public Hashtable<Integer, DeliveryData> getDataTable(String connection_name){
        return connection_sn_data_store.get(connection_name);
    }

    public LinkedBlockingQueue<DeliveryData> getDataQueue(String connection_name){
        return connection_data_queue_table.get(connection_name);
    }

    public Hashtable<Long, SubmitData> getSubmitDateTable(String connection_name){
        return connection_message_id_sdata_store.get(connection_name);
    }

    public void shutdown(){

        scheduler.shutdown();

        if (bw != null) {
            try {
                /* From javadoc: Closes the stream, flushing it first. Once the stream has been closed, further write()
                or flush() invocations will cause an IOException to be thrown. Closing a previously closed stream has no
                effect. */
                bw.close();
                log.debug("Close delivery journal buffered writer.");
            } catch (IOException e) {
                log.error(e);
            }
        }

        if (bwt != null) {
            try {
                bwt.close();
                log.debug("Close delivery journal buffered writer .");
            } catch (IOException e) {
                log.error(e);
            }
        }

        if (sdbw != null){
            try{
                sdbw.close();
                log.debug("Close submit journal buffered writer.");
            } catch (IOException e) {
                log.error(e);
            }
        }

        if (sdbwt != null){
            try{
                sdbwt.close();
                log.debug("Close submit journal buffered writer.");
            } catch (IOException e) {
                log.error(e);
            }
        }

        log.debug("Journal is shut down.");
    }

    public void write(SubmitData data)throws CouldNotWriteToJournalException {

        synchronized (monitor2){

            try {
                if (sdj1.createNewFile()){
                    log.debug("Create journal "+sdj1.getName());
                }
            } catch (IOException e) {
                log.error(e);
                throw new CouldNotWriteToJournalException(e);
            }

            String s = SubmitData.format(data);

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
                        sdbw = new BufferedWriter(new FileWriter(sdj1, true), buffer_size);
                        log.debug("Initialize buffered writer for journal "+sdj1.getName());
                    }
                    sdbw.write(s+"\n");
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
