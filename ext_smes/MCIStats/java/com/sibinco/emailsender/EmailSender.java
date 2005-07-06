/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package com.sibinco.emailsender;

import javax.mail.Message;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.Multipart;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;
import javax.mail.internet.MimeMultipart;
import javax.mail.internet.MimeBodyPart;
import javax.activation.DataHandler;
import javax.activation.FileDataSource;
import java.util.Date;
import java.util.Properties;
import java.io.File;

/**
 * Created by IntelliJ IDEA.
 * User: igor
 * Date: Jun 1, 2005
 * Time: 10:17:41 AM
 * To change this template use File | Settings | File Templates.
 */
public class EmailSender {

    private String smtphost;
    private String from;
    private String to;
    private String mailHeader;
    private String mailBody;
    private Date sendDate;
    private File file;
    String msgText1 = "Sending a file.\n";
    public EmailSender(String smtphost, String from, String to,
                       String mailHeader, String mailBody, Date sendDate) {
        this.smtphost = smtphost;
        this.from = from;
        this.to = to;
        this.mailHeader = mailHeader;
        this.mailBody = mailBody;
        this.sendDate = sendDate;
    }

    public EmailSender(String smtphost, String from,
                       String to, String mailHeader,
                       String mailBody, Date sendDate, File file, String msgText1) {
        this.smtphost = smtphost;
        this.from = from;
        this.to = to;
        this.mailHeader = mailHeader;
        this.mailBody = mailBody;
        this.sendDate = sendDate;
        this.file = file;
        this.msgText1 = msgText1;
    }


    public void send() throws Exception {

        Properties props = new Properties();

        props.put("mail.smtp.host", smtphost);
        Session session = Session.getDefaultInstance(props, null);
        Message msg = new MimeMessage(session);
        msg.setFrom(new InternetAddress(from));
        InternetAddress[] address = {
            new InternetAddress(to)};
        InternetAddress[] address2 = {
            new InternetAddress(from)};
        msg.setReplyTo(address2);
        msg.setRecipients(Message.RecipientType.TO, address);
        msg.setSubject(mailHeader);
        msg.setSentDate(sendDate);
        msg.setText(mailBody);
        ///////////////////////////////////////
        // create and fill the first message part
	    MimeBodyPart mbp1 = new MimeBodyPart();
	    mbp1.setText(msgText1);

	    // create the second message part
	    MimeBodyPart mbp2 = new MimeBodyPart();

            // attach the file to the message
   	    FileDataSource fds = new FileDataSource(file);
	    mbp2.setDataHandler(new DataHandler(fds));
	    mbp2.setFileName(fds.getName());

	    // create the Multipart and add its parts to it
	    Multipart mp = new MimeMultipart();
	    mp.addBodyPart(mbp1);
	    mp.addBodyPart(mbp2);

	    // add the Multipart to the message
	    msg.setContent(mp);

	    // set the Date: header
	    msg.setSentDate(new Date());



        /////////////////////////////////////////

        Transport.send(msg);
    }
}
