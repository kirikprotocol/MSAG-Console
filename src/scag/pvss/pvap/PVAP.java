package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferReader;
// import protogen.framework.InvalidMessageTypeException;
// import protogen.framework.BufferWriter;
// import java.io.IOException;

/**
 * This protocol has header: len (4 oct), seq (4 oct), tag ( 2 oct.), message body
 */
public class PVAP 
{
    static final short tag_PC_DEL=(short)1;
    static final short tag_PC_DEL_RESP=(short)32769;
    static final short tag_PC_SET=(short)2;
    static final short tag_PC_SET_RESP=(short)32770;
    static final short tag_PC_GET=(short)3;
    static final short tag_PC_GET_RESP=(short)32771;
    static final short tag_PC_INC=(short)4;
    static final short tag_PC_INC_RESP=(short)32772;
    static final short tag_PC_INC_MOD=(short)5;
    static final short tag_PC_INC_MOD_RESP=(short)32773;
    static final short tag_PC_PING=(short)6;
    static final short tag_PC_PING_RESP=(short)32774;
    static final short tag_PC_AUTH=(short)10;
    static final short tag_PC_AUTH_RESP=(short)32778;
    static final short tag_PC_BATCH=(short)7;
    static final short tag_PC_BATCH_RESP=(short)32775;

    public static interface Handler {
        public void handle( PC_DEL object );
        public void handle( PC_DEL_RESP object );
        public void handle( PC_SET object );
        public void handle( PC_SET_RESP object );
        public void handle( PC_GET object );
        public void handle( PC_GET_RESP object );
        public void handle( PC_INC object );
        public void handle( PC_INC_RESP object );
        public void handle( PC_INC_MOD object );
        public void handle( PC_INC_MOD_RESP object );
        public void handle( PC_PING object );
        public void handle( PC_PING_RESP object );
        public void handle( PC_AUTH object );
        public void handle( PC_AUTH_RESP object );
        public void handle( PC_BATCH object );
        public void handle( PC_BATCH_RESP object );
    }

    Handler handler;
    public PVAP(Handler handler) {
        this.handler = handler;
    }

    public void assignHandler(Handler newHandler) {
        this.handler = newHandler;
    }

    public void decodeMessage( IBufferReader reader ) throws java.io.IOException
    {
        int seqNum = reader.readInt();
        short tag = reader.readTag();
        switch( tag ) {
        case tag_PC_DEL: {
            PC_DEL object = new PC_DEL();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_DEL_RESP: {
            PC_DEL_RESP object = new PC_DEL_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_SET: {
            PC_SET object = new PC_SET();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_SET_RESP: {
            PC_SET_RESP object = new PC_SET_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_GET: {
            PC_GET object = new PC_GET();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_GET_RESP: {
            PC_GET_RESP object = new PC_GET_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_INC: {
            PC_INC object = new PC_INC();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_INC_RESP: {
            PC_INC_RESP object = new PC_INC_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_INC_MOD: {
            PC_INC_MOD object = new PC_INC_MOD();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_INC_MOD_RESP: {
            PC_INC_MOD_RESP object = new PC_INC_MOD_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_PING: {
            PC_PING object = new PC_PING();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_PING_RESP: {
            PC_PING_RESP object = new PC_PING_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_AUTH: {
            PC_AUTH object = new PC_AUTH();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_AUTH_RESP: {
            PC_AUTH_RESP object = new PC_AUTH_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_BATCH: {
            PC_BATCH object = new PC_BATCH();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        case tag_PC_BATCH_RESP: {
            PC_BATCH_RESP object = new PC_BATCH_RESP();
            object.setSeqNum(seqNum);
            object.decode(reader);
            handler.handle(object);
            break;
        }
        default: throw new InvalidMessageTypeException();
        }
    }

    public byte[] encodeMessage( PC_DEL object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_DEL);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_DEL_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_DEL_RESP);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_SET object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_SET);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_SET_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_SET_RESP);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_GET object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_GET);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_GET_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_GET_RESP);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_INC object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_INC);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_INC_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_INC_RESP);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_INC_MOD object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_INC_MOD);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_INC_MOD_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_INC_MOD_RESP);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_PING object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_PING);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_PING_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_PING_RESP);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_AUTH object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_AUTH);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_AUTH_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_AUTH_RESP);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_BATCH object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_BATCH);
        object.encode(writer);
        return writer.getData();
    }
    public byte[] encodeMessage( PC_BATCH_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_PC_BATCH_RESP);
        object.encode(writer);
        return writer.getData();
    }
}
