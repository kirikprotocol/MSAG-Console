package ru.novosoft.smsc.perfmon.applet;

import java.awt.*;
import ru.novosoft.smsc.perfmon.PerfSnap;

public class PerformanceBar extends Canvas {
  int scale;
  int counter;

  String scaleString;
  String bottomString;
  Image offscreen;
  int numGrids = 1;
  int gridsInBlock = 1;
  int pad = 2;
  int bottomSpace = pad;
  int topSpace = pad;
  int graphWidth = 50;
  int barWidth = 34;
  int midBarWidth = 2;
  int textwidth = 0;

  Dimension prefsz = new Dimension( 80, 200 );

  PerfSnap snap;

  public PerformanceBar(int scale, int counter, PerfSnap snap ) {
    super();
    this.scale = scale;
    this.counter = counter;
    this.snap = snap;
    scaleString = String.valueOf( scale );
    bottomString = PerfMon.localeText.getString( "sms.per.s" );
  }

  public synchronized void setSnap( PerfSnap snap ) {
    this.snap = new PerfSnap( snap );
    this.repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if( font != null ) {
      FontMetrics fm = getFontMetrics(font);
      textwidth = 3*pad+fm.charsWidth( scaleString.toCharArray(), 0, scaleString.length() );
      Dimension sz = getSize();
      
      bottomSpace = 2*pad+fm.getDescent()+fm.getHeight();
      topSpace = pad+fm.getAscent();
      numGrids = (sz.height-bottomSpace-topSpace)/5;
      gridsInBlock = (numGrids+2)/counter;

      setSize( textwidth+graphWidth+2*pad, sz.height );
    }
    offscreen = null;
    super.invalidate();
  }

  Color background = Color.black;
  Color grid = new Color( 0, 64, 0 );
  Color gridlight = new Color( 0, 128, 0 );
  Color shadowBar = new Color( 0, 96, 0 );
  Color green = Color.green;
  Color yellow = Color.yellow;
  Color red = Color.red;

  public synchronized void paint(Graphics gg) {
    Dimension size = getSize();
    if( offscreen == null ) {
      offscreen = createImage( size.width, size.height );
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor( background );
    g.fillRect( 0, 0, size.width, size.height );
    
    int gmax = (counter)*gridsInBlock;
    int barposx = pad+textwidth+(graphWidth-barWidth)/2;

    // bar background 
    g.setColor( shadowBar );
    g.fillRect( barposx, size.height-(gmax*5)-bottomSpace, barWidth, gmax*5 );
    
    int maxheight = gmax*5;
    int spent = 0;

    // last error bar
    g.setColor( red );
    int barheight = (int)((maxheight*snap.last[2])/scale);
    g.fillRect( barposx, size.height-bottomSpace-spent-barheight, barWidth, barheight );
    spent += barheight;

    // last rescheduled bar
    g.setColor( yellow );
    barheight = (int)((maxheight*snap.last[3])/scale);
    g.fillRect( barposx, size.height-bottomSpace-spent-barheight, barWidth, barheight );
    spent += barheight;

    // last success bar
    g.setColor( green );
    barheight = (int)((maxheight*snap.last[1])/scale);
    g.fillRect( barposx, size.height-bottomSpace-spent-barheight, barWidth, barheight );
    spent += barheight;
    
    // middle bar background 
    g.setColor( background );
    barposx = pad+textwidth+(graphWidth-midBarWidth)/2;
    g.fillRect( barposx, size.height-(gmax*5)-bottomSpace, midBarWidth, gmax*5 );
    
    int labcnt = 0;

    for( int i = 0; i <= gmax; i++ ) {
      int yy = size.height-bottomSpace-i*5;
      if( (i % gridsInBlock) == 0 ) {
        g.setColor( green );
        String s = String.valueOf( labcnt );
        g.drawChars( s.toCharArray(), 0, s.length(), 
                     size.width-graphWidth-3*pad-fm.charsWidth(s.toCharArray(), 0, s.length()),
                     yy+fm.getDescent() );
        labcnt += scale/counter;
        g.setColor( gridlight );
      } else {
        g.setColor( grid );
      }
      g.drawLine( size.width-graphWidth-pad, yy, size.width-pad, yy );
    }

    g.setColor( green );
    g.drawChars( bottomString.toCharArray(), 0, bottomString.length(),
                 (size.width-fm.charsWidth( bottomString.toCharArray(), 0, bottomString.length()))/2,
                 size.height-pad-fm.getDescent() );

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }


  public Dimension getPreferredSize() {
    return prefsz;
  }

  public void update( Graphics gg ) {
    paint(gg);
  }
}
