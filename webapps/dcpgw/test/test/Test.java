package test;

import java.util.ArrayList;
import java.util.Iterator;

public abstract class Test {

    public static ArrayList testObjects = new ArrayList();

    public void registerTestObject(Object object){
        testObjects.add(object);
    }

    public void clean(){
        for (Object testObject : testObjects) {
            try {
                deleteObject(testObject);
            } catch (RuntimeException e) {
                System.out.println(e);
            }
        }
    }

    private void deleteObject(Object o){
        o = null;
    }

}
