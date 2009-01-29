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

    public BufferReader() {
        buf = null;
        pos = 0;
        size = 0;
    }

    public BufferReader( byte[] data )
    {
        reset(data,0,data.length);
    }

    public void reset( byte[] data, int offset, int size )
    {
        buf = new byte[size];
        System.arraycopy( data, offset, buf, 0, size );
        this.pos = 0;
        this.size = size;
    }

    public int getPos() {
        return pos;
    }
    public int readTag() throws java.io.IOException 
    {
        if ( pos == size ) { return -1; }
        int rv = ((int)readShort()) & 0xffff;
        if ( rv == 0xffff ) return -1;
        return rv;
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

    public boolean readBoolLV() throws java.io.IOException
    {
        byte b = readByteLV();
        return ( b != (byte)0 );
    }
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
            return "";
        }
    }

    public String readUTFLV() throws java.io.IOException
    {
        int sz = ((int)readShort()) & 0xffff;
        if ( sz > 0 ) {
            if ( pos+sz > size ) {
                System.err.println("Buffer: Requested string len " + sz + " at " + pos + " out of buffer size" + size);
                throw new java.io.EOFException();
            }
            String rv = new String( buf, pos, sz, "UTF-8" );
            pos += sz;
            return rv;
        } else {
            return "";
        }
    }

    public IBufferReader createNew() throws java.io.IOException {
        BufferReader rv = new BufferReader();
        int sz = readInt();
        rv.reset( buf, pos, sz );
        pos += sz;
        return rv;
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
