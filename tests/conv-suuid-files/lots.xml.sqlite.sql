BEGIN TRANSACTION;
INSERT INTO uuids (t,u,tag,host,cwd,username,tty,sess,txt,s) VALUES('2016-11-30T08:49:39.2512000Z','eda46800-b6d9-11e6-96f0-279c2a0468a3',NULL,'','','','','[""]','Lots of stuff!
New line.

tab	here
< & >
\
''
"in\valid\','<suuid t="2016-11-30T08:49:39.2512000Z" u="eda46800-b6d9-11e6-96f0-279c2a0468a3"> <txt>Lots of stuff!\nNew line.\n\ntab\there\n&lt; &amp; &gt;\n\\\n''\n"in\valid\</txt> </suuid>');
COMMIT;
