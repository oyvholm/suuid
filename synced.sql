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
INSERT INTO "synced" VALUES('conv-suuid','Lib/std/perl','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('finduuid','Lib/std/perl-tab','c58b81b5652b062928cc7bde7153bda48f2474a8','2016-11-15 00:26:12');
INSERT INTO "synced" VALUES('needuuid','Lib/std/perl-tab','c6365e55b0510bc1a385dc497ff4d958b003c012','2016-11-11 21:46:30');
INSERT INTO "synced" VALUES('obsolete/suuid.pl','','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
INSERT INTO "synced" VALUES('src/COPYING','Lib/std/COPYING','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 03:02:53');
INSERT INTO "synced" VALUES('src/Gen-version','Lib/std/sh','4f9d95911fd2ae01ab5f189b1a5f6ef7ecfe3694','2016-11-25 13:18:18');
INSERT INTO "synced" VALUES('src/Makefile','Lib/std/c/Makefile','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('src/common.h','Lib/std/c/std.h','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/sess.c','Lib/std/c/std.c','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/sess.h','Lib/std/c/std.h','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/suuid.c','Lib/std/c/std.c','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/suuid.h','Lib/std/c/std.h','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/t/suuid.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('tests/Add_test','Lib/std/Add_test','b1c0c1047fda193d2ac42d9de96dc08cfd864704','2016-11-12 23:54:40');
INSERT INTO "synced" VALUES('tests/Genlog','Lib/std/Genlog','54a033402a020bf69f09f5572a9c9b39e8b9034e','2016-11-17 14:52:43');
INSERT INTO "synced" VALUES('tests/conv-suuid.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('tests/finduuid.t','Lib/std/perl-tests-tab','c58b81b5652b062928cc7bde7153bda48f2474a8','2016-11-15 00:26:12');
INSERT INTO "synced" VALUES('tests/retval.sh','','',NULL);
INSERT INTO "synced" VALUES('tests/sess.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('ti','Lib/std/sh','53d7643de23eed270013018ef12e7e80b2eb3f63','2016-11-20 16:04:29');
INSERT INTO "synced" VALUES('tjah','Lib/std/sj','53d7643de23eed270013018ef12e7e80b2eb3f63','2016-11-20 16:04:29');
INSERT INTO "synced" VALUES('wi','','',NULL);
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
