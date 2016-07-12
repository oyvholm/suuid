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
INSERT INTO "synced" VALUES('obsolete/suuid.pl','Lib/std/perl','273d6f5b1b1e7eb6c8553cd2cb3b176c5678ac7c','2016-05-17 14:10:28');
INSERT INTO "synced" VALUES('src/COPYING','Lib/std/COPYING','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 03:02:53');
INSERT INTO "synced" VALUES('src/Makefile','Lib/std/c/Makefile','c707e16461a3b371a6e270a285d9ac200b8a1e76','2016-07-07 13:15:36');
INSERT INTO "synced" VALUES('src/common.h','Lib/std/c/std.h','a4c18cda5d2ee2640fcfa27631312c6c651540e5','2016-07-12 01:57:35');
INSERT INTO "synced" VALUES('src/sess.c','Lib/std/c/std.c','a4c18cda5d2ee2640fcfa27631312c6c651540e5','2016-07-12 01:57:35');
INSERT INTO "synced" VALUES('src/sess.h','Lib/std/c/std.h','a4c18cda5d2ee2640fcfa27631312c6c651540e5','2016-07-12 01:57:35');
INSERT INTO "synced" VALUES('src/suuid.c','Lib/std/c/std.c','3721489ea544cf7985dc45870d48e2b1ee60a0ed','2016-07-07 13:30:08');
INSERT INTO "synced" VALUES('src/suuid.h','Lib/std/c/std.h','3721489ea544cf7985dc45870d48e2b1ee60a0ed','2016-07-07 13:30:08');
INSERT INTO "synced" VALUES('src/t/suuid.t','Lib/std/perl-tests','419ded4305b40d128560d85946e3ebd9745944d8','2016-07-07 11:59:49');
INSERT INTO "synced" VALUES('tests/Add_test','Lib/std/Add_test','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 02:30:32');
INSERT INTO "synced" VALUES('tests/Genlog','Lib/std/Genlog','17bc73d79f068dfaf2655db1090f6b9206734555','2016-06-06 02:30:32');
INSERT INTO "synced" VALUES('tests/conv-suuid.t','Lib/std/perl-tests','419ded4305b40d128560d85946e3ebd9745944d8','2016-07-07 11:59:49');
INSERT INTO "synced" VALUES('tests/sess.t','Lib/std/perl-tests','ee75f42c8e84cc59c8b694417731b8130dbdc050','2016-07-07 18:01:45');
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
