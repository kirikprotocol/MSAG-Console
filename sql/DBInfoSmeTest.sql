

-- ********************* Testing tables for InfoSme ********************* --

DROP TABLE INFOSME_TEST_PROVIDER;
CREATE TABLE INFOSME_TEST_PROVIDER
(
    ABONENT	VARCHAR2(30)	NOT NULL,
    BALANCE     NUMBER(6,2)     NOT NULL,
    DT          DATE            NULL
);

CREATE OR REPLACE PROCEDURE INFOSME_DATA (
    total     IN NUMBER DEFAULT 100,
    num_end   IN NUMBER DEFAULT 5,
    num_start IN NUMBER DEFAULT 2) AS
BEGIN
    FOR counter IN 1..total LOOP
	FOR i IN num_start..num_end LOOP
    	    INSERT INTO INFOSME_TEST_PROVIDER (ABONENT , BALANCE, DT)
	    VALUES (CONCAT('.0.1.', TO_CHAR(i)), i*11.11, NULL);
	END LOOP;
    END LOOP;
END;
/
