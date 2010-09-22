package xml;

import java.io.*;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 17.05.2006
 * Time: 15:42:44
 * To change this template use File | Settings | File Templates.
 */
public class PermanentInputSource extends org.xml.sax.InputSource{
  //byte array input stream as permanent layer
  ByteArrayInputStream bais;
  public PermanentInputSource(String systemId) {
    super(systemId);
  }

  public PermanentInputSource(InputStream byteStream) {
    super(byteStream);
  }

  public PermanentInputSource(Reader characterStream) {
    super(characterStream);
  }

  //read byteStream into permanent layer
  public void setByteStream(InputStream byteStream) {
    read(new InputStreamReader(new BufferedInputStream(byteStream)));
  }

  private void read(Reader toRead) {
    ByteArrayOutputStream bs = new ByteArrayOutputStream();
    int b;
    try {
     while ((b=toRead.read())!=-1) {
        bs.write(b);
     }
    } catch (Exception e) {}
    bais= new ByteArrayInputStream(bs.toByteArray());
  }

  public void setCharacterStream (Reader characterStream)
  {
      
  	read(new BufferedReader(characterStream));
  }

  public InputStream getByteStream ()
  {
    //reset for rereading!!!
    bais.reset();
    return bais;
  }

  public Reader getCharacterStream ()
  {
    bais.reset();
    return new InputStreamReader(bais);
  }

}
