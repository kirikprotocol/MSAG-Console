package com.eyelinecom.whoisd.pvss.pvap;

/**
 * This protocol has header: len (4 oct), seq (4 oct), tag ( 2 oct.), message body
 */
public class PVAPBC
{
    static final short tag_BC_DEL=(short)1;
    static final short tag_BC_DEL_RESP=(short)32769;
    static final short tag_BC_SET=(short)2;
    static final short tag_BC_SET_RESP=(short)32770;
    static final short tag_BC_GET=(short)3;
    static final short tag_BC_GET_RESP=(short)32771;
    static final short tag_BC_INC=(short)4;
    static final short tag_BC_INC_RESP=(short)32772;
    static final short tag_BC_INC_MOD=(short)5;
    static final short tag_BC_INC_MOD_RESP=(short)32773;

    public static interface Handler {
        public boolean hasSeqNum( int seqNum );
        public void handle( BC_DEL object );
        public void handle( BC_DEL_RESP object );
        public void handle( BC_SET object );
        public void handle( BC_SET_RESP object );
        public void handle( BC_GET object );
        public void handle( BC_GET_RESP object );
        public void handle( BC_INC object );
        public void handle( BC_INC_RESP object );
        public void handle( BC_INC_MOD object );
        public void handle( BC_INC_MOD_RESP object );
    }

    Handler handler;
    public PVAPBC() {
    }

    public PVAPBC(Handler handler) {
        this.handler = handler;
    }

    public void assignHandler(Handler newHandler) {
        this.handler = newHandler;
    }

    public void decodeMessage( IBufferReader reader ) throws java.io.IOException
    {
        int seqNum = reader.readInt();
        if ( ! handler.hasSeqNum(seqNum) ) throw new UnexpectedSeqNumException("seqnum="+seqNum);
        int tag = reader.readTag();
        switch( tag ) {
        case tag_BC_DEL: {
            BC_DEL object = new BC_DEL();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_DEL_RESP: {
            BC_DEL_RESP object = new BC_DEL_RESP();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_SET: {
            BC_SET object = new BC_SET();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_SET_RESP: {
            BC_SET_RESP object = new BC_SET_RESP();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_GET: {
            BC_GET object = new BC_GET();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_GET_RESP: {
            BC_GET_RESP object = new BC_GET_RESP();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_INC: {
            BC_INC object = new BC_INC();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_INC_RESP: {
            BC_INC_RESP object = new BC_INC_RESP();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_INC_MOD: {
            BC_INC_MOD object = new BC_INC_MOD();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        case tag_BC_INC_MOD_RESP: {
            BC_INC_MOD_RESP object = new BC_INC_MOD_RESP();
            object.setSeqNum(seqNum);
            object.decode(this,reader);
            handler.handle(object);
            break;
        }
        default: throw new InvalidMessageTypeException("tag="+tag);
        }
    }

    public byte[] encodeMessage( BC_DEL object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_DEL);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_DEL_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_DEL_RESP);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_SET object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_SET);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_SET_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_SET_RESP);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_GET object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_GET);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_GET_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_GET_RESP);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_INC object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_INC);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_INC_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_INC_RESP);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_INC_MOD object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_INC_MOD);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeMessage( BC_INC_MOD_RESP object, IBufferWriter writer ) throws java.io.IOException
    {
        // BufferWriter writer = new BufferWriter();
        writer.writeInt(object.getSeqNum());
        writer.writeTag(tag_BC_INC_MOD_RESP);
        object.encode(this,writer);
        return writer.getData();
    }

    public byte[] encodeCastMessage( BC_CMD msg, IBufferWriter writer ) throws java.io.IOException
    {
        Class<? extends BC_CMD> c = msg.getClass();
        if ( c == BC_DEL.class ) {
            encodeMessage((BC_DEL)msg, writer);
        } else
        if ( c == BC_DEL_RESP.class ) {
            encodeMessage((BC_DEL_RESP)msg, writer);
        } else
        if ( c == BC_SET.class ) {
            encodeMessage((BC_SET)msg, writer);
        } else
        if ( c == BC_SET_RESP.class ) {
            encodeMessage((BC_SET_RESP)msg, writer);
        } else
        if ( c == BC_GET.class ) {
            encodeMessage((BC_GET)msg, writer);
        } else
        if ( c == BC_GET_RESP.class ) {
            encodeMessage((BC_GET_RESP)msg, writer);
        } else
        if ( c == BC_INC.class ) {
            encodeMessage((BC_INC)msg, writer);
        } else
        if ( c == BC_INC_RESP.class ) {
            encodeMessage((BC_INC_RESP)msg, writer);
        } else
        if ( c == BC_INC_MOD.class ) {
            encodeMessage((BC_INC_MOD)msg, writer);
        } else
        if ( c == BC_INC_MOD_RESP.class ) {
            encodeMessage((BC_INC_MOD_RESP)msg, writer);
        } else
        {
            throw new InvalidMessageTypeException(c.toString());
        }
        return writer.getData();
    }
}
