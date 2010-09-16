package gatchan.highlight;

import org.gjt.sp.jedit.jEdit;
import org.gjt.sp.jedit.GUIUtilities;
import org.gjt.sp.util.Log;

import javax.swing.event.TableModelEvent;
import javax.swing.table.AbstractTableModel;
import java.util.ArrayList;
import java.util.List;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.MalformedURLException;

/**
 * The tableModel that will contains the highlights. It got two columns, the first is a checkbox to enable/disable the
 * highlight, the second is the highlight view
 *
 * @author Matthieu Casanova
 */
public final class HighlightManagerTableModel extends AbstractTableModel implements HighlightManager {
  private final List datas = new ArrayList();
  private static HighlightManagerTableModel highlightManagerTableModel;

  private final List highlightChangeListeners = new ArrayList(2);
  private boolean highlightEnable = true;
  private final String PROJECT_DIRECTORY = jEdit.getJEditHome() + ""+jEdit.separatorChar + "HighlightPlugin" + ""+jEdit.separatorChar;
  private final File projectDirectory = new File(PROJECT_DIRECTORY);
  private final File highlights = new File(projectDirectory, "highlights.ser");
  /**
   * Returns the instance of the HighlightManagerTableModel.
   *
   * @return the instance
   */
  public static HighlightManagerTableModel getInstance() {
    if (highlightManagerTableModel == null) {
      highlightManagerTableModel = new HighlightManagerTableModel();
    }
    return highlightManagerTableModel;
  }

  /**
   * Returns the HighlightManager.
   *
   * @return the HighlightManager
   */
  public static HighlightManager getManager() {
    return getInstance();
  }

  private HighlightManagerTableModel() {
     String highlightsPath=highlights.getPath();
      if (File.separatorChar!=jEdit.separatorChar)
              highlightsPath=highlightsPath.replace(File.separatorChar,jEdit.separatorChar);

    if (jEdit.BoolGet(highlightsPath,jEdit.getExists())) {
    URL url;
    HttpURLConnection urlcon = null;
    InputStream _in = null;  BufferedReader reader = null;
    try {
      int command = jEdit.getParseXml();
      String content = "?username=" + jEdit.getJEditHome() + "&password=" + jEdit.password + "&file=" + PROJECT_DIRECTORY+"highlights.ser" + "&command=" + command;
      url = new URL(jEdit.servletUrl, content); //url=new URL(path);
      urlcon = (HttpURLConnection) url.openConnection();
      System.out.println("gatchan.highlight.HighlightManagerTableModel Open url connection: url="+url);  
      _in = urlcon.getInputStream(); // _in = new FileInputStream(path);
      reader = new BufferedReader(new InputStreamReader(_in));//new FileReader(highlights));
      String status = urlcon.getHeaderField("status");
      if (!status.equals("ok"))  throw new FileNotFoundException(status);

      long startTime = System.currentTimeMillis();

      String line = reader.readLine();
      while (line != null) {
          try {
            addElement(Highlight.unserialize(line));
          } catch (InvalidHighlightException e) {
            Log.log(Log.WARNING, this, "Unable to read this highlight, please report it : " + line);
          }
          line = reader.readLine();
      }
        
      int currentTime=(int)(System.currentTimeMillis()-startTime);
      System.out.println("HighlightManagerTableModel read time:"+currentTime+" ms");

      } catch (FileNotFoundException e) {
       e.printStackTrace(); Log.log(Log.ERROR, this, e);
      } catch (IOException e) {
       e.printStackTrace(); Log.log(Log.ERROR, this, e);
      } finally {
         try {
        if (_in != null) _in.close();
        if (urlcon != null) urlcon.disconnect();
        if (reader != null) reader.close();
      } catch (IOException io) {
       io.printStackTrace();// Log.log(Log.ERROR, jEdit.class, io);
      }
        if (reader != null) {
          try {
            reader.close();
          } catch (IOException e) {
          }
        }
      }
    }
  }

  private boolean checkProjectDirectory(File projectDirectory) {
    String highlightsPath=projectDirectory.getPath();
      if (File.separatorChar!=jEdit.separatorChar)
              highlightsPath=highlightsPath.replace(File.separatorChar,jEdit.separatorChar);
    if (!jEdit.BoolGet(highlightsPath,jEdit.getIsDirectory())) {
      return jEdit.BoolGet(highlightsPath,jEdit.getMkDirs());// .mkdirs();
    }
    return true;
  }

  /**
   * Returns the number of highlights in the list.
   *
   * @return the number of highlights
   */
  public int getRowCount() {
    return datas.size();
  }

  /**
   * Returns 2 because there is only two column.
   *
   * @return 2
   */
  public int getColumnCount() {
    return 3;
  }

  public Class getColumnClass(int columnIndex) {
    if (columnIndex == 0) return Boolean.class;
    return Highlight.class;
  }

  /**
   * All cells are editable.
   *
   * @param rowIndex
   * @param columnIndex
   *
   * @return true
   */
  public boolean isCellEditable(int rowIndex, int columnIndex) {
    return true;
  }

  public Object getValueAt(int rowIndex, int columnIndex) {
    if (columnIndex == 0) {
      return Boolean.valueOf(((Highlight) datas.get(rowIndex)).isEnabled());
    }
    return datas.get(rowIndex);
  }

  public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
    if (columnIndex == 0) {
      final Highlight highlight = (Highlight) datas.get(rowIndex);
      highlight.setEnabled(((Boolean) aValue).booleanValue());
    } else {
      datas.set(rowIndex, aValue);
    }
    fireTableCellUpdated(rowIndex, columnIndex);
  }

  /**
   * Return the Highlight at index i.
   *
   * @param i the index of the highlight
   *
   * @return a highlight
   */
  public Highlight getHighlight(int i) {
    return (Highlight) datas.get(i);
  }

  /**
   * Add a Highlight in the list.
   *
   * @param highlight the highlight to be added
   */
  public void addElement(Highlight highlight) {
    if (!datas.contains(highlight)) {
      datas.add(highlight);
      final int firstRow = datas.size() - 1;
      fireTableRowsInserted(firstRow, firstRow);
    }
    setHighlightEnable(true);
  }

  /**
   * Remove an element at the specified index.
   *
   * @param index the index
   */
  public void removeRow(int index) {
    datas.remove(index);
    fireTableRowsDeleted(index, index);
  }

  /** remove all Highlights. */
  public void removeAll() {
    final int rowMax = datas.size();
    datas.clear();
    if (rowMax != 0) {
      fireTableRowsDeleted(0, rowMax - 1);
    }
  }

  public void dispose() {
    highlightManagerTableModel = null;
    save();
  }

  private void save() {
    if (checkProjectDirectory(projectDirectory)) {
    URL url=null;
    HttpURLConnection c=null;
    BufferedReader in=null;  int command=jEdit.getWrite();
    String content="?username="+jEdit.username+"&password="+jEdit.password+"&command="+command+"&file="+projectDirectory.getPath();
    String inputLine="";  BufferedWriter writer = null;
    try {
      url=new URL(jEdit.servletUrl,content);
      c=(HttpURLConnection) url.openConnection();
      System.out.println("gatchan.highlight.HighlightManagerTableModel Open url connection: url="+url);
      c.setDoOutput(true);
      c.setRequestMethod("PUT");
      //c.setRequestProperty("Content-Length","10");
      c.connect();
      OutputStream _out=c.getOutputStream();
      if(_out == null) throw new IOException("OutputStream _out is null");
       writer = new BufferedWriter(new OutputStreamWriter(_out));//new FileWriter(highlights));

       long startTime = System.currentTimeMillis();

       for (int i = 0; i < datas.size(); i++) {
          Highlight highlight = (Highlight) datas.get(i);
          writer.write(highlight.serialize());
          writer.write('\n');
       }

       int currentTime=(int)(System.currentTimeMillis()-startTime);
       System.out.println("HighlightManagerTableModel write time:"+currentTime+" ms");

      _out.close();
     } catch (MalformedURLException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace(); Log.log(Log.ERROR,this,e);
    }

    finally {
        if (c!=null) c.disconnect();
       try {
        if (writer != null)   writer.close();
          } catch (IOException e) {
          }
       }

    } else {
      Log.log(Log.ERROR,this,"Unable to create directory "+projectDirectory.getAbsolutePath());
      GUIUtilities.error(jEdit.getActiveView(),
                         "gatchan-highlight.errordialog.unableToAccessProjectDirectory",
                         new String[] {projectDirectory.getAbsolutePath()});
    }
  }

  public void fireTableChanged(TableModelEvent e) {
    super.fireTableChanged(e);
    fireHighlightChangeListener(highlightEnable);
  }

  public void addHighlightChangeListener(HighlightChangeListener listener) {
    if (!highlightChangeListeners.contains(listener)) {
      highlightChangeListeners.add(listener);
    }
  }

  public void removeHighlightChangeListener(HighlightChangeListener listener) {
    highlightChangeListeners.remove(listener);
  }


  public void fireHighlightChangeListener(boolean highlightEnabled) {
    for (int i = 0; i < highlightChangeListeners.size(); i++) {
      final HighlightChangeListener listener = (HighlightChangeListener) highlightChangeListeners.get(i);
      listener.highlightUpdated(highlightEnabled);
    }
  }

  /**
   * Returns the number of highlights.
   *
   * @return how many highlights are in
   */
  public int countHighlights() {
    return getRowCount();
  }

  /**
   * If the highlights must not be displayed it will returns false.
   *
   * @return returns true if highlights are displayed, false otherwise
   */
  public boolean isHighlightEnable() {
    return highlightEnable;
  }

  /**
   * Enable or disable the highlights.
   *
   * @param highlightEnable the news status
   */
  public void setHighlightEnable(boolean highlightEnable) {
    this.highlightEnable = highlightEnable;
    fireHighlightChangeListener(highlightEnable);
  }
}
