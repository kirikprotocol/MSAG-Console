package com.eyelinecom.whoisd.pvss.pvap;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 12:42:05
 */
public class BufferReader implements IBufferReader
{
    byte[] buf;
    int    pos;
    int    size;

    public BufferReader( byte[] data )
    {
        buf = data;
        pos = 0;
        size = data.length;
    }

    public void reset( byte[] data, int offset )
    {
        buf = data;
        this.pos = offset;
        size = data.length;
    }

    
    public short  readTag() throws java.io.IOException 
    {
        if ( pos == size ) {
            return (short)0xffff;
        }
        return readShort();
    }
    
    public byte   readByte() throws java.io.IOException
    {
        return (byte) read();
    }

    public short  readShort() throws java.io.IOException
    {
        int ch1 = read();
        int ch2 = read();
        return (short) ((ch1 << 8) + ch2);
    }

    public int    readInt() throws java.io.IOException
    {
        int ch1 = readShort();
        int ch2 = readShort();
        return (ch1 << 16) + ch2;
    }

    public long   readLong() throws java.io.IOException
    {
        long ch1 = readInt();
        long ch2 = readInt();
        return (ch1 << 32) + (ch2 & 0xffffffffL);
    }

    /*
    public String readString() throws java.io.IOException
    {
    }
     */

    public byte   readByteLV() throws java.io.IOException
    {
        int sz = readShort();
        if ( sz != 1 ) throw new InvalidTagLenException("byte");
        return readByte();
    }
    public short  readShortLV() throws java.io.IOException
    {
        int sz = readShort();
        if ( sz != 2 ) throw new InvalidTagLenException("short");
        return readShort();
    }
    public int    readIntLV() throws java.io.IOException 
    {
        int sz = readShort();
        if ( sz != 4 ) throw new InvalidTagLenException("int");
        return readInt();
    }

    public long   readLongLV() throws java.io.IOException
    {
        int sz = readShort();
        if ( sz != 8 ) throw new InvalidTagLenException("long");
        return readLong();
    }

    public String readStringLV() throws java.io.IOException
    {
        int sz = ((int) readShort()) & 0xFFFF;
        if (sz > 0) {
            if (pos + sz > size) {
                System.err.println("Buffer: Requested string len " + sz + " at " + pos + " out of buffer size" + size);
                throw new java.io.EOFException();
            }
            sz /= 2;
            char data[] = new char[sz];
            for ( int i = 0; i < sz; i++ ) {
                data[i] = (char)readShort();
            }
            return new String(data);
        } else {
            return new String();
        }
    }

    // ---

    protected int read() throws java.io.EOFException
    {
        if (pos >= size) {
            // logger.error("Buffer: Requested byte at " + offset + " of " + size);
            System.err.println("Buffer: requested byte at " + pos + " of " + size );
            throw new java.io.EOFException();
        }
        return ((int) buf[pos++]) & 0xFF;
    }
}
