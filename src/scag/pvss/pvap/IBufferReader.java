package com.eyelinecom.whoisd.pvss.pvap;

public interface IBufferReader
{
    // for debugging
    public int  getPos();
    public int  readTag() throws java.io.IOException;
    
    public byte   readByte() throws java.io.IOException;
    public short  readShort() throws java.io.IOException;
    public int    readInt() throws java.io.IOException;
    public long   readLong() throws java.io.IOException;
    // public String readString() throws java.io.IOException;

    public boolean readBoolLV() throws java.io.IOException;
    public byte   readByteLV() throws java.io.IOException;
    public short  readShortLV() throws java.io.IOException;
    public int    readIntLV() throws java.io.IOException;
    public long   readLongLV() throws java.io.IOException;
    public String readStringLV() throws java.io.IOException;
    public String readUTFLV() throws java.io.IOException;

    /// create a new subreader
    public IBufferReader createNew() throws java.io.IOException;
    // public void read( IBufferReader );
}
