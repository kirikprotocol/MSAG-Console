package mobi.eyeline.dcpgw.model;

import java.util.Arrays;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 11.08.11
 * Time: 15:04
 */
public class Delivery {

    private int id;

    private String user;

    private String[] services_numbers;

    public Delivery(){

    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }


    public String getUser() {
        return user;
    }

    public void setUser(String user) {
        this.user = user;
    }

    public String[] getServicesNumbers() {
        return services_numbers;
    }

    public void setServicesNumbers(String[] services_numbers) {
        this.services_numbers = services_numbers;
    }

    public String toString(){
        return "delivery: id="+id+", user="+user+", services_numbers="+ Arrays.toString(services_numbers);
    }
}
