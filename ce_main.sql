-- ***************************************************************************
--	clice_main.sql - Script for creating the Command Line Coding Ecosystem database
--	Usage:-
--		sqlite3 /var/local/clice_main.db < ce_main.sql
-- ***************************************************************************

-- Main table with full module details

CREATE TABLE ce_main (
	id		INTEGER PRIMARY KEY  NOT NULL,
	name	CHARACTER(30),
	type	INTEGER,
	status	INTEGER,
	dir		CHARACTER(30),
	source	CHARACTER(30),
	project	CHARACTER(3),
	cdate	INTEGER,
	ctime	INTEGER,
	mdate	INTEGER,
	mtime	INTEGER,
	desc	CHARACTER(40),
	lang	CHARACTER(1),
	size	INTEGER);

-- Pointer table to show relationships between modules

CREATE TABLE ce_link (
	id		INTEGER PRIMARY KEY NOT NULL,
	name	CHARACTER(30),
	calls	CHARACTER(30),
	rel		CHARACTER(1),
	code	CHARACTER(60),
	time	INTEGER );
