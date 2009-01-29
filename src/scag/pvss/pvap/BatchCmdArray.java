package com.eyelinecom.whoisd.pvss.pvap;

import java.util.ArrayList;

public class BatchCmdArray
{
    private class BatchCmdArrayFiller implements PVAPBC.Handler
    {
        BatchCmdArray array;

        public BatchCmdArrayFiller( BatchCmdArray array ) {
            this.array = array;
        }

        public boolean hasSeqNum( int s ) { return true; }
        public void handle( BC_DEL object ) { push(object); }
        public void handle( BC_DEL_RESP object ) { push(object); }
        public void handle( BC_SET object ) { push(object); }
        public void handle( BC_SET_RESP object ) { push(object); }
        public void handle( BC_GET object ) { push(object); }
        public void handle( BC_GET_RESP object ) { push(object); }
        public void handle( BC_INC object ) { push(object); }
        public void handle( BC_INC_RESP object ) { push(object); }
        public void handle( BC_INC_MOD object ) { push(object); }
        public void handle( BC_INC_MOD_RESP object ) { push(object); }

        private void push( BC_CMD obj ) {
            array.push( obj );
        }
    }

    ArrayList< BC_CMD > commands = new ArrayList< BC_CMD >();

    public void clear() {
        commands.clear();
    }

    public void push( BC_CMD cmd ) {
        commands.add( cmd );
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        boolean comma = false;
        for ( BC_CMD cmd : commands ) {
            if ( comma ) sb.append(", ");
            else comma = true;
            sb.append( cmd.toString() );
        }
        sb.append("]");
        return sb.toString();
    }

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        PVAPBC subproto = new PVAPBC();
        int cnt = size() & 0xffff;
        writer.writeShort((short)cnt);
        IBufferWriter subwriter = writer.createNew();
        for ( BC_CMD cmd : commands ) {
            if ( cnt-- == 0 ) break;
            subwriter.clear();
            subproto.encodeCastMessage( cmd, subwriter );
            subwriter.write(writer);
        }
    }

    public void decode( PVAP proto, IBufferReader reader ) throws java.io.IOException
    {
        clear();
        BatchCmdArrayFiller filler = new BatchCmdArrayFiller(this);
        PVAPBC subproto = new PVAPBC(filler);
        int cnt = reader.readShort();
        for ( int i = 0; i < cnt; ++i ) {
            IBufferReader subreader = reader.createNew();
            // subreader.read(reader);
            subproto.decodeMessage( subreader );
        }
    }

    public int size() {
        return commands.size();
    }

    BC_CMD getCommand(int i) {
        return commands.get(i);
    }
}
