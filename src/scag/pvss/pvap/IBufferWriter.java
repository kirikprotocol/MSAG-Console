package com.eyelinecom.whoisd.pvss.pvap;

public interface IBufferWriter
{
    public void clear();

    public void writeTag( int tag );

    public void writeByte( byte val );
    public void writeShort( short val );
    public void writeInt( int val );
    public void writeLong( long val );
    // public void writeString( String val );

    public void writeBoolLV( boolean val );
    public void writeByteLV( byte val );
    public void writeShortLV( short val );
    public void writeIntLV( int val );
    public void writeLongLV( long val );
    public void writeStringLV( String val );
    public void writeUTFLV( String val ) throws java.io.IOException;

    public byte[] getData();
    public int getPos();
    public String getDump();

    public void write( IBufferWriter w );
    public void append( byte[] data, int pos, int size );

    // to have a writer of the same type
    IBufferWriter createNew();
}
