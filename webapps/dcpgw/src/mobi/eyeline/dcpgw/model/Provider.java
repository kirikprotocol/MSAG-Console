package mobi.eyeline.dcpgw.model;

import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 11.08.11
 * Time: 15:17
 */
public class Provider {

    private String name, description;

    private Vector<Delivery> deliveries;

    private Hashtable<String, Delivery> service_number_delivery_table;

    private String[] endpoint_ids;

    public Provider(){
        service_number_delivery_table = new Hashtable<String, Delivery>();
        deliveries = new Vector<Delivery>();
    }

    public Vector<Delivery> getDeliveries() {
        return deliveries;
    }

    public void setDeliveries(Vector<Delivery> deliveries) {
        this.deliveries = deliveries;

        for(Delivery delivery: deliveries){
            String[] services_numbers = delivery.getServicesNumbers();
            for(String service_number: services_numbers){
                service_number_delivery_table.put(service_number, delivery);
            }
        }
    }

    public void addDelivery(Delivery delivery){
        deliveries.add(delivery);

        String[] services_numbers = delivery.getServicesNumbers();
        for(String service_number: services_numbers){
            service_number_delivery_table.put(service_number, delivery);
        }
    }

    public Delivery getDelivery(String service_number){
        return service_number_delivery_table.get(service_number);
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String[] getEndpointIds() {
        return endpoint_ids;
    }

    public void setEndpointIds(String[] endpoint_ids) {
        this.endpoint_ids = endpoint_ids;
    }

    public String toString(){
        int[] ids = new int[deliveries.size()];
        for(int i=0;i<deliveries.size();i++){
            Delivery delivery = deliveries.get(i);
            ids[i] = delivery.getId();
        }
        return "provider: name="+name+", description="+description+", endpoints="+ Arrays.toString(endpoint_ids)+", deliveries_ids="+Arrays.toString(ids);
    }
}
