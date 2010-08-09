package ru.sibinco.scag.backend.stat.counters;

/**
 * Created by IntelliJ IDEA.
 * User: stepanov
 * Date: 30.07.2010
 * Time: 13:30:46
 * To change this template use File | Settings | File Templates.
 */
public class Limit implements java.io.Serializable, Cloneable
    {
        private int percent = 0;
        private LevelType level;
        private String op = null;
        private String value = null;

        public Limit(int percent, LevelType level) {
            setPercent(percent); setLevel(level);
        }
        public Limit(int percent, String level) {
            setPercent(percent); setLevel(level);
        }
        public Limit(String percent, String level) {
            setPercent(percent); setLevel(level);
        }

        public LevelType getLevel() {
            return level;
        }
        public String getLevelStr() {
            return level.toString();
        }
        public void setLevel(LevelType level) {
            this.level = level;
        }
        public void setLevel(String level) {
            this.level = LevelType.valueOf(level);
        }

        public int getPercent() {
            return percent;
        }
        public String getPercentStr() {
            return Integer.toString(percent);
        }
        public void setPercent(int percent) {
            this.percent = percent;
        }
        public void setPercent(String percent) {
            this.percent = Integer.valueOf(percent);
        }

        public String getOp(){
            return this.op;
        }

        public void setOp(String op){
            this.op = op;
        }

        public void setValue(String value){
            this.value = value;
        }

        public String getValue(){
            return this.value;
        }

        public String toString() {
            return "Limit{" +
                    "percent=\"" + getPercent() + "\"" +
                    ", level=\"" + getLevel() + "\"" +
                    ", op=\"" + getOp() + "\"" +
                    ", value=\"" + getValue() +
                    "\"}";
        }

    }
