package ru.novosoft.smsc.web.applets.topmon;

import ru.novosoft.smsc.admin.topmon.ErrorSnap;
import ru.novosoft.smsc.admin.topmon.SmeSnap;
import ru.novosoft.smsc.admin.topmon.TopSnap;

import java.awt.*;
import java.awt.event.*;
import java.util.Comparator;
import java.util.ResourceBundle;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 15:16:54
 */
class SmeTopGraph extends Canvas implements MouseListener, MouseMotionListener, KeyListener {
  private static final int pad = 1;
  private static final Color tipBackground = SystemColor.control;
  private static final Color tipTextColor = SystemColor.controlText;
  private static final Color colorBackground = Color.black;
  private static final Color colorHiBackground = new Color(colorBackground.getRed(), colorBackground.getGreen() + 64, colorBackground.getBlue());
  private static final Color headColor = Color.lightGray;
  private static final Color graphColor = Color.green;
  private static final Color graphHiGridColor = new Color(colorBackground.getRed(), colorBackground.getGreen() + 128, colorBackground.getBlue());
  private static final Color graphGridColor = new Color(colorBackground.getRed(), colorBackground.getGreen() + 80, colorBackground.getBlue());
  private static final Color graphBarBGColor = new Color(colorBackground.getRed(), colorBackground.getGreen() + 96, colorBackground.getBlue());
  private static final Color smeHiColor = Color.white;
  private static final Color smeColor = Color.lightGray;
  private static final Color columnsColor[] = {
      new Color(128, 128, 224), // accepted
      new Color(224, 224, 224), // rejected
      new Color(128, 224, 128), // delivered
      new Color(128, 224, 224), // rescheduled
      new Color(224, 184, 128), // temp error
      new Color(224, 128, 128) // perm error
  };
  private static final Color colorGraphDeliver = Color.green;
  private static final Color colorGraphDeliverErr = Color.red;
  private static final Color colorGraphTempErr = Color.orange;
  private static final Color colorGraphRetry = Color.cyan;
  private static final Color colorGraphSubmit = Color.blue;
  private static final Color colorGraphSubmitErr = Color.white;

  private static final int scrollerWidth = 8;
  private int scrollPageRows = 10;
  private int scrollPos = 0;

  private Image offscreen;
  private int bottomSpace = pad;
  private int topSpace = pad;
  private int counterWidth = 0;
  private int smeNameWidth = 0;
  private int maxSpeed = 50;
  private int rowHeight = SmeSnap.COUNTERS_NUM * 2;
  private final ResourceBundle localeText;
  private final ResourceBundle messagesText;
  private final int graphScale;
  private final int graphGrid;
  private final int graphHiGrid;
  private final int graphHead;
  private int headerHeight;
  private int errListWidth;
  private int smeListStart;
  private int smeGraphStart;
  private final int separatorWidth = 8;

  private int mouseX;
  private int mouseY;

  private static final int SORT_DESC = 1;
  private static final int SORT_ASC = 2;
  private int errSortId = -1;
  private int errSortState;
  private int smeSortId = -1;
  private int smeSortState;
  private Comparator<SmeSnap> smeComparator = null;
  private Comparator<ErrorSnap> errComparator = null;

  private int split = -1;
  private final int shiftStep = 10;

  private TopSnap snap;
  private final SnapHistory snapHistory;

  private final Font graphFont;
  private int graphTextWidth;
  private final int barSeparator = 4;

  public SmeTopGraph(TopSnap snap, int maxSpeed, int graphScale, int graphGrid, int graphHiGrid, int graphHead, ResourceBundle localeText, ResourceBundle messagesText, SnapHistory snapHistory) {
    super();
    this.maxSpeed = maxSpeed;
    this.localeText = localeText;
    this.graphScale = graphScale;
    this.graphGrid = graphGrid;
    this.graphHiGrid = graphHiGrid;
    this.graphHead = graphHead;
    this.messagesText = messagesText;
    this.snapHistory = snapHistory;
    addMouseListener(this);
    addMouseMotionListener(this);
    addKeyListener(this);
    graphFont = new Font("dialog", Font.PLAIN, 10);
    setSnap(snap);

  }

  public void setSnap(TopSnap snap) {
    snapHistory.addSnap(snap);
    this.snap = new TopSnap(snap);
    if (smeComparator != null) this.snap.sortSme(smeComparator);
    if (errComparator != null) this.snap.sortErr(errComparator);
    repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if (font != null) {
      FontMetrics fm = getFontMetrics(font);
      FontMetrics fmg = getFontMetrics(graphFont);
      Dimension sz = getSize();
      if (sz.width > 0 && sz.height > 0) {
        if (fm.getHeight() > SmeSnap.COUNTERS_NUM * 2) {
          rowHeight = fm.getHeight() + 1;
        } else {
          rowHeight = SmeSnap.COUNTERS_NUM * 2;
        }
        bottomSpace = 2 * pad + fm.getDescent() + fm.getHeight();
        topSpace = pad + fm.getAscent();
        headerHeight = fm.getHeight() + pad + 3;
        counterWidth = fm.stringWidth("0000");
        smeNameWidth = fm.stringWidth("WWWWWWWWWWWWWWWW");
        errListWidth = (counterWidth + pad) * 3;
        smeListStart = errListWidth + pad + separatorWidth;
        smeGraphStart = smeListStart + pad + smeNameWidth + (counterWidth + pad) * (SmeSnap.COUNTERS_NUM * 2) + pad + pad;
        prefsz = new Dimension(smeGraphStart + 1 + maxSpeed * graphScale + pad,
            headerHeight + rowHeight * 6 + pad);

        graphTextWidth = fmg.stringWidth("0000");
        scrollPageRows = (sz.height - split) / rowHeight - 1;
      }
    }
    offscreen = null;
    super.invalidate();
  }

  public void paint(Graphics gg) {
    Dimension size = getSize();
    if (!(size.width > 0 && size.height > 0)) return;
    if (offscreen == null) {
      offscreen = createImage(size.width, size.height);
    }
    Graphics g = offscreen.getGraphics();
    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);
    int fh = fm.getHeight();
    g.setColor(colorBackground);
    g.fillRect(0, 0, size.width, size.height);

    g.setClip(0, 0, size.width, size.height - split);
    drawSortState(g, fm);
    // drawing heading
    g.setColor(SystemColor.control);
    g.drawLine(pad, pad + fh + 1, size.width - pad, pad + fh + 1);

    int x = 0;
    g.setColor(headColor);
    int hpos = pad + fm.getAscent();
    drawCounterHead(g, localeText.getString("snh.errcode"), x + pad, hpos, fm);
    x += counterWidth + pad;
    drawCounterHead(g, localeText.getString("snh.errcnt"), x + pad, hpos, fm);
    x += counterWidth + pad;
    drawCounterHead(g, localeText.getString("snh.erravg"), x + pad, hpos, fm);
    x += counterWidth + pad;

    x = smeListStart;
    g.setColor(smeColor);
    g.drawString(localeText.getString("snh.smename"), x + pad, hpos);
    x += smeNameWidth;
    String avgstr = localeText.getString("snh.count.avg");
    for (int i = 0; i < SmeSnap.COUNTERS_NUM; i++) {
      g.setColor(columnsColor[i]);
      drawCounterHead(g, localeText.getString("snh.count." + i), x + pad, hpos, fm);
      x += pad + counterWidth;
      drawCounterHead(g, avgstr, x + pad, hpos, fm);
      x += pad + counterWidth;
    }
    // draw graph scale
    x = smeGraphStart + 1;
    g.setColor(graphColor);
    for (int i = graphHead; ; i += graphHead) {
      int pos = x + i * graphScale;
      if (pos >= size.width - pad) break;
      String s = String.valueOf(i);
      g.drawString(s, pos - (fm.stringWidth(s) / 2), hpos);
    }

    g.setColor(headColor);
    g.drawLine(pad, pad + fh + 1, size.width - pad, pad + fh + 1);

    g.setClip(0, 0, size.width - scrollerWidth, size.height - split);
    // draw counters
    int y = headerHeight;
    x = 0;
    for (int i = 0; i < snap.errCount; i++) {
      if ((i % 2) == 0) {
        g.setColor(colorHiBackground);
        g.fillRect(x + pad, y, (counterWidth + pad) * 3 - pad, rowHeight);
      }
      drawErrSnap(g, i, x, y, fm);
      y += rowHeight;
    }
    y = pad + fh + 3;
    x = smeListStart;
    for (int i = scrollPos; i < snap.smeCount && y < size.height - split; i++) {
      if ((i % 2) == 0) {
        g.setColor(colorHiBackground);
        g.fillRect(x + pad, y, size.width - x - 2 * pad, rowHeight);
      }
      drawSmeSnap(g, i, x, y, size, fm);
      y += rowHeight;
    }
    if (split > separatorWidth + fh + pad) {
      g.setClip(0, size.height - split + separatorWidth, size.width, split - separatorWidth);
      drawGraph(g, size);
    }
    g.setClip(0, 0, size.width, size.height);
    drawScroller(g, size);
    drawSeparator(g, size);
    drawErrTip(g, fm);
    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  void drawSeparator(Graphics g, Dimension size) {
    g.setColor(SystemColor.control);
    g.fillRect(errListWidth + pad, 0, separatorWidth, size.height);
    if (split > 0) g.fillRect(0, size.height - split, size.width, separatorWidth);
    int sl = (separatorWidth - 2) / 2;
    int vp = errListWidth + pad + sl;
    int hp = size.height - split + sl;
    g.setColor(SystemColor.controlHighlight);
    g.drawLine(vp + 1, pad + 1, vp + 1, size.height - pad);
    if (split > 0) g.drawLine(pad + 1, hp + 1, size.width - pad - 1, hp + 1);
    g.setColor(SystemColor.controlShadow);
    g.drawLine(vp, pad, vp, size.height - pad - 1);
    if (split > 0) g.drawLine(pad, hp, size.width - pad, hp);
  }

  void drawScroller(Graphics g, Dimension size) {
    if(snap.smeCount == 0) {
      return;
    }
    g.setColor(smeColor);
    g.drawLine(size.width - scrollerWidth, headerHeight, size.width - scrollerWidth, size.height - split);
    int drowerSize = (scrollPageRows * (size.height - headerHeight - split)) / snap.smeCount;
    int drowerPos = (scrollPos * (size.height - headerHeight - split)) / snap.smeCount;
    g.fillRect(size.width - scrollerWidth + 2, headerHeight + drowerPos, 4, drowerSize);
  }

  void drawErrSnap(Graphics g, int i, int x, int y, FontMetrics fm) {
    int hpos = y + fm.getAscent();
    g.setColor(smeColor);
    ErrorSnap es = snap.errSnap[i];
    drawCounter(g, es.code, x + pad, hpos, fm);
    x += pad + counterWidth;
    drawCounter(g, es.count, x + pad, hpos, fm);
    x += pad + counterWidth;
    drawCounter(g, es.avgCount, x + pad, hpos, fm);
    x += pad + counterWidth;
  }

  void drawSmeSnap(Graphics g, int i, int x, int y, Dimension size, FontMetrics fm) {
    int hpos = y + fm.getAscent();
    SmeSnap ss = snap.smeSnap[i];
    if (snapHistory.getCurrentSme() != null && snapHistory.getCurrentSme().equals(ss.smeId))
      g.setColor(smeHiColor);
    else
      g.setColor(smeColor);
    g.drawString(ss.smeId, x + pad, hpos);
    x += pad + smeNameWidth;
    for (int k = 0; k < SmeSnap.COUNTERS_NUM; k++) {
      g.setColor(columnsColor[k]);
      drawCounter(g, ss.speed[k], x + pad, hpos, fm);
      x += pad + counterWidth;
      drawCounter(g, ss.avgSpeed[k], x + pad, hpos, fm);
      x += pad + counterWidth;
    }
    x += pad + pad;
    for (int k = 0; k < SmeSnap.COUNTERS_NUM; k++) {
      if (ss.speed[k] > 0) {
        g.setColor(columnsColor[k]);
        g.fillRect(x + 1, y + 1 + k * 2, ss.speed[k] * graphScale, 2);
      }
    }
    g.setColor(graphColor);
    g.drawLine(x + pad, y, x + pad, y + rowHeight);
    x++;
    for (int k = graphGrid; ; k += graphGrid) {
      int pos = x + (k * graphScale);
      if (pos >= (size.width - pad)) break;
      if ((k % graphHiGrid) == 0) {
        g.setColor(graphHiGridColor);
      } else {
        g.setColor(graphGridColor);
      }
      g.drawLine(pos, y, pos, y + rowHeight);
    }
  }

  void drawErrTip(Graphics g, FontMetrics fm) {
    if (mouseX < errListWidth && mouseY > headerHeight) {
      int idx = (mouseY - (headerHeight)) / (rowHeight);
      if (idx < snap.errCount) {
        String errText;
        try {
          errText = messagesText.getString("smsc.errcode." + snap.errSnap[idx].code);
        } catch (Throwable e) {
          e.printStackTrace();
          System.out.println("cant get smsc.errcode." + snap.errSnap[idx].code);
/*          for( Enumeration enum = messagesText.getKeys(); enum.hasMoreElements(); ) {
            System.out.println(enum.nextElement());
          }*/
          errText = messagesText.getString("smsc.errcode.unknown");
        }
        g.setColor(tipBackground);
        g.fillRect(mouseX, mouseY - (fm.getHeight() + 4 * pad + 2), fm.stringWidth(errText) + 4 * pad + 2, fm.getHeight() + 4 * pad + 2);
        g.setColor(tipTextColor);
        g.drawRect(mouseX + pad, mouseY - (fm.getHeight() + 3 * pad + 2), fm.stringWidth(errText) + 2 * pad + 1, fm.getHeight() + 2 * pad + 1);
        g.drawString(errText, mouseX + 1 + 2 * pad, mouseY - 1 - 2 * pad - fm.getDescent());
      }
    }
  }

  void drawCounter(Graphics g, int cnt, int x, int y, FontMetrics fm) {
    String cc = String.valueOf(cnt);
    g.drawString(cc, x + counterWidth - fm.stringWidth(cc), y);
  }

  void drawCounterHead(Graphics g, String s, int x, int y, FontMetrics fm) {
    g.drawString(s, x + counterWidth - fm.stringWidth(s), y);
  }

  void drawSortState(Graphics g, FontMetrics fm) {
    if (errSortId != -1) {
      drawSortGradient(g, pad + errSortId * (counterWidth + pad), counterWidth, fm, errSortState == SORT_ASC);
    }
    if (smeSortId == 0) {
      drawSortGradient(g, smeListStart, smeNameWidth, fm, smeSortState == SORT_ASC);
    } else if (smeSortId > 0) {
      drawSortGradient(g, smeListStart + smeNameWidth + pad + (counterWidth + pad) * (smeSortId - 1), counterWidth, fm, smeSortState == SORT_ASC);
    }
  }

  private final static int gradientStep = 10;

  void drawSortGradient(Graphics g, int x, int len, FontMetrics fm, boolean ascent) {
    int fh = fm.getHeight();
    int red = colorBackground.getRed();
    int green = colorBackground.getGreen();
    int blue = colorBackground.getBlue();
    if (ascent) {
      for (int i = pad; i <= fh; i++) {
        g.setColor(new Color(red + i * gradientStep, green + i * gradientStep, blue + i * gradientStep));
        g.drawLine(x, i, x + len, i);
      }
    } else {
      for (int i = pad; i <= fh; i++) {
        g.setColor(new Color(red + i * gradientStep, green + i * gradientStep, blue + i * gradientStep));
        g.drawLine(x, fh + pad - i, x + len, fh + pad - i);
      }
    }
  }

  void drawGraph(Graphics g, Dimension size) {
    g.setColor(graphHiGridColor);
    FontMetrics fm = getFontMetrics(graphFont);
    int fh = fm.getHeight();
    int height = split - separatorWidth - pad;
    int top = size.height - height;
    int y = size.height - pad - fh - pad;
    int barwidth = (errListWidth - graphTextWidth - 3 * barSeparator - 2 * pad) / 2;
    g.setFont(graphFont);
    // draw bars
    int barx = pad + graphTextWidth + barSeparator;
    g.setColor(graphBarBGColor);
    g.fillRect(barx, top - pad, barwidth, height - fh - pad);

    SmeSnap smesnap = snapHistory.getLast();
    if (smesnap == null) return;
    int spent = 0;
    // last submit err bar
    g.setColor(colorGraphSubmitErr);
    int barheight = smesnap.speed[SmeSnap.SP_REJECTED] * graphScale;
    g.fillRect(barx, y - spent - barheight, barwidth, barheight);
    spent += barheight;
    // last retry bar
    g.setColor(colorGraphRetry);
    barheight = smesnap.speed[SmeSnap.SP_RESCHEDULED] * graphScale;
    g.fillRect(barx, y - spent - barheight, barwidth, barheight);
    spent += barheight;
    // last submit ok bar
    g.setColor(colorGraphSubmit);
    barheight = smesnap.speed[SmeSnap.SP_ACCEPTED] * graphScale;
    g.fillRect(barx, y - spent - barheight, barwidth, barheight);
    spent += barheight;

    barx += barwidth + barSeparator;
    spent = 0;
    g.setColor(graphBarBGColor);
    g.fillRect(barx, top - pad, barwidth, height - fh - pad);
    // last perm err
    g.setColor(colorGraphDeliverErr);
    barheight = smesnap.speed[SmeSnap.SP_PERMERROR] * graphScale;
    g.fillRect(barx, y - spent - barheight, barwidth, barheight);
    spent += barheight;
    // last retry bar
    g.setColor(colorGraphTempErr);
    barheight = smesnap.speed[SmeSnap.SP_TEMPERROR] * graphScale;
    g.fillRect(barx, y - spent - barheight, barwidth, barheight);
    spent += barheight;
    // last submit ok bar
    g.setColor(colorGraphDeliver);
    barheight = smesnap.speed[SmeSnap.SP_DELIVERED] * graphScale;
    g.fillRect(barx, y - spent - barheight, barwidth, barheight);
    spent += barheight;

    g.setColor(graphColor);
    for (int i = 0; y - (i * graphScale) > top; i += graphHead) {
      String s = String.valueOf(i);
      g.drawString(s, graphTextWidth - fm.stringWidth(s), y - (i * graphScale) + fm.getDescent());
    }
    int lineLeft = pad + graphTextWidth + pad;
    g.drawLine(lineLeft, y, size.width - pad - pad, y);
    y--;
    for (int i = graphGrid; ; i += graphGrid) {
      int pos = y - i * graphScale;
      if (pos <= top) break;
      if ((i % graphHiGrid) == 0) {
        g.setColor(graphHiGridColor);
        g.drawLine(lineLeft, pos, size.width - pad, pos);
      } else {
        g.setColor(graphGridColor);
        g.drawLine(lineLeft, pos, size.width - pad, pos);
      }
    }
    // draw vertical scale
    int right = size.width - pad - pad;
    int graphStart = errListWidth + 2 * pad + separatorWidth;
    SmeSnap prevsnap = smesnap;
    for (int i = 0; ; i += graphGrid) {
      int pos = right - i * graphScale;
      if (pos < graphStart) break;
      if ((i % graphHiGrid) == 0)
        g.setColor(graphHiGridColor);
      else
        g.setColor(graphGridColor);
      g.drawLine(pos, top, pos, y);
    }
    int gsz = (snapHistory.count - 1) * graphScale;
    for (int i = 0; i < gsz; i += graphScale) {
      int pos = right - i;
      if (pos < graphStart) break;
      if (prevsnap != null) {
        smesnap = snapHistory.getPrev();
        if (smesnap != null) {
          drawGraphLine(g, pos, y, smesnap.speed[SmeSnap.SP_REJECTED], prevsnap.speed[SmeSnap.SP_REJECTED], 0, 0, colorGraphSubmitErr);
          drawGraphLine(g, pos, y, smesnap.speed[SmeSnap.SP_RESCHEDULED], prevsnap.speed[SmeSnap.SP_RESCHEDULED], smesnap.speed[SmeSnap.SP_REJECTED], prevsnap.speed[SmeSnap.SP_REJECTED], colorGraphRetry);
          drawGraphLine(g, pos, y, smesnap.speed[SmeSnap.SP_ACCEPTED], prevsnap.speed[SmeSnap.SP_ACCEPTED], smesnap.speed[SmeSnap.SP_RESCHEDULED], prevsnap.speed[SmeSnap.SP_RESCHEDULED], colorGraphSubmit);

          drawGraphLine(g, pos, y, smesnap.speed[SmeSnap.SP_PERMERROR], prevsnap.speed[SmeSnap.SP_PERMERROR], 0, 0, colorGraphDeliverErr);
          drawGraphLine(g, pos, y, smesnap.speed[SmeSnap.SP_TEMPERROR], prevsnap.speed[SmeSnap.SP_TEMPERROR], smesnap.speed[SmeSnap.SP_PERMERROR], prevsnap.speed[SmeSnap.SP_PERMERROR], colorGraphTempErr);
          drawGraphLine(g, pos, y, smesnap.speed[SmeSnap.SP_DELIVERED], prevsnap.speed[SmeSnap.SP_DELIVERED], smesnap.speed[SmeSnap.SP_TEMPERROR], prevsnap.speed[SmeSnap.SP_TEMPERROR], colorGraphDeliver);
        }
        prevsnap = smesnap;
      }
    }
    g.setColor(graphColor);
    // draw time scale
    int ty = size.height - pad - fm.getDescent();
    for (int i = 30; ; i += 30) {
      int pos = right - i * graphScale;
      String s = String.valueOf(i);
      int sw = fm.stringWidth(s) / 2;
      if (pos - sw < graphStart) break;
      g.drawString(s, pos - sw, ty);
    }
    g.setColor(colorBackground);
    g.drawLine(errListWidth, top - 1, errListWidth, size.height);
    g.drawLine(errListWidth + separatorWidth + 1, top - 1, errListWidth + separatorWidth + 1, size.height);
    g.setFont(getFont());
  }

  void drawGraphLine(Graphics g, int x, int y, int snapVal, int prevSnapVal, int underGraphVal, int underGraphPrevVal, Color color) {
    if (snapVal == 0 && prevSnapVal == 0) return;
    g.setColor(color);
    g.drawLine(x - graphScale, y - (snapVal + underGraphVal) * graphScale, x, y - (prevSnapVal + underGraphPrevVal) * graphScale);
  }


  public void update(Graphics gg) {
    paint(gg);
  }

  private Dimension prefsz = new Dimension(80, 200);

  public Dimension getPreferredSize() {
    return prefsz;
  }

  private final Dimension minsz = new Dimension(0, 0);

  public Dimension getMinimumSize() {
    return minsz;
  }

  public void mouseClicked(MouseEvent e) {
    System.out.println("Mouse clicked: " + e.getX() + "/" + e.getY());
    Dimension size = getSize();
    int y = e.getY();
    int x = e.getX();
    if (y < headerHeight) {
      // clickedon header
      if (x < errListWidth) {
        // sort err list
        int sortid = x / (counterWidth + pad);
        if (errSortId == sortid) {
          if (errSortState == SORT_ASC) {
            errSortId = -1;
            errComparator = null;
          } else {
            errSortState = SORT_ASC;
            setErrComparator(sortid, true);
          }
        } else {
          errSortId = sortid;
          errSortState = SORT_DESC;
          setErrComparator(sortid, false);
        }
      } else if (x > smeListStart && x < smeGraphStart) {
        // clicked on sme list header
        x -= smeListStart;
        int sortid;
        if (x < smeNameWidth) {
          sortid = 0;
        } else {
          sortid = ((x - smeNameWidth) / (counterWidth + pad)) + 1;
        }
        if (smeSortId == sortid) {
          if (smeSortState == SORT_ASC) {
            smeSortId = -1;
            smeComparator = null;
          } else {
            smeSortState = SORT_ASC;
            setSmeComparator(sortid, true);
          }
        } else {
          smeSortId = sortid;
          smeSortState = SORT_DESC;
          setSmeComparator(sortid, false);
        }
      }
    } else if (y < size.height - split) {
      // lists area
      if (x > smeListStart && x < smeListStart + smeNameWidth) {
        // click on sme name
        int idx = (y - headerHeight) / rowHeight;
        if (idx + scrollPos < snap.smeCount) {
          snapHistory.setCurrentSme(snap.smeSnap[idx + scrollPos].smeId);
          if (split == -1) split = maxSpeed * graphScale + rowHeight + separatorWidth + 2 * pad;
          invalidate();
          repaint();
        }
      }
    } else {
      // graphs area
    }
  }

  void setErrComparator(int sortid, boolean asc) {
    if (sortid == 0)
      errComparator = new ErrorCodeComparator(asc);
    else if (sortid == 1)
      errComparator = new ErrorCountComparator(asc);
    else
      errComparator = new ErrorAverageCountComparator(asc);
  }

  void setSmeComparator(int sortid, boolean asc) {
    if (sortid == 0)
      smeComparator = new SmeIdComparator(asc);
    else if (((sortid - 1) % 2) == 0)
      smeComparator = new SmeSpeedComparator(asc, (sortid - 1) / 2);
    else
      smeComparator = new SmeAverageSpeedComparator(asc, (sortid - 1) / 2);
  }

  public void mousePressed(MouseEvent e) {
  }

  public void mouseReleased(MouseEvent e) {
  }

  public void mouseEntered(MouseEvent e) {
  }

  public void mouseExited(MouseEvent e) {
  }

  public void mouseDragged(MouseEvent e) {
  }

  public void mouseMoved(MouseEvent e) {
    mouseX = e.getX();
    mouseY = e.getY();
  }

  public void keyTyped(KeyEvent e) {
//    System.out.println("Key typed: "+e.getKeyChar());
  }

  public void keyPressed(KeyEvent e) {
//    System.out.println("Key pressed: "+e.getKeyCode());
    switch (e.getKeyCode()) {
      case KeyEvent.VK_UP:
        if ((e.getModifiers() & KeyEvent.SHIFT_MASK) != 0)
          shiftUp();
        else
          scrollUp();
        break;
      case KeyEvent.VK_DOWN:
        if ((e.getModifiers() & KeyEvent.SHIFT_MASK) != 0)
          shiftDown();
        else
          scrollDown();
        break;
      case KeyEvent.VK_PAGE_UP:
        scrollPageUp();
        break;
      case KeyEvent.VK_PAGE_DOWN:
        scrollPageDown();
        break;
      case KeyEvent.VK_HOME:
        scrollHome();
        break;
      case KeyEvent.VK_END:
        scrollEnd();
        break;
      default:
        break;
    }
  }

  public void keyReleased(KeyEvent e) {
  }

  void shiftUp() {
    Dimension sz = getSize();
    if (sz.height - split < headerHeight + rowHeight * 6) return;
    split += shiftStep;
    invalidate();
    repaint();
  }

  void shiftDown() {
    if (split <= 0) return;
    split -= shiftStep;
    if (split < 0) split = 0;
    invalidate();
    repaint();
  }

  void scrollUp() {
    if (scrollPos <= 0) return;
    scrollPos--;
    invalidate();
    repaint();
  }

  void scrollDown() {
    if (scrollPos >= (snap.smeCount - scrollPageRows)) return;
    scrollPos++;
    invalidate();
    repaint();
  }

  void scrollPageUp() {
    if (scrollPos <= 0) return;
    scrollPos -= scrollPageRows;
    if (scrollPos < 0) scrollPos = 0;
    invalidate();
    repaint();
  }

  void scrollPageDown() {
    if (scrollPos >= (snap.smeCount - scrollPageRows)) return;
    scrollPos += scrollPageRows;
    if (scrollPos > (snap.smeCount - scrollPageRows)) scrollPos = snap.smeCount - scrollPageRows;
    invalidate();
    repaint();
  }

  void scrollHome() {
    if (scrollPos <= 0) return;
    scrollPos = 0;
    invalidate();
    repaint();
  }

  void scrollEnd() {
    if (scrollPos >= (snap.smeCount - scrollPageRows)) return;
    scrollPos = snap.smeCount - scrollPageRows;
    invalidate();
    repaint();
  }
}
