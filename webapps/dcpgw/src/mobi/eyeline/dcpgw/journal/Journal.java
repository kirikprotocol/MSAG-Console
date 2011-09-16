package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.dcpgw.Gateway;
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

    //private Hashtable<Long, Data> message_id_receipt_table;
    //private Hashtable<Integer, Long> sequence_number_message_id_table;


    public Journal() throws InitializationException{

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
        log.debug("Successfully append journal.");
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
            log.error("Size of the journal after appending string will be more than maximum allowed journal size "+ max_journal_size_mb +" mb.");
            throw new CouldNotWriteToJournalException("Size of the journal after appending string will be more than maximum allowed juornal size "+ max_journal_size_mb +" mb.");
        }

    }

    public void clean(){
        log.debug("Try to clean journal ... ");

        long t = System.currentTimeMillis();

        if (j1.exists()){
            BufferedReader buffReader1 = null;
            BufferedReader buffReader2 = null;
            PrintWriter pw = null;
            try {
                if (j2.createNewFile()) log.debug("Create file "+j2.getName());

                appendFile(j1, j2);

                if (j1.delete()) log.debug("Delete file "+j1.getName());

                if (j1.createNewFile()) log.debug("Create file "+j1.getName());

                if (j2t.createNewFile()) log.debug("Create file "+j2t.getName());

                pw = new PrintWriter(new FileWriter(j2t));

                Set<Long> message_ids = new HashSet<Long>();

                // Читаем очищаемый файл первый раз, чтобы запомнить message_id-ы сообщений, которые уже отработаны.
                buffReader1 = new BufferedReader (new FileReader(j2));
                String line;
                while((line = buffReader1.readLine()) != null){
                    String[] ar = line.split(sep);
                    long message_id = Long.parseLong(ar[2].trim());
                    Status status = Status.valueOf(ar[11].trim());

                    if (status == Status.DONE || status == Status.EXPIRED_TIMEOUT || status == Status.EXPIRED_MAX_TIMEOUT) {
                        if (message_ids.add(message_id)){
                            log.debug(message_id+"_message has "+status+" status, remember it.");
                        } else {
                            log.warn("Couldn't remember message with status "+status+" because it's already added to the map. ");
                        }
                    }
                }
                buffReader1.close();

                // Читаем второй раз, при этом копируя не отработанные записи во временный файл.
                int counter = 0;
                buffReader2 = new BufferedReader (new FileReader(j2));
                while((line = buffReader2.readLine()) != null){
                    String[] ar = line.split(sep);

                    long message_id = Long.parseLong(ar[2].trim());
                    String status = ar[11].trim();

                    if (!message_ids.contains(message_id)){
                        log.debug(message_id+"_message has "+status+" status, write it to the temporary journal "+j2t.getName());
                        pw.println(line);
                        pw.flush();
                        counter++;
                    }

                }
                buffReader2.close();
                pw.close();

                if (counter>0){

                    if (j2.delete()) log.debug("Delete file "+j2.getName());
                    if (j2t.renameTo(j2)) log.debug("Rename file "+j2t.getName()+" to "+j2.getName());

                } else {

                    if (j2t.delete()) log.debug("Delete file "+j2t.getName());
                    if (j2.delete()) log.debug("Delete file "+j2.getName());
                    if (j2.createNewFile()) log.debug("Create file "+j2.getName());

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

    public Hashtable<Long, Data> load() throws CouldNotLoadJournalException {
        log.debug("Try to load journal to the memory ...");

        Hashtable<Long, Data> table = new Hashtable<Long, Data>();

        // Проверяем существует ли временный файл, если он существует, значит шлюз закончил работу не стандартно во время
        // очистки журнала.
        if (j2t.exists()){
            log.debug("Detected that file '"+j2t.getName()+"' exist.");
            // Проверяем существует ли файл, который чистится.
            if (j2.exists()){
                log.debug("Detected that file '"+j2.getName()+"' exist.");
                // Если файл, который чистится существует, значит шлюз закончил работу во время выбора не отработанных
                // записей. При этом не все не отработанные записи могли быть перенесены во временный файл, поэтому
                // файл который используется для чистки нужно чистить еще раз, временный файл нужно удалить.
                if (j2t.delete()){
                    log.debug("Successfully delete file '"+j2t+"'.");
                } else {
                    log.debug("Couldn't delete file '"+j2t+"'.");
                    throw new CouldNotLoadJournalException("Couldn't delete file '"+j2t+"'.");
                }
            } else {
                log.debug("Detected that file '"+j2.getName()+"' doesn't exist.");
                // Если файла, который чистится нет, а временный есть, то шлюз прекратил работу в тот момент, когда
                // все не отработанные записи были уже перенесены во временный файл, а файл который чистится был удален.
                // Поэтому надо временный файл, переименовать в файл, который чистится.
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

                            Status status  =  Status.valueOf(ar[11]);

                            if (status == Status.DONE
                                    || status == Status.EXPIRED_MAX_TIMEOUT
                                        ||  status == Status.EXPIRED_TIMEOUT){
                                Gateway.removeDeliveryReceiptData(sequence_number);
                                log.debug("Remove from memory delivery receipt data with message id "+sequence_number+" .");
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
                                data.setSequenceNumber(sequence_number);
                                data.setStatus(status);
                                Gateway.addDeliveryReceiptData(sequence_number, data);
                                log.debug("Write in memory delivery receipt data with system id "+message_id+" --> "+ data.toString()+" .");
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
