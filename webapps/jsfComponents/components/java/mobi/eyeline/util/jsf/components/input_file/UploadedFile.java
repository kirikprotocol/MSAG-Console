package mobi.eyeline.util.jsf.components.input_file;

import java.io.InputStream;

/**
 * author: Aleksandr Khalitov
 */
public interface UploadedFile {

  public String getContentType();

  public long getLength();

  public InputStream getInputStream() throws Exception;

}
