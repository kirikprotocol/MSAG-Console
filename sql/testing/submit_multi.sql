--principal
insert into dl_principals(address, max_lst, max_el) values('111', 5, 5);

--list1 (no route to 000)
insert into dl_set(list, max_el) values('list1', 5);
insert into dl_members(list, address) values('list1', '3330');
insert into dl_members(list, address) values('list1', '0000');
insert into dl_submitters(list, address) values('list1', '111');

--list2 (222 - transmitter)
insert into dl_set(list, max_el) values('list2', 5);
insert into dl_members(list, address) values('list2', '4440');
insert into dl_members(list, address) values('list2', '2220');
insert into dl_submitters(list, address) values('list2', '111');

--list3 (empty)
insert into dl_set(list, max_el) values('list3', 5);
insert into dl_submitters(list, address) values('list3', '111');

--list4 (not a submitter)
insert into dl_set(list, max_el) values('list4', 5);
