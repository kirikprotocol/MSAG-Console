package com.eyelinecom.whoisd.pvss.pvap;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 12:42:05
 */
public class BufferWriter implements IBufferWriter
{
    byte[] data;
    int pos = 0;
    int delta = 256;

    public BufferWriter() {
        data = new byte[delta];
    }

    public BufferWriter( int initSize ) {
        data = new byte[initSize];
    }

    public void writeTag( short tag ) {
        writeShort((short)tag);
    }

    public void writeByte( byte val ) {
        write(val);
    }
    public void writeShort( short val ) {
        write( (val >> 8)&0xff );
        write( val & 0xff );
    }
    public void writeInt( int val ) {
        writeShort( (short)((val >> 16) & 0xffff ) );
        writeShort( (short)(val & 0xffff) );
    }
    public void writeLong( long val ) {
        writeInt( (int) ((val>>32) & 0xffffffffL) );
        writeInt( (int) (val & 0xffffffffL) );
    }

    /*
    public void writeString( String val ) {
    }
     */

    public void writeByteLV( byte val ) {
        writeShort((short)1);
        writeByte(val);
    }
    public void writeShortLV( short val ) {
        writeShort((short)2);
        writeShort(val);
    }
    public void writeIntLV( int val ) {
        writeShort((short)4);
        writeInt(val);
    }
    public void writeLongLV( long val ) {
        writeShort((short)8);
        writeLong(val);
    }
    public void writeStringLV( String val ) {
        int sz = val.length();
        writeShort((short)(sz*2));
        for ( int i = 0; i < sz; ++i ) {
            writeShort((short) val.charAt(i));
        }
    }

    public byte[] getData() {
        byte[] rv = new byte[pos];
        System.arraycopy(data,0,rv,0,pos);
        return rv;
    }

    public String getDump() {
        java.io.ByteArrayOutputStream baos = new java.io.ByteArrayOutputStream();
        java.io.PrintStream out = new java.io.PrintStream(baos, false);
        for ( int i = 0; i < pos; ++i ) {
            out.format( "%02x ", (int)data[i] );
        }
        out.flush();
        return baos.toString();
    }

    // ---

    protected void write( int byteval ) {
        if ( pos == data.length ) {
            byte[] newdata = new byte[ data.length + delta ];
            System.arraycopy( data, 0, newdata, 0, data.length );
            data = newdata;
        }
        data[pos++] = (byte)byteval;
    }

}
