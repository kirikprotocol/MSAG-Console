/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package com.sibinco.emailsender;

import com.sibinco.utils.Utils;
import com.sibinco.logger.LogReader;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.File;
import java.io.PrintWriter;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.FileNotFoundException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;
import java.util.List;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;
import java.text.MessageFormat;

import org.apache.log4j.Category;
import org.apache.log4j.PropertyConfigurator;

/**
 * Created by IntelliJ IDEA.
 * User: igor
 * Date: May 31, 2005
 * Time: 2:17:51 PM
 * To change this template use File | Settings | File Templates.
 */
public class McismeStatReport {
    static Category logger = Category.getInstance(McismeStatReport.class);
    public static final String SELECT_QUERY = "SELECT period, sum(missed), sum(delivered), sum(failed),\n" +
            "sum(notified) FROM MCISME_STAT where period>=? AND period <? GROUP BY period\n" +
            " ORDER BY period ASC";

    public static final String SELECT_QUERY_MCISME_ABONENTS = "select count(*) CNT from MCISME_ABONENTS where " +
            "(bitand(event_mask,16)=0 AND " +
            "inform='Y') OR (bitand(event_mask,16)=16 AND inform='N')";
    public static final String INSERT_MCISME_ABONENTS = " INSERT INTO " +
            "MCISME_ABONENTS( ABONENT, INFORM, NOTIFY, INFORM_ID, NOTIFY_ID, EVENT_MASK)" +
            " VALUES(?, ?, ?, ?, ?, ?)";

    private static Connection connection;  // db  connection object
    private static final String PROP_FILE = "--prop_file";
    private static final String PERIOD_FROM = "--per_from";
    private static final String PERIOD_TO = "--per_to";
    private static final String SEND_MAIL = "--send_mail";

    private static final String[] HELP = new String[]{
        "Format to run: ",
        "\n --prop_file - the path to property file",
        "\n --per_from - start period - use format: yyyyMMddHH as integer, example 2005112316  ",
        "\n --per_to   - end period   - use format: yyyyMMddHH as integer, example 2005112316  ",
        "\n --send_mail - an argument for email property if not empty the report will send by email"
    };


    public static void main(String[] args) {

        File file = null;
        String log_unic_abonents = null;
        String mciprof_file = null;
        String fileName = "";
        int periodFrom = 0;
        int periodTo = 0;
        if (args.length == 0) {
            help();
            return;
        }

        boolean isEmail = false;

        for (int i = 0, len = args.length; i < len; i++) {
            if (args[i].equalsIgnoreCase(PERIOD_FROM)) {
                try {
                    periodFrom = new Integer(args[++i]).intValue();
                } catch (NumberFormatException e) {
                    logger.error("NumberFormatException " + e.getMessage());
                }

            } else if (args[i].equalsIgnoreCase(PERIOD_TO)) {
                try {
                    periodTo = new Integer(args[++i]).intValue();
                } catch (NumberFormatException e) {
                    logger.error("NumberFormatException " + e.getMessage());
                }
            } else if (args[i].equalsIgnoreCase(SEND_MAIL)) {
                isEmail = true;
            } else if (args[i].equalsIgnoreCase(PROP_FILE)) {
                if (i == len - 1) {
                    logger.error("Incorrect parameters. Missed property file. See help.\n");
                    help();
                    return;
                }
                fileName = args[++i];
            } else {
                logger.error("Unknown parameters '" + args[i] + "'. See help.\n");
                help();
                return;
            }
        }

        try {

            // Load the properties file to get the connection information and other information

            MessageFormat date = new MessageFormat("{0}");

            Properties prop = null;
            try {
                prop = loadParams(fileName);
            } catch (IOException e) {
                logger.error("Con't load property file" + fileName);
            }

            String currentDate = Utils.formatDate(new Date(), "yyyyMMdd");
            file = new File(prop.get("out_file").toString() + currentDate + ".txt");

            log_unic_abonents = prop.get("log_unic_abonents").toString();
            if (log_unic_abonents == null) {
                logger.error("Property " + log_unic_abonents + " is null!");
                throw new NullPointerException("Property " + log_unic_abonents + " is null!");
            }
            mciprof_file = prop.get("mciprof_file").toString();
            if (mciprof_file == null) {
                logger.error("Property " + mciprof_file + " is null!");
                throw new NullPointerException("Property " + mciprof_file + " is null!");
            }
            getConnection(prop);
            //checkTables();
            String msgText1 = "\n";
            long abonents = 0;
            McismeStat[] mcismeStats = null;
            if (periodFrom == 0) {
                String periodFromTime = Utils.getBeforeDay() + "00";
                try {
                    periodFrom = new Integer(periodFromTime).intValue();
                } catch (NumberFormatException e) {
                    logger.error("NumberFormatException " + e.getMessage());
                }
            }
            if (periodTo == 0) {
                String periodToTime = Utils.getBeforeDay() + "24";
                try {
                    periodTo = new Integer(periodToTime).intValue();
                } catch (NumberFormatException e) {
                    logger.error("NumberFormatException " + e.getMessage());
                }
            }
            mcismeStats = getMcismeStats(periodFrom, periodTo);
            abonents = countMcismeAbonents();

            MessageFormat form = new MessageFormat(prop.get("header.message").toString(), new Locale("RU"));
            MessageFormat upcolumns = new MessageFormat(prop.get("up.columns.message").toString(), new Locale("RU"));
            MessageFormat dwcolumns = new MessageFormat(prop.get("dw.columns.message").toString(), new Locale("RU"));
            MessageFormat unicAbonents = new MessageFormat(prop.get("message.unic.abonents").toString(), new Locale("RU"));
            MessageFormat denialService = new MessageFormat(prop.get("message.denial.Service.abonents").toString(), new Locale("RU"));
            MessageFormat applyToServise = new MessageFormat(prop.get("apply.to.servise").toString(), new Locale("RU"));

            Object[] headers = {Utils.parseDate(new String(String.valueOf(periodFrom)), "yyyyMMddHH"), Utils.parseDate(new String(String.valueOf(periodTo)), "yyyyMMddHH")};
            StringBuffer buffer = new StringBuffer();

            buffer.append("\n\n").append(form.format(headers)).append("\n\n").
                    append(upcolumns.format(null)).append("\n").append(dwcolumns.format(null)).append("\n\n");
            for (int i = 0; i < mcismeStats.length; i++) {
                McismeStat mcismeStat = mcismeStats[i];
                buffer.append(getHours(mcismeStat.getPeriod()));
                buffer.append("                ");
                buffer.append(mcismeStat.getSumMissed());

                buffer.append("       ");
                buffer.append(mcismeStat.getSumDelivered());
                buffer.append("          ");
                buffer.append(mcismeStat.getSumFailed());
                buffer.append("             ");
                buffer.append(mcismeStat.getSumNotified());
                buffer.append("\n");
            }

            //System.out.println(buffer.toString());
            buffer.append("\n\n");
            buffer.append(denialService.format(null)).append(" ").append(abonents);
            buffer.append("\n\n");
            buffer.append(unicAbonents.format(null)).append(" ").append(LogReader.getAbonents(log_unic_abonents, new Date(), "D"));
            buffer.append("\n\n");
            buffer.append(applyToServise.format(null)).append(" ").append(LogReader.applyToServise(mciprof_file, new Date(), prop.getProperty("session.pattern")));

            msgText1 = buffer.toString();

            OutputStreamWriter fout = null;
            file.createNewFile();
            String fileStr = file.getAbsolutePath();
            if (file.exists()) {
                fout = new OutputStreamWriter(new FileOutputStream(file), "windows-1251");
                PrintWriter fw = new PrintWriter(fout, true);
                fw.println();
                fw.println(buffer.toString());
                fw.println();
                fw.println();

                fw.flush();
                fout.close();
                logger.info("");
                logger.info("--------------------------------------------------------------");
                logger.info("You can see all details in file: " + fileStr + ";");

            } else {
                logger.error("File " + fileStr + " dosn't exist!");
            }
            if (isEmail) {// sending email

                String smtphost = (String) prop.get("smtphost");
                String mailFrom = (String) prop.get("mailFrom");
                String mailTo = (String) prop.get("mailTo");
                String subject = (String) prop.get("subject");

                String msgBody = "Some information";

                EmailSender sendMail = new EmailSender(smtphost,
                        mailFrom, mailTo,
                        subject,
                        msgBody, new Date(), file, msgText1);
                sendMail.send();
            }
            connection.close();
        } catch (SQLException e) {
            e.printStackTrace();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (connection != null) {
                try {
                    connection.close();
                    logger.info("Connection closed");
                } catch (SQLException e) {
                    logger.error(e.getMessage());
                }
            }
        }
    }

    /**
     * This method reads a properties file which is passed as
     * the parameter to it and load it into a java Properties
     * object and returns it.
     */
    private static Properties loadParams(String file) throws IOException {
        // Loads a ResourceBundle and creates Properties from it
        Properties prop = new Properties();
        prop.load(new FileInputStream(file));
        return prop;
    }

    /**
     * Creates a database connection object using DataSource object. Please
     * substitute the database connection parameters with appropriate values in
     * Connection.properties file
     */
    private static Connection getConnection(Properties prop) {
        try {
            logger.info("Trying to connect to the Database");
            Class.forName(prop.getProperty("jdbc.driver")).newInstance();
            connection = DriverManager.getConnection(prop.getProperty("jdbc.source"), prop.getProperty("jdbc.user"), prop.getProperty("jdbc.pass"));
        } catch (SQLException ex) { // Trap SQL errors
            ex.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        } catch (ClassNotFoundException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        } catch (InstantiationException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }
        logger.info("Connected");
        return connection;
    }

    /**
     * Fills the McismeStat array with initial entries which can be selected and information
     * about Mcisme Statatistic by period
     */
    private static McismeStat[] getMcismeStats(int period1, int period2) {

        List list = new ArrayList();
        ResultSet rset = null;
        try {

            PreparedStatement pstmt = connection.prepareStatement(SELECT_QUERY);
            pstmt.setInt(1, period1);
            pstmt.setInt(2, period2);
            // Execute the query
            rset = pstmt.executeQuery();

            while (rset.next()) { // Point result set to next row
                // Retrieve column values for this row
                McismeStat mcismeStat = new McismeStat();
                mcismeStat.setPeriod(rset.getString("PERIOD"));
                mcismeStat.setSumMissed(rset.getString(2));
                mcismeStat.setSumDelivered(rset.getString(3));
                mcismeStat.setSumFailed(rset.getString(4));
                mcismeStat.setSumNotified(rset.getString(5));
                list.add(mcismeStat);
            }
            rset.close();
            pstmt.close();   // Close the connection object

        } catch (SQLException ex) { // Trap SQL errors
            logger.error(" Error selecting records from database table: " + '\n' + ex.toString());
        }
        return (McismeStat[]) list.toArray(new McismeStat[list.size()]);
    }

    /**
     * Close the database Connection and exit the application
     */
    private static void closeConnection() {
        if (connection != null) {
            try {
                connection.close();   // close the connection when exit
            } catch (SQLException ex) {
                ex.printStackTrace();
            }
        }
    }


    /**
     * This method creates the table  in the database
     */
    private static void createTableMCISME_STAT() {
        try {

            // Statement object for executing the query
            Statement stmt = connection.createStatement();
            String createTableMCISME_STAT = "CREATE TABLE MCISME_STAT ( " +
                    " PERIOD NUMBER(22) NOT NULL,  " +
                    " MISSED NUMBER(22) NOT NULL,  " +
                    " DELIVERED NUMBER(22) NOT NULL, " +
                    " FAILED NUMBER(22) NOT NULL, " +
                    " NOTIFIED NUMBER(22) NOT NULL )";


            String createIndexQuery = "CREATE INDEX MCISME_PERIOD_IDX ON MCISME_STAT (PERIOD)";
            // Execute the statement
            stmt.executeQuery(createTableMCISME_STAT);
            stmt.executeQuery(createIndexQuery);

            // Closing the Statement
            stmt.close();
        } catch (SQLException ex) {
            ex.printStackTrace();
            logger.error("Error creating the table. " + ex.toString());
        }
    }

    /**
     * This method creates the table  in the database
     */
    private static void createTableMCISME_ABONENTS() {
        try {

            // Statement object for executing the query
            Statement stmt = connection.createStatement();

            String createTableMCISME_ABONENTS = "CREATE TABLE MCISME_ABONENTS( " +
                    " ABONENT VARCHAR2(30) NOT NULL CONSTRAINT MCISME_ABONENTS_ABONENT_PK PRIMARY KEY," +
                    " INFORM CHAR(1) NULL," +
                    " NOTIFY CHAR(1) NULL," +
                    " INFORM_ID  NUMBER(22) NULL," +
                    " NOTIFY_ID  NUMBER(22) NULL," +
                    " EVENT_MASK NUMBER(3) NULL)";

            // Execute the statement
            stmt.executeQuery(createTableMCISME_ABONENTS);

            // Closing the Statement
            stmt.close();
        } catch (SQLException ex) {
            ex.printStackTrace();
            logger.error("Error creating the table. " + ex.toString());
        }
    }

    /**
     * This method is return count abonents Denial of Service
     */
    private static long countMcismeAbonents() {
        long result = 0;
        try {
            ResultSet rset = null;
            // Statement object for executing the query
            Statement stmt = connection.createStatement();

            // Execute the statement
            rset = stmt.executeQuery(SELECT_QUERY_MCISME_ABONENTS);
            while (rset.next()) {
                return rset.getInt("CNT");
            }
            // Closing the ResultSet
            rset.close();
            // Closing the Statement
            stmt.close();
        } catch (SQLException ex) {
            ex.printStackTrace();
            logger.error("Error creating the table. " + ex.toString());
        }
        return result;
    }


    /**
     * Checks if the table ('MCISME_STAT') is present, else creates it.
     * Look into PopulateTable class methods for more details
     */
    private static void checkTables() {
        Statement stmt = null;
        ResultSet rset = null;
        try {
            stmt = connection.createStatement();

            // check from User_tables data dictionary table if the table is existing.
            rset = stmt.executeQuery(" SELECT Table_Name FROM User_Tables " +
                    " WHERE Table_Name = 'MCISME_STAT' ");

            // if the resultset of the above query does not have any record, it means
            // table is not existing. So the table is created.
            if (!rset.next()) {
                // call the class to create the table
                createSchemaTableMCISME_STAT();
            }

            // check from User_tables data dictionary table if the table is existing.
            rset = stmt.executeQuery(" SELECT Table_Name FROM User_Tables " +
                    " WHERE Table_Name = 'MCISME_ABONENTS' ");

            // if the resultset of the above query does not have any record, it means
            // table is not existing. So the table is created.
            if (!rset.next()) {
                // call the class to create the table
                createSchemaTableMCISME_ABONENTS();
            }

        } catch (SQLException sqlEx) {
            logger.error("Could not create table MCISME_STAT : " + sqlEx.toString());
        } finally {
            try {
                if (rset != null) rset.close();
                if (stmt != null) stmt.close();
            } catch (SQLException ex) {
            }
        }
    }

    /**
     * Important method of the class. Sets up the database connection and
     * creates the MCISME_STAT table, and populates it.
     */
    public static void createSchemaTableMCISME_STAT() {
        try {
            createTableMCISME_STAT(); //Create the MCISME_STAT table

            // Insert rows into the table yyyyMMddHH
            insertData(2005051611, 17437, 3413, 1231, 31);
            insertData(2005051612, 17275, 3398, 1151, 31);
            insertData(2005051613, 17722, 3604, 1118, 57);
            insertData(2005051614, 17683, 3443, 1176, 46);
            insertData(2005051615, 18585, 3552, 1160, 66);
            insertData(2005051616, 19824, 4021, 1129, 56);
            insertData(2005051617, 20724, 4248, 1284, 34);
            insertData(2005051618, 20453, 4120, 1450, 45);
            insertData(2005051619, 21124, 4205, 1554, 66);
            insertData(2005051620, 22353, 4424, 1564, 68);
            insertData(2005051621, 21538, 4327, 1278, 83);
            insertData(2005051622, 14846, 3130, 771, 71);
            insertData(2005051623, 8356, 1754, 322, 52);


        } catch (Exception ex) {
            logger.error("Error in inserting data. " + ex.toString());
        }
    }

    /**
     * Important method of the class. Sets up the database connection and
     * creates the MCISME_ABONENTS table, and populates it.
     */
    public static void createSchemaTableMCISME_ABONENTS() {
        try {
            createTableMCISME_ABONENTS(); //Create the MCISME_STAT table

            insertDataToMCISME_ABONENTS("+79172594597", 'Y', 'N', 4, 3, 31);
            insertDataToMCISME_ABONENTS("+79178666976", 'Y', 'Y', 4, 3, 31);
            insertDataToMCISME_ABONENTS("+79178581683", 'N', 'Y', 3, 2, 31);
            insertDataToMCISME_ABONENTS("+79178573613", 'Y', 'N', 3, 3, 31);
            insertDataToMCISME_ABONENTS("+79178668041", 'Y', 'Y', 3, 2, 31);
            insertDataToMCISME_ABONENTS("+79172818156", 'N', 'N', 4, 3, 15);
            insertDataToMCISME_ABONENTS("+79178674180", 'Y', 'N', 4, 3, 31);
            insertDataToMCISME_ABONENTS("+79172604614", 'Y', 'Y', 3, 3, 15);
            insertDataToMCISME_ABONENTS("+79178614587", 'Y', 'Y', 3, 3, 31);
            insertDataToMCISME_ABONENTS("+79172626524", 'N', 'N', 4, 3, 15);
            insertDataToMCISME_ABONENTS("+79172650405", 'N', 'N', 3, 3, 31);
            insertDataToMCISME_ABONENTS("+79178630775", 'N', 'N', 4, 2, 31);
            insertDataToMCISME_ABONENTS("+79172755557", 'Y', 'N', 4, 3, 15);
            insertDataToMCISME_ABONENTS("+79178556857", 'N', 'N', 4, 3, 31);
            insertDataToMCISME_ABONENTS("+79172944056", 'Y', 'Y', 3, 3, 31);
            insertDataToMCISME_ABONENTS("+79172742600", 'Y', 'N', 4, 3, 31);
            insertDataToMCISME_ABONENTS("+79172583656", 'N', 'N', 4, 3, 15);
            insertDataToMCISME_ABONENTS("+79172933338", 'N', 'N', 4, 3, 31);
            insertDataToMCISME_ABONENTS("+79172412182", 'N', 'N', 4, 2, 31);
            insertDataToMCISME_ABONENTS("+79172733319", 'Y', 'N', 3, 2, 15);
            insertDataToMCISME_ABONENTS("+79172856981", 'Y', 'Y', 4, 3, 31);
            insertDataToMCISME_ABONENTS("+79172649887", 'Y', 'N', 4, 3, 31);


        } catch (Exception ex) {
            logger.error("Error in inserting data. " + ex.toString());
        }
    }


    /**
     * This method inserts record in the MCISME_STAT table
     */
    public static void insertDataToMCISME_ABONENTS(String abonent, char inform, char notify,
                                                   int inform_id, int notify_id, int event_mask) throws Exception {

        // Prepare the statement for inserting a row into the MCISME_STAT
        PreparedStatement pstmt = connection.prepareStatement(INSERT_MCISME_ABONENTS);

        // Bind the parameter values for the above statement
        pstmt.setString(1, abonent);
        pstmt.setString(2, String.valueOf(inform));
        pstmt.setString(3, String.valueOf(notify));
        pstmt.setInt(4, inform_id);
        pstmt.setInt(5, notify_id);
        pstmt.setInt(6, event_mask);

        // Execute the statement
        pstmt.execute();
    }

    /**
     * This method inserts record in the MCISME_STAT table
     */
    public static void insertData(int period, int missed, int delivered,
                                  int failed, int notified) throws Exception {

        // Prepare the statement for inserting a row into the MCISME_STAT
        PreparedStatement pstmt = connection.prepareStatement(" INSERT INTO " +
                "MCISME_STAT( PERIOD, MISSED, DELIVERED, FAILED, NOTIFIED)" +
                " VALUES(?, ?, ?, ?, ?)");

        // Bind the parameter values for the above statement
        pstmt.setInt(1, period);
        pstmt.setInt(2, missed);
        pstmt.setInt(3, delivered);
        pstmt.setInt(4, failed);
        pstmt.setInt(5, notified);

        // Execute the statement
        pstmt.execute();
    }


    /**
     * Печатаем стандартный поток вывода - помощь для выполнения
     */
    protected static void help() {
        for (int i = 0, len = HELP.length; i < len; ++i) {
            logger.info(HELP[i]);
        }
    }

    private static String getHours(String value) {
        return value.substring(8, 10);
    }


}
