subscriber.save = INSERT INTO subscribers(address, timezone) VALUES(?,?)
subscriber.remove = DELETE FROM subscribers WHERE address=?
subscriber.lookup = SELECT address, timezone FROM subscribers WHERE address=?

distribution.save = INSERT INTO distributions(name, status) VALUES(?,?)
distribution.remove = DELETE FROM distributions WHERE name=?
distribution.lookup = SELECT name, status FROM distributions WHERE name=?

subscription.save = INSERT INTO subscriptions(subscriber_address, distribution_name, volume, start_date, end_date) VALUES(?,?,?,?,?)
subscription.update = UPDATE subscriptions SET subscriber_address=?, distribution_name=?, volume=?, start_date=?, end_date=? WHERE id=?
subscription.remove = DELETE FROM subscriptions WHERE id=?
subscription.lookupactive = SELECT id, subscriber_address, distribution_name, volume, start_date, end_date FROM subscriptions WHERE subscriber_address=? AND distribution_name=? AND (end_date IS NULL OR end_date > ?)
subscription.lookupfull = SELECT t1.id, t1.subscriber_address, t1.distribution_name, t1.volume, t1.start_date, t1.end_date, t2.name, t2.status, t3.address, t3.timezone FROM subscriptions t1 INNER JOIN distributions t2 ON (t1.distribution_name = t2.name) INNER JOIN subscribers t3 ON (t1.subscriber_address = t3.address) WHERE t1.distribution_name =? AND t1.start_date < ? AND (t1.end_date IS NULL OR t1.end_date > ?) AND t3.timezone=?

volumestat.get = SELECT t1.volume, count(*) FROM subscriptions t1 INNER JOIN subscribers t2 ON (t1.subscriber_address = t2.address) WHERE t1.distribution_name=? AND t2.timezone=? AND t1.start_date < ? AND (t1.end_date IS NULL OR t1.end_date > ?) GROUP BY t1.volume