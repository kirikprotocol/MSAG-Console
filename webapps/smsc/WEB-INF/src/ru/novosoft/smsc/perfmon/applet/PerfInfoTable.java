package ru.novosoft.smsc.perfmon.applet;

import java.awt.*;
import java.util.*;
import ru.novosoft.smsc.perfmon.PerfSnap;


public class PerfInfoTable extends Canvas {
  PerfSnap snap;
  Dimension prefSize;
  Image offscreen;

  int pad  = 1;
  int grid = 2;

  String heads[];
  String rows[];

  public PerfInfoTable( PerfSnap snap ) {
    this.snap = snap;
    prefSize = new Dimension( 100, 0 );

    heads = new String[3];
    heads[0] = PerfMon.localeText.getString( "ptabh.last" );
    heads[1] =  PerfMon.localeText.getString( "ptabh.avg" );
    heads[2] =  PerfMon.localeText.getString( "ptabh.total" );

    rows = new String[4];
    rows[0] = PerfMon.localeText.getString( "ptabr.total" );
    rows[1] = PerfMon.localeText.getString( "ptabr.succ" );
    rows[2] = PerfMon.localeText.getString( "ptabr.err" );
    rows[3] = PerfMon.localeText.getString( "ptabr.resch" );

    invalidate();
  }

  public synchronized void setSnap( PerfSnap snap ) {
    this.snap = new PerfSnap( snap );
    this.repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if( font != null ) {
      FontMetrics fm = getFontMetrics(font);
      prefSize.height = (fm.getHeight()+pad)*5+grid*4;
      Dimension sz = getSize();
      setSize( sz.width, prefSize.height );
    }
    offscreen = null;
    super.invalidate();
  }

  public Dimension getPrefferedSize() {
    return prefSize;
  }

  public Dimension getMinimumSize() {
    return prefSize;
  }


  Color bgColor = SystemColor.control;
  Color shadowColor = SystemColor.controlShadow;
  Color lightShadowColor = SystemColor.controlLtHighlight;
  Color textColor = SystemColor.textText;
  Color totalBgColor = new Color( bgColor.getRed()+16, bgColor.getGreen()+16, bgColor.getBlue()+16 );
  Color successBgColor = new Color( bgColor.getRed(), bgColor.getGreen()+32, bgColor.getBlue() );
  Color errorBgColor = new Color( bgColor.getRed()+32, bgColor.getGreen(), bgColor.getBlue() );
  Color rescheduledBgColor = new Color( bgColor.getRed()+32, bgColor.getGreen()+32, bgColor.getBlue() );


  public synchronized void paint(Graphics gg) {
    Dimension sz = getSize();
    if( offscreen == null ) {
      offscreen = createImage( sz.width, sz.height );
    }
    Graphics g = offscreen.getGraphics();
    g.setColor( bgColor );
    g.fillRect( 0, 0, sz.width, sz.height );

    

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    {
      int y = fm.getHeight()+pad+grid;

      g.setColor( totalBgColor );
      g.fillRect( 0, 1*y, sz.width, fm.getHeight() );

      g.setColor( successBgColor );
      g.fillRect( 0, 2*y, sz.width, fm.getHeight() );

      g.setColor( errorBgColor );
      g.fillRect( 0, 3*y, sz.width, fm.getHeight() );

      g.setColor( rescheduledBgColor );
      g.fillRect( 0, 4*y, sz.width, fm.getHeight() );
    }

    {
      // draw column headings
      g.setColor( textColor );
      int x = sz.width/4;
      for( int i = 1; i <= 3; i++ ) {
        g.drawChars( heads[i-1].toCharArray(), 0, heads[i-1].length(), 
                     i*x-pad+(x-fm.charsWidth(heads[i-1].toCharArray(), 0, heads[i-1].length())),
                     fm.getAscent()+pad );
      }
    }
    {
      // draw row headings
      g.setColor( textColor );
      int y = fm.getHeight()+pad+grid;
      int x = sz.width/4;
      for( int i = 1; i <= 4; i++ ) {
        g.drawChars( rows[i-1].toCharArray(), 0, rows[i-1].length(), 
                     x-fm.charsWidth(rows[i-1].toCharArray(), 0, rows[i-1].length())-pad, 
                     i*y+fm.getAscent()+pad );
      }
    }
    {
      // draw counters
      drawCounters( g, snap.last, sz, fm, 1 );   
      drawCounters( g, snap.avg, sz, fm, 2 );   
      drawCounters( g, snap.total, sz, fm, 3 );   
    }
    {
      // draw grids
      int y = fm.getHeight()+pad+grid;
      int x = sz.width/4;

      g.setColor( lightShadowColor );
      for( int i = 1; i<=4; i++ ) {
        g.drawLine( 0, i*y, sz.width, i*y );
      }
      for( int i = 1; i<=3; i++ ) {
        g.drawLine( i*x+3, 0, i*x+3, sz.height );
      }

      g.setColor( shadowColor );
      for( int i = 1; i<=4; i++ ) {
        g.drawLine( 0, i*y-1, sz.width, i*y-1 );
      }
      for( int i = 1; i<=3; i++ ) {
        g.drawLine( i*x+2, 0, i*x+2, sz.height );
      }
    }

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  void drawCounters( Graphics g, long counters[], Dimension sz, FontMetrics fm, int col ) {
    g.setColor( textColor );
    int y = fm.getHeight()+pad+grid;
    int x = sz.width/4;
    for( int i = 1; i <= 4; i++ ) {
      String counter = String.valueOf( counters[i-1] );
      g.drawChars( counter.toCharArray(), 0, counter.length(), 
                   col*x+(x-fm.charsWidth(counter.toCharArray(), 0, counter.length())-pad), 
                   i*y+fm.getAscent()+pad );
    }
  }

  public void update( Graphics gg ) {
    paint(gg);
  }

}
