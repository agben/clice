-- ***************************************************************************
--	Script for creating the Command Line Coding Ecosystem database
--	Usage:-
--		sqlite3 /var/local/clice_main.db < ce_main.sql
-- ***************************************************************************

-- Main table with full module details

CREATE TABLE ce_main (
	id		INTEGER PRIMARY KEY  NOT NULL,
	name	CHARACTER(60),
	type	INTEGER,
	main	CHARACTER(1),
	ignore	CHARACTER(1),
	library	CHARACTER(1),
	spare	CHARACTER(1),
	dir		CHARACTER(30),
	source	CHARACTER(30),
	project	CHARACTER(3),
	cdate	INTEGER,
	ctime	INTEGER,
	mdate	INTEGER,
	mtime	INTEGER,
	desc	CHARACTER(60),
	code	CHARACTER(60),
	lang	CHARACTER(1),
	size	INTEGER);

-- Pointer table to show relationships between modules

CREATE TABLE ce_link (
	id		INTEGER PRIMARY KEY NOT NULL,
	name	CHARACTER(60),
	calls	CHARACTER(60),
	ntype	INTEGER,
	ctype	INTEGER,
	code	CHARACTER(60),
	time	INTEGER );
