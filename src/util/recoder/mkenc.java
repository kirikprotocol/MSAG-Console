
class mkenc{
	public static void main(String[] args) throws Exception{
		if ( args.length == 0 ) {
			System.out.println("use <encoding>");
			System.exit(0);
		}		
		byte[] b = new byte[256];
		for ( int i=0; i<b.length; ++i ) b[i] = (byte)i;
		String s = new String(b,args[0]);
		System.out.println("unsigned short c2w_"+args[0]+"_table[] = {");
		for ( int i=0; i<255; ++i ){
			System.out.println("  0x"+Integer.toHexString(s.charAt(i))+",");
		}
		System.out.println("  0x"+Integer.toHexString(s.charAt(255)));
		System.out.println("};");
		for ( int i=0; i<b.length; ++i ) b[i] = (byte)'?';
		for ( int i=0; i<255; ++i ) {
			if ( (((int)s.charAt(i))&0x00ff00 ) == 0x0400 ){
				b[((int)s.charAt(i))&0xff] = (byte)i;
			}
		}
		System.out.println("unsigned char w2c_"+args[0]+"_table[] = {");
		for ( int i=0; i<255; ++i ){
			System.out.println("  0x"+Integer.toHexString(((int)b[i])&0x0ff)+",");
		}
		System.out.println("  0x"+Integer.toHexString(((int)b[255])&0x0ff));
		System.out.println("};");
	}
}