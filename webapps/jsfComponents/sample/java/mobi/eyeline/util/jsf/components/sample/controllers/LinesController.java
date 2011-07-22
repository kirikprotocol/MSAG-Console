package mobi.eyeline.util.jsf.components.sample.controllers;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * author: Aleksandr Khalitov
 */
public class LinesController {


  public int[][] getValues() {
    Random r = new Random();
    int[][] res = new int[10][2];
    for(int i=0;i<10;i++) {
      res[i][0] = i;
      res[i][1] = r.nextInt(100);
    }
    return res;
  }

  public List<String> getLabels() {
    Random r = new Random();
    List<String> res = new ArrayList<String>(10);
    for(int i=0;i<10;i++) {
      res.add("label"+r.nextInt(10));
    }
    return res;
  }
}
