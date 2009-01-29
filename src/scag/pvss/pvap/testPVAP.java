package com.eyelinecom.whoisd.pvss.pvap;

class SamplePvapHandler implements PVAP.Handler
{
    public boolean hasSeqNum( int sn ) { return true; }
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
    public static void main( String[] args ) throws java.lang.Exception
    {
        // try {

            SamplePvapHandler handler = new SamplePvapHandler();
            PVAP pvap = new PVAP( handler );

            BufferWriter writer = new BufferWriter();
            {
                PC_BATCH pcbatch = new PC_BATCH();
                pcbatch.setSeqNum(1);
                pcbatch.setProfileType((byte)0x01);
                pcbatch.setAbonentKey(".0.1.79137654079");
                pcbatch.setBatchMode( true );
                BatchCmdArray cmds = new BatchCmdArray();

                {
                    BC_GET bcget = new BC_GET();
                    bcget.setSeqNum(2);
                    bcget.setVarName("test_varname");
                    cmds.push( bcget );

                    BC_SET bcset = new BC_SET();
                    bcset.setSeqNum(3);
                    bcset.setVarName("test_setvar");
                    bcset.setValueType((byte)0x2);
                    bcset.setTimePolicy((byte)0x1);
                    bcset.setFinalDate(100);
                    bcset.setLifeTime(200);
                    bcset.setStringValue( "хелло, ворлд" );
                    cmds.push( bcset );
                }
                pcbatch.setBatchContent( cmds );
                System.out.println( "pcbatch is:" + pcbatch.toString() );

                pvap.encodeMessage(pcbatch,writer);
                System.out.println( "writer dump:" + writer.getDump() );
            }

            System.out.println("...transferring packet");
            System.out.println("...processing");

            {
                final byte[] buf = writer.getData();
                BufferReader reader = new BufferReader(buf);
                pvap.decodeMessage(reader);
            }

        // } catch ( java.lang.Exception e ) {
        //    System.err.println( e );
        // }
    }
}
