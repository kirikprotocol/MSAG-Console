
class mktr{
	public static void main(String[] args) throws Exception{
		if ( args.length == 0 ) {
			System.out.println("use <encoding>");
			System.exit(0);
		}		
		byte[] b = new byte[256];
		for ( int i=0; i<b.length; ++i ) b[i] = (byte)i;
		String s = new String(b,args[0]);
		System.out.println("unsigned short c2t_"+args[0]+"_table[] = {");
		for ( int i=0; i<192; ++i ){
			System.out.println("(const unsigned char*)"+"\"\""+",");
		}
		for ( int i=192; i<255; ++i ){
			System.out.println("(const unsigned char*)"+"\"\""+",//"+s.charAt(i)+" "+i);
		}
		System.out.println("(const unsigned char*)"+"\"\""+"//"+s.charAt(255));
		System.out.println("};");
	}
}