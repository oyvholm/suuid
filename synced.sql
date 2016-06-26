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
INSERT INTO "synced" VALUES('conv-suuid','Lib/std/perl','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
INSERT INTO "synced" VALUES('src/COPYING','Lib/std/COPYING','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 03:02:53');
INSERT INTO "synced" VALUES('src/Makefile','Lib/std/c/Makefile','29b859aea33e9d479fc122d84c60a8f7cace8ee5','2016-06-23 23:08:31');
INSERT INTO "synced" VALUES('src/suuid.c','Lib/std/c/std.c','29b859aea33e9d479fc122d84c60a8f7cace8ee5','2016-06-23 23:08:31');
INSERT INTO "synced" VALUES('src/suuid.h','Lib/std/c/std.h','29b859aea33e9d479fc122d84c60a8f7cace8ee5','2016-06-23 23:08:31');
INSERT INTO "synced" VALUES('suuid','Lib/std/perl','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
INSERT INTO "synced" VALUES('tests/Add_test','Lib/std/Add_test','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 02:30:32');
INSERT INTO "synced" VALUES('tests/Genlog','Lib/std/Genlog','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 02:30:32');
INSERT INTO "synced" VALUES('tests/conv-suuid.t','Lib/std/perl-tests','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
INSERT INTO "synced" VALUES('tests/suuid.t','Lib/std/perl-tests','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
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
