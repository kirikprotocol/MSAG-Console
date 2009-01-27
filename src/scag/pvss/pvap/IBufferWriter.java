package com.eyelinecom.whoisd.pvss.pvap;

public interface IBufferWriter
{
    public void writeTag( short tag );

    public void writeByte( byte val );
    public void writeShort( short val );
    public void writeInt( int val );
    public void writeLong( long val );
    // public void writeString( String val );

    public void writeByteLV( byte val );
    public void writeShortLV( short val );
    public void writeIntLV( int val );
    public void writeLongLV( long val );
    public void writeStringLV( String val );

    public byte[] getData();
    public String getDump();
}
