delete from wsme_ad;
delete from wsme_langs;
delete from wsme_visitors;

insert into wsme_visitors (mask) values ('.0.1.1?');
insert into wsme_visitors (mask) values ('.0.1.11');

insert into wsme_langs (mask, lang) values ('.0.1.1?', 'ru_ru');
insert into wsme_langs (mask, lang) values ('.0.1.10', 'en_gb');

insert into wsme_ad (id, lang, ad) values (1, 'en_us', 'First en_us ad: hello!!!');
insert into wsme_ad (id, lang, ad) values (3, 'en_us', 'Second en_us ad: hello again!!!');
insert into wsme_ad (id, lang, ad) values (5, 'en_us', 'Third en_us ad: Web services refer to modular platform-independent and language-independent system functionality that is based on open standards and used on demand to support business solutions');

insert into wsme_ad (id, lang, ad) values (1, 'ru_ru', 'First ru_ru ad: hello!!!');
insert into wsme_ad (id, lang, ad) values (3, 'ru_ru', 'Second ru_ru ad: снова привет!!!');
insert into wsme_ad (id, lang, ad) values (5, 'ru_ru', '“ретий спам: –ост реальных доходов росси€н способствует увеличению спроса на пассажирские авиаперевозки');

insert into wsme_ad (id, lang, ad) values (1, 'en_gb', 'First en_gb ad: hello!!!');
insert into wsme_ad (id, lang, ad) values (3, 'en_gb', 'Second en_gb ad: hello again!!!');
insert into wsme_ad (id, lang, ad) values (5, 'en_gb', 'Third en_gb ad: hello again again!!!');
