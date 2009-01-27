package com.eyelinecom.whoisd.pvss.pvap;

class SamplePvapHandler implements PVAP.Handler
{
    public void handle( PC_DEL msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_DEL_RESP msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_SET msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_SET_RESP msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_GET msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_GET_RESP msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_INC msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_INC_RESP msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_INC_MOD msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_INC_MOD_RESP msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_PING msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_PING_RESP msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_AUTH msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_AUTH_RESP msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_BATCH msg ) { System.out.println( "got: " + msg.toString()); }
    public void handle( PC_BATCH_RESP msg ) { System.out.println( "got: " + msg.toString()); }
}

public class testPVAP
{
    public static void main( String[] args )
    {
        try {

            SamplePvapHandler handler = new SamplePvapHandler();
            PVAP pvap = new PVAP( handler );

            BufferWriter writer = new BufferWriter();
            PC_GET pcget = new PC_GET();
            pcget.setSeqNum(1);
            pcget.setProfileType((byte)0x01);
            pcget.setAbonentKey(".0.1.79137654079");
            pcget.setVarName("test_varname");

            System.out.println( "pcget is:" + pcget.toString() );

            pvap.encodeMessage(pcget,writer);
            System.out.println( "pcget dump:" + writer.getDump() );

            System.out.println("...transferring packet");
            System.out.println("...processing");

            final byte[] buf = writer.getData();
            BufferReader reader = new BufferReader(buf);
            pvap.decodeMessage(reader);
        } catch ( java.lang.Exception e ) {
            System.err.println( e );
        }
    }
}
