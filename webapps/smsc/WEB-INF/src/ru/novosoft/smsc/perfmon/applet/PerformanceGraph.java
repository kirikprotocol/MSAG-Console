package ru.novosoft.smsc.perfmon.applet;

import java.awt.*;
import java.util.*;
import java.text.*;
import ru.novosoft.smsc.perfmon.PerfSnap;

public class PerformanceGraph extends Canvas {
  int scale;
  int counter;

  Image offscreen;
  int pixInGrid = 5;
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

  private static final int MAX_SNAP_LIST_SIZE = 4096;
  Vector snaps = new Vector(MAX_SNAP_LIST_SIZE);


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
    if( snaps.size() == MAX_SNAP_LIST_SIZE ) {
      snaps.removeElementAt( 0 );
    }
    snaps.addElement( new PerfSnap(snap) );
    repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if( font != null ) {
      FontMetrics fm = getFontMetrics(font);
      Dimension sz = getSize();
      if( sz.width > 0 && sz.height > 0 ) {
        bottomSpace = 2*pad+fm.getDescent()+fm.getHeight();
        topSpace = pad+fm.getAscent();
        numGrids = (sz.height-bottomSpace-topSpace)/pixInGrid;
        gridsInBlock = (numGrids+2)/counter;
        graphWidth = sz.width-2*pad;
        vertNumGrids = (graphWidth+vertGridWidth-1)/vertGridWidth;
      }
    }
    offscreen = null;
    super.invalidate();
  }

  Color colorBackground = Color.black;
  Color colorGrid = new Color( 0, 64, 0 );
  Color colorGridLight = new Color( 0, 128, 0 );
  Color colorGridMin = new Color( 0, 224, 0 );
  Color colorGraphSucc = Color.green;
  Color colorGraphResch = Color.yellow;
  Color colorGraphErr = Color.red;
  Color colorGraphRecv = Color.blue;
  Color colorGraphTotal = Color.white;

  public void paint(Graphics gg) {
    Dimension size = getSize();
    if( !(size.width > 0 && size.height > 0) ) return;
    if( offscreen == null ) {
      offscreen = createImage( size.width, size.height );
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor( colorBackground );
    g.fillRect( 0, 0, size.width, size.height );
    
    int gmax = (counter)*gridsInBlock;

    PerfSnap lastSnap = (PerfSnap)snaps.elementAt( snaps.size()-1 );
    int gridShift = ((lastSnap.sctime%vertGridWidth)*pixPerSecond)%vertGridWidth;

    int posx = size.width-pad-gridShift;
    int posy1 = size.height-bottomSpace-gmax*pixInGrid;
    int posy2 = size.height-bottomSpace;

    // draw dark grid
    // vertical grids
    g.setColor( colorGrid );
    for (int i = 0; i < vertNumGrids; i++ ) {
      g.drawLine( posx-i*vertGridWidth, posy1, posx-i*vertGridWidth, posy2 );
    }

    // horizontal grids
    for( int i = 0; i <= gmax; i++ ) {
      int yy = size.height-bottomSpace-i*pixInGrid;
      if( (i % gridsInBlock) == 0 ) {
        g.setColor( colorGridLight );
      } else {
        g.setColor( colorGrid );
      }
      g.drawLine( pad, yy, size.width-2*pad, yy );
    }


    //draw lighter grid
    gridShift = ((lastSnap.sctime%(vertGridWidth*vertLightGrid))*pixPerSecond)%(vertGridWidth*vertLightGrid);
    posx = size.width-pad-gridShift;
    int cnt = (graphWidth+vertLightGrid*vertGridWidth-1)/(vertLightGrid*vertGridWidth);
    g.setColor( colorGridLight );
    for( int i = 0; i < cnt; i++ ) {
      g.drawLine( posx-i*vertGridWidth*vertLightGrid, posy1, posx-i*vertGridWidth*vertLightGrid, posy2 );
    }

    //draw minute grid
    gridShift = (((lastSnap.sctime)%(vertGridWidth*vertMinuteGrid))*pixPerSecond)%(vertGridWidth*vertMinuteGrid);
    posx = size.width-pad-gridShift;
    long sctime = lastSnap.sctime-gridShift/pixPerSecond;
    cnt = (graphWidth+vertMinuteGrid*vertGridWidth-1)/(vertMinuteGrid*vertGridWidth);
    g.setColor( colorGridMin );
    for( int i = 0; i < cnt; i++ ) {
      int xx = posx-(i*vertGridWidth*vertMinuteGrid);
      g.drawLine( xx, posy1-2, xx, posy2+2 );
      String strSctime = PerfMon.gridFormat.format( new Date( ((long)sctime)*1000 ) );
      sctime-=vertGridWidth*vertMinuteGrid/pixPerSecond;
      g.drawChars( strSctime.toCharArray(), 0, strSctime.length(),
                   xx-fm.charsWidth(strSctime.toCharArray(), 0, strSctime.length())/2,
                   size.height-pad-fm.getDescent() );
    }

    //draw graph
    // prepare arrays for polyline
    if( snaps.size() > 1 ) {
        int maxheight = gmax*5;
        int viewableGraph = graphWidth/pixPerSecond;
        System.out.println("szw="+size.width+" grw="+graphWidth+" pix="+pixPerSecond+" vig="+viewableGraph);
        int polyx[] = new int[viewableGraph];
        int polySucc[] = new int[viewableGraph];
        int polyErr[] = new int[viewableGraph];
        int polyResch[] = new int[viewableGraph];
        int polyRecv[] = new int[viewableGraph];
        int polyTotal[] = new int[viewableGraph];

        posx = size.width-pad;
        int posy = size.height-bottomSpace;
        int idx = snaps.size();
        int cntg = 0;
        for ( int i = 0; i < viewableGraph && idx > 0; i++, cntg++) {
          PerfSnap snap = (PerfSnap)snaps.elementAt( --idx );
          polyx[i] = posx;
          polySucc[i] = (int)(posy-(maxheight*snap.last[PerfSnap.IDX_SUCCESS])/scale);
          polyErr[i] = (int)(posy-(maxheight*snap.last[PerfSnap.IDX_ERROR])/scale);
          polyResch[i] = (int)(posy-(maxheight*snap.last[PerfSnap.IDX_RETRY])/scale);
          polyRecv[i] = (int)(posy-(maxheight*snap.last[PerfSnap.IDX_RECEIVED])/scale);
          polyTotal[i] = (int)(posy-
                  (maxheight*
                    ( snap.last[PerfSnap.IDX_SUCCESS]
                     +snap.last[PerfSnap.IDX_ERROR]
                     +snap.last[PerfSnap.IDX_RETRY]
                     +snap.last[PerfSnap.IDX_RECEIVED]
                    )
                  )/scale);
          posx -= pixPerSecond;
        }
        // draw poly lines
        g.setColor( colorGraphRecv );
        g.drawPolyline( polyx, polyRecv, cntg );
        g.setColor( colorGraphResch );
        g.drawPolyline( polyx, polyResch, cntg );
        g.setColor( colorGraphErr );
        g.drawPolyline( polyx, polyErr, cntg );
        g.setColor( colorGraphSucc );
        g.drawPolyline( polyx, polySucc, cntg );
        g.setColor( colorGraphTotal );
        g.drawPolyline( polyx, polyTotal, cntg );
        {
          // redraw bottom grid line over graph to hide zero graphs
          int yy = size.height-bottomSpace;
          g.setColor( colorGridLight );
          g.drawLine( pad, yy, size.width-2*pad, yy );
        }
    }

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
