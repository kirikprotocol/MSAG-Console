/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.protocol.response;

import org.apache.log4j.Logger;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.xml.DtdsEntityResolver;


public class ResponseReader {

    private Logger logger = Logger.getLogger(this.getClass());
    private InputStream in;

    private final DocumentBuilder builder;

    public ResponseReader(InputStream inStream) throws SibincoException {
        in = inStream;
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setIgnoringComments(true);
        factory.setValidating(false);
        try {
            builder = factory.newDocumentBuilder();
        } catch (ParserConfigurationException e) {
            logger.debug("Couldn't create XML document builder", e);
            throw new SibincoException("Couldn't create XML document builder", e);
        }
        builder.setEntityResolver(new DtdsEntityResolver());
    }

    public Response read()
            throws IOException, SibincoException {
        byte[] buffer = readBytes();
        if (buffer == null)
            return null;

//        logger.debug( "ResponseReader.read() source BAIS" );

        printResponse( buffer );

        InputSource source = new InputSource(new ByteArrayInputStream(buffer));
        logger.debug( "ResponseReader.read() source with new InputStreamReader( BAIS, \"UTF-8\" )" );
        source = new InputSource( new InputStreamReader( new ByteArrayInputStream(buffer), "UTF-8") ); //added

        try {
            return new Response( builder.parse(source) );
        } catch (SAXException e) {
            logger.debug("Couldn' parse received response", e);
            throw new SibincoException("Couldn' parse received response", e);
        }
    }

    private void printResponse(byte[] buffer) {
        BufferedReader br = null;
        try{
            br = new BufferedReader( new InputStreamReader( new ByteArrayInputStream(buffer), "UTF-8") );
            logger.debug( "ResponseReader.printResponse() start\n--------" );
            String s;
            s=br.readLine();
            while( s != null ){
                String sNext = br.readLine();
                if( sNext == null ){
                    logger.debug( s + "\n--------" );
                }else{
                    logger.debug( s );
                }
                s = sNext;
            }
            logger.debug( "ResponseReader.printResponse() end." );
        } catch (UnsupportedEncodingException e) {
            logger.debug( "ResponseReader.printResponse() UnsupportedEncodingException" );
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }catch( IOException e ){
            logger.debug( "ResponseReader.printResponse() IOException" );
            e.printStackTrace();
        }finally{
            try {
                if( br!= null ) br.close();
            } catch (IOException e) {
                logger.debug( "ResponseReader.printResponse() Exception, finally" );
                e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
            }
        }
    }

    private byte[] readBytes()
            throws IOException {
        int length = readLength();
        byte buffer[] = new byte[length];
        for (int readed = 0; readed < length;) {
            int readedNow = 0;
            try {
                readedNow = in.read(buffer, readed, length - readed);
            } catch (IOException e) {
                logger.info("Couldn't read response", e);
                throw e;
            }
            if (readedNow == -1)
                return null;
            readed += readedNow;
        }
//        logger.debug("ResponseReader.readBytes() buffer:\n" + new String(buffer) + "\n--------------");
        printResponse( buffer );
        return buffer;
    }

    protected int readLength()
            throws IOException {
        int length = 0;
        for (int i = 0; i < 4; i++) {
            int c = 0;
            try {
                c = in.read();
            } catch (IOException e) {
                logger.info("Couldn't read response length", e);
                throw e;
            }
            if (c == -1)
                throw new IOException("Couldn't read response length");
            length = (length << 8) + c;
        }
        return length;
    }

}
