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
INSERT INTO "synced" VALUES('obsolete/suuid.pl','','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
INSERT INTO "synced" VALUES('src/COPYING','Lib/std/COPYING','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 03:02:53');
INSERT INTO "synced" VALUES('src/Makefile','Lib/std/c/Makefile','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('src/common.h','Lib/std/c/std.h','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/sess.c','Lib/std/c/std.c','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/sess.h','Lib/std/c/std.h','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/suuid.c','Lib/std/c/std.c','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/suuid.h','Lib/std/c/std.h','e51fd07fe8a4d3a95c20ab339534707c2faf5d38','2016-11-10 10:58:31');
INSERT INTO "synced" VALUES('src/t/suuid.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('tests/Add_test','Lib/std/Add_test','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('tests/Genlog','Lib/std/Genlog','d1ebf8e7aaf89fd22baf628255bd698b95d7df29','2016-07-13 20:23:13');
INSERT INTO "synced" VALUES('tests/conv-suuid.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
INSERT INTO "synced" VALUES('tests/sess.t','Lib/std/perl-tests','5d9a5b337cbc4997dfa181e3fd9a9484487b5605','2016-10-25 02:43:03');
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
