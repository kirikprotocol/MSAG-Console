package mobi.eyeline.smsquiz.subscription.script;

import mobi.eyeline.smsquiz.subscription.datasource.*;
import mobi.eyeline.smsquiz.subscription.datasource.impl.DBSubscriptionDataSource;
import mobi.eyeline.smsquiz.subscription.Subscription;
import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.storage.StorageException;

import java.io.*;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * author: alkhal
 */
public class SubscribedAbDownloader {

    private static SubscriptionDataSource dataSource;
    private static SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
    private static SimpleDateFormat dateParser = new SimpleDateFormat("dd.MM.yyyy");
    private static void init() {
        try {
            ConnectionPoolFactory.init("conf/config.xml");
        } catch (StorageException e) {
            e.printStackTrace();
        }
    }
    public static void main(String[] args){
        init();
        ResultSet result = null;
        PrintWriter writer=null;
        Subscription sub=null;
        Date wantedDate = null;
        String dest;

        if(args.length<1) {
            help();
            return;
        }
        else {
            dest = args[0];
            if  (args.length>=2) {
                try {
                    wantedDate = dateParser.parse(args[1]);
                    System.out.println("Second argument defined");
                } catch (ParseException e) {
                    help();
                    return;
                }
            }
        }

        if(wantedDate==null) {
            wantedDate=new Date();
        }
        try {
            writer = new PrintWriter(new BufferedWriter(new FileWriter(dest))) ;
            dataSource = new DBSubscriptionDataSource();
            result = dataSource.list(wantedDate);
            while (result.next()) {
                if((sub=(Subscription)result.get())!=null){
                    writer.print(sub.getAddress());
                    writer.print(",");
                    writer.println(dateFormat.format(sub.getStartDate()));
                }
            }
            writer.flush();
            System.out.println("File created");
        } catch (IOException e) {
            e.printStackTrace();
        } catch (StorageException e) {
            e.printStackTrace();
        } finally {
            if(writer!=null) {
                writer.close();
            }
            if(dataSource!=null) {
                dataSource.close();
            }
            if(result!=null) {
                result.close();
            }
        }
        System.exit(1);
    }


    private static void help() {
        System.out.println("Usage with arguments:");
        System.out.println("<destination_file>  //Download the file into <destanation_file> with addresses, which  subscribed now");
        System.out.println("<destination_file> <date> //Download the file into <destanation_file> with addresses, which (was) subscribed on <date>. Format: "+"dd.MM.yyyy");

    }
}
