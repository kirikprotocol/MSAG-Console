package ru.novosoft.smsc.perfmon.applet;

import java.awt.*;
import java.util.*;
import java.text.*;
import ru.novosoft.smsc.perfmon.PerfSnap;

public class PerformanceGraph extends Canvas {
  int scale;
  int counter;

  Image offscreen;
  int numGrids = 1;
  int gridsInBlock = 1;
  int pad = 2;
  int bottomSpace = pad;
  int topSpace = pad;
  int graphWidth = 0;

  int pixPerSecond = 2;
  int vertNumGrids = 0;
  int vertGridWidth = 5;
  int vertLightGrid = 4;
  int vertMinuteGrid = 12;

  Dimension prefsz = new Dimension( 80, 200 );

  Vector snaps = new Vector();

  public PerformanceGraph(int scale, int counter, int pixPerSecond, int vertLightGrid, int vertMinuteGrid, PerfSnap snap ) {
    super();
    this.scale = scale;
    this.counter = counter;
    this.pixPerSecond = pixPerSecond;
    this.vertLightGrid = vertLightGrid;
    this.vertMinuteGrid = vertMinuteGrid*pixPerSecond;
    snaps.addElement(new PerfSnap(snap));
  }

  public void addSnap( PerfSnap snap ) {
    if( snaps.size() == graphWidth/pixPerSecond ) {
      snaps.removeElementAt( 0 );
    }
    snaps.addElement( new PerfSnap(snap) );
    repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if( font != null ) {
      FontMetrics fm = getFontMetrics(font);
//      textwidth = 3*pad+fm.charsWidth( scaleString.toCharArray(), 0, scaleString.length() );
      Dimension sz = getSize();
      if( sz.width > 0 && sz.height > 0 ) {
        bottomSpace = 2*pad+fm.getDescent()+fm.getHeight();
        topSpace = pad+fm.getAscent();
        numGrids = (sz.height-bottomSpace-topSpace)/5;
        gridsInBlock = (numGrids+2)/counter;
        graphWidth = sz.width-2*pad;
        vertNumGrids = (graphWidth+vertGridWidth-1)/vertGridWidth;
        if( snaps.size() > graphWidth ) {
          int cnt = snaps.size()-graphWidth;
          for (int i = 0; i < cnt; i++) {
            snaps.removeElementAt( 0 );
          }
        }
      }
    }
    offscreen = null;
    super.invalidate();
  }

  Color background = Color.black;
  Color grid = new Color( 0, 64, 0 );
  Color gridlight = new Color( 0, 128, 0 );
  Color gridmin = new Color( 0, 224, 0 );
  Color shadowBar = new Color( 0, 32, 0 );
  Color green = Color.green;
  Color yellow = Color.yellow;
  Color red = Color.red;

  public void paint(Graphics gg) {
    Dimension size = getSize();
    if( !(size.width > 0 && size.height > 0) ) return;
    if( offscreen == null ) {
      offscreen = createImage( size.width, size.height );
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor( background );
    g.fillRect( 0, 0, size.width, size.height );
    
    int gmax = (counter)*gridsInBlock;

    PerfSnap lastSnap = (PerfSnap)snaps.elementAt( snaps.size()-1 );
    int gridShift = ((lastSnap.sctime%vertGridWidth)*pixPerSecond)%vertGridWidth;

    int posx = size.width-pad-gridShift;
    int posy1 = size.height-bottomSpace-gmax*5;
    int posy2 = size.height-bottomSpace;

    // draw dark grid
    for (int i = 0; i < vertNumGrids; i++ ) {
        g.setColor( grid );
      g.drawLine( posx-i*vertGridWidth, posy1, posx-i*vertGridWidth, posy2 );
    }

    for( int i = 0; i <= gmax; i++ ) {
      int yy = size.height-bottomSpace-i*5;
      if( (i % gridsInBlock) == 0 ) {
        g.setColor( gridlight );
      } else {
        g.setColor( grid );
      }
      g.drawLine( pad, yy, size.width-2*pad, yy );
    }


    //draw lighter grid
    gridShift = ((lastSnap.sctime%(vertGridWidth*vertLightGrid))*pixPerSecond)%(vertGridWidth*vertLightGrid);
    posx = size.width-pad-gridShift;
    int cnt = (graphWidth+vertLightGrid*vertGridWidth-1)/(vertLightGrid*vertGridWidth);
    for( int i = 0; i < cnt; i++ ) {
      g.setColor( gridlight );
      g.drawLine( posx-i*vertGridWidth*vertLightGrid, posy1, posx-i*vertGridWidth*vertLightGrid, posy2 );
    }

    //draw minute grid
    gridShift = (((lastSnap.sctime)%(vertGridWidth*vertMinuteGrid))*pixPerSecond)%(vertGridWidth*vertMinuteGrid);
    posx = size.width-pad-gridShift;
    long sctime = lastSnap.sctime-gridShift/pixPerSecond;
    cnt = (graphWidth+vertMinuteGrid*vertGridWidth-1)/(vertMinuteGrid*vertGridWidth);
    for( int i = 0; i < cnt; i++ ) {
      int xx = posx-(i*vertGridWidth*vertMinuteGrid);
      g.setColor( gridmin );
      g.drawLine( xx, posy1-2, xx, posy2+2 );
      String strSctime = PerfMon.gridFormat.format( new Date( ((long)sctime)*1000 ) );
      sctime-=vertGridWidth*vertMinuteGrid/pixPerSecond;
      g.setColor( green );
      g.drawChars( strSctime.toCharArray(), 0, strSctime.length(), 
                   xx-fm.charsWidth(strSctime.toCharArray(), 0, strSctime.length())/2,
                   size.height-pad-fm.getDescent() );
    }

    //draw graph
    int maxheight = gmax*5;
    int polyx[] = new int[snaps.size()];
    int polyy1[] = new int[snaps.size()];
    int polyy2[] = new int[snaps.size()];
    int polyy3[] = new int[snaps.size()];
    posx = size.width-pad-snaps.size()*pixPerSecond;
    int posy = size.height-bottomSpace;
    for ( int i = 0; i < snaps.size(); i++) {
      PerfSnap snap = (PerfSnap)snaps.elementAt( i );
      polyx[i] = posx;
      polyy1[i] = (int)(posy-(maxheight*(snap.last[1]+snap.last[2]+snap.last[3]))/scale);
      polyy2[i] = (int)(posy-(maxheight*snap.last[2])/scale);
      polyy3[i] = (int)(posy-(maxheight*(snap.last[3]+snap.last[2]))/scale);
      posx += pixPerSecond;
    }
    g.setColor( red );
    g.drawPolyline( polyx, polyy2, snaps.size() );
    g.setColor( yellow );
    g.drawPolyline( polyx, polyy3, snaps.size() );
    g.setColor( green );
    g.drawPolyline( polyx, polyy1, snaps.size() );


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
