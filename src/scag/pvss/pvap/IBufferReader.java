package com.eyelinecom.whoisd.pvss.pvap;

public interface IBufferReader
{
    public short  readTag() throws java.io.IOException;
    
    public byte   readByte() throws java.io.IOException;
    public short  readShort() throws java.io.IOException;
    public int    readInt() throws java.io.IOException;
    public long   readLong() throws java.io.IOException;
    // public String readString() throws java.io.IOException;

    public byte   readByteLV() throws java.io.IOException;
    public short  readShortLV() throws java.io.IOException;
    public int    readIntLV() throws java.io.IOException;
    public long   readLongLV() throws java.io.IOException;
    public String readStringLV() throws java.io.IOException;
}
