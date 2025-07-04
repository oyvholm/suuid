PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE synced (
  file TEXT
    CONSTRAINT synced_file_length
      CHECK (length(file) > 0)
    UNIQUE
    NOT NULL
  ,
  orig TEXT
  ,
  rev TEXT
    CONSTRAINT synced_rev_length
      CHECK (length(rev) = 40 OR rev = '')
  ,
  date TEXT
    CONSTRAINT synced_date_length
      CHECK (date IS NULL OR length(date) = 19)
    CONSTRAINT synced_date_valid
      CHECK (date IS NULL OR datetime(date) IS NOT NULL)
);
INSERT INTO synced VALUES('conv-suuid','Lib/std/perl','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO synced VALUES('finduuid','Lib/std/perl-tab','c58b81b5652b062928cc7bde7153bda48f2474a8','2016-11-15 00:26:12');
INSERT INTO synced VALUES('needuuid','Lib/std/perl-tab','c6365e55b0510bc1a385dc497ff4d958b003c012','2016-11-11 21:46:30');
INSERT INTO synced VALUES('obsolete/suuid.pl','','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
INSERT INTO synced VALUES('src/COPYING','Lib/std/COPYING','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 03:02:53');
INSERT INTO synced VALUES('src/Gen-version','Lib/std/c/Gen-version','9cf4bf09ae2a5fca8bbf26c8ee46d99e589f3381','2018-02-24 21:09:41');
INSERT INTO synced VALUES('src/Makefile','Lib/std/c/Makefile','9cf4bf09ae2a5fca8bbf26c8ee46d99e589f3381','2018-02-24 21:09:41');
INSERT INTO synced VALUES('src/gdbrc','Lib/std/c/gdbrc','70a3d4af3bfc94480ade6a6206ecaaafbe23d5a0','2018-02-22 01:09:32');
INSERT INTO synced VALUES('src/selftest.c','Lib/std/c/selftest.c','6277f2b035d8cab6d50c0d102ae626e54bd3bb72','2017-12-21 12:56:56');
INSERT INTO synced VALUES('src/suuid.c','Lib/std/c/std.c','70a3d4af3bfc94480ade6a6206ecaaafbe23d5a0','2018-02-22 01:09:32');
INSERT INTO synced VALUES('src/suuid.h','Lib/std/c/std.h','194ba0690e345aaa5c44ba2c429c17e42f1f9e9b','2018-02-24 16:36:59');
INSERT INTO synced VALUES('src/t/suuid.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO synced VALUES('tests/conv-suuid.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO synced VALUES('tests/finduuid.t','Lib/std/perl-tests-tab','c58b81b5652b062928cc7bde7153bda48f2474a8','2016-11-15 00:26:12');
INSERT INTO synced VALUES('tests/retval.sh','','',NULL);
INSERT INTO synced VALUES('tests/sess.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO synced VALUES('ti','Lib/std/sh','53d7643de23eed270013018ef12e7e80b2eb3f63','2016-11-20 16:04:29');
INSERT INTO synced VALUES('tjah','Lib/std/sh','53d7643de23eed270013018ef12e7e80b2eb3f63','2016-11-20 16:04:29');
INSERT INTO synced VALUES('wi','','',NULL);
CREATE TABLE todo (
  file TEXT
    CONSTRAINT todo_file_length
      CHECK(length(file) > 0)
    UNIQUE
    NOT NULL
  ,
  pri INTEGER
    CONSTRAINT todo_pri_range
      CHECK(pri BETWEEN 1 AND 5)
  ,
  comment TEXT
);
COMMIT;
