/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_BASE_YY_Y_TAB_H_INCLUDED
# define YY_BASE_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int base_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ABORT_P = 258,
    ABSOLUTE_P = 259,
    ACCESS = 260,
    ACTION = 261,
    ADD_P = 262,
    ADMIN = 263,
    AFTER = 264,
    AGGREGATE = 265,
    ALL = 266,
    ALSO = 267,
    ALTER = 268,
    ALWAYS = 269,
    ANALYSE = 270,
    ANALYZE = 271,
    AND = 272,
    ANY = 273,
    ARRAY = 274,
    AS = 275,
    ASC = 276,
    ASSERTION = 277,
    ASSIGNMENT = 278,
    ASYMMETRIC = 279,
    AT = 280,
    AUTHORIZATION = 281,
    AUTHSELECT = 282,
    AUTHTABLE = 283,
    BACKWARD = 284,
    BEFORE = 285,
    BEGIN_P = 286,
    BETWEEN = 287,
    BIGINT = 288,
    BINARY = 289,
    BIT = 290,
    BOOLEAN_P = 291,
    BOTH = 292,
    BY = 293,
    CACHE = 294,
    CALLED = 295,
    CASCADE = 296,
    CASCADED = 297,
    CASE = 298,
    CAST = 299,
    CATALOG_P = 300,
    CHAIN = 301,
    CHAR_P = 302,
    CHARACTER = 303,
    CHARACTERISTICS = 304,
    CHECK = 305,
    CHECKPOINT = 306,
    CLASS = 307,
    CLOSE = 308,
    CLUSTER = 309,
    COALESCE = 310,
    COLLATE = 311,
    COLUMN = 312,
    COMMENT = 313,
    COMMIT = 314,
    COMMITTED = 315,
    CONCURRENTLY = 316,
    CONFIGURATION = 317,
    CONNECTION = 318,
    CONSTRAINT = 319,
    CONSTRAINTS = 320,
    CONTENT_P = 321,
    CONTINUE_P = 322,
    CONVERSION_P = 323,
    COPY = 324,
    COST = 325,
    CREATE = 326,
    CREATEDB = 327,
    CREATEROLE = 328,
    CREATEUSER = 329,
    CROSS = 330,
    CSV = 331,
    CURRENT_P = 332,
    CURRENT_CATALOG = 333,
    CURRENT_DATE = 334,
    CURRENT_ROLE = 335,
    CURRENT_SCHEMA = 336,
    CURRENT_TIME = 337,
    CURRENT_TIMESTAMP = 338,
    CURRENT_USER = 339,
    CURSOR = 340,
    CYCLE = 341,
    DATA_P = 342,
    DATABASE = 343,
    DAY_P = 344,
    DEALLOCATE = 345,
    DEC = 346,
    DECIMAL_P = 347,
    DECLARE = 348,
    DEFAULT = 349,
    DEFAULTS = 350,
    DEFERRABLE = 351,
    DEFERRED = 352,
    DEFINER = 353,
    DELETE_P = 354,
    DELIMITER = 355,
    DELIMITERS = 356,
    DESC = 357,
    DICTIONARY = 358,
    DISABLE_P = 359,
    DISCARD = 360,
    DISTINCT = 361,
    DO = 362,
    DOCUMENT_P = 363,
    DOMAIN_P = 364,
    DOUBLE_P = 365,
    DROP = 366,
    EACH = 367,
    ELSE = 368,
    ENABLE_P = 369,
    ENCODING = 370,
    ENCRYPTED = 371,
    END_P = 372,
    ENUM_P = 373,
    ESCAPE = 374,
    EXCEPT = 375,
    EXCLUDING = 376,
    EXCLUSIVE = 377,
    EXECUTE = 378,
    EXISTS = 379,
    EXPLAIN = 380,
    EXTERNAL = 381,
    EXTRACT = 382,
    FALSE_P = 383,
    FAMILY = 384,
    FETCH = 385,
    FIRST_P = 386,
    FLOAT_P = 387,
    FOLLOWING = 388,
    FOR = 389,
    FORCE = 390,
    FOREIGN = 391,
    FORWARD = 392,
    FREEZE = 393,
    FROM = 394,
    FULL = 395,
    FUNCTION = 396,
    GLOBAL = 397,
    GRANT = 398,
    GRANTED = 399,
    GREATEST = 400,
    GROUP_P = 401,
    HANDLER = 402,
    HAVING = 403,
    HEADER_P = 404,
    HOLD = 405,
    HOUR_P = 406,
    IDENTITY_P = 407,
    IF_P = 408,
    ILIKE = 409,
    IMMEDIATE = 410,
    IMMUTABLE = 411,
    IMPLICIT_P = 412,
    IN_P = 413,
    INCLUDING = 414,
    INCREMENT = 415,
    INDEX = 416,
    INDEXES = 417,
    INHERIT = 418,
    INHERITS = 419,
    INITIALLY = 420,
    INNER_P = 421,
    INOUT = 422,
    INPUT_P = 423,
    INSENSITIVE = 424,
    INSERT = 425,
    INSTEAD = 426,
    INT_P = 427,
    INTEGER = 428,
    INTERSECT = 429,
    INTERVAL = 430,
    INTO = 431,
    INVOKER = 432,
    IS = 433,
    ISNULL = 434,
    ISOLATION = 435,
    JOIN = 436,
    KEY = 437,
    LANCOMPILER = 438,
    LANGUAGE = 439,
    LARGE_P = 440,
    LAST_P = 441,
    LC_COLLATE_P = 442,
    LC_CTYPE_P = 443,
    LEADING = 444,
    LEAST = 445,
    LEFT = 446,
    LEVEL = 447,
    LIKE = 448,
    LIMIT = 449,
    LISTEN = 450,
    LOAD = 451,
    LOCAL = 452,
    LOCALTIME = 453,
    LOCALTIMESTAMP = 454,
    LOCATION = 455,
    LOCK_P = 456,
    LOGIN_P = 457,
    MAPPING = 458,
    MATCH = 459,
    MAXVALUE = 460,
    MINUTE_P = 461,
    MINVALUE = 462,
    MODE = 463,
    MONTH_P = 464,
    MOVE = 465,
    NAME_P = 466,
    NAMES = 467,
    NATIONAL = 468,
    NATURAL = 469,
    NCHAR = 470,
    NEW = 471,
    NEXT = 472,
    NO = 473,
    NOCREATEDB = 474,
    NOCREATEROLE = 475,
    NOCREATEUSER = 476,
    NOINHERIT = 477,
    NOLOGIN_P = 478,
    NONE = 479,
    NOSUPERUSER = 480,
    NOT = 481,
    NOTHING = 482,
    NOTIFY = 483,
    NOTNULL = 484,
    NOWAIT = 485,
    NULL_P = 486,
    NULLIF = 487,
    NULLS_P = 488,
    NUMERIC = 489,
    OBJECT_P = 490,
    OF = 491,
    OFF = 492,
    OFFSET = 493,
    OIDS = 494,
    OLD = 495,
    ON = 496,
    ONLY = 497,
    OPERATOR = 498,
    OPTION = 499,
    OPTIONS = 500,
    OR = 501,
    ORDER = 502,
    OUT_P = 503,
    OUTER_P = 504,
    OVER = 505,
    OVERLAPS = 506,
    OVERLAY = 507,
    OWNED = 508,
    OWNER = 509,
    PARSER = 510,
    PARTIAL = 511,
    PARTITION = 512,
    PASSWORD = 513,
    PLACING = 514,
    PLANS = 515,
    POSITION = 516,
    PRECEDING = 517,
    PRECISION = 518,
    PRESERVE = 519,
    PREPARE = 520,
    PREPARED = 521,
    PRIMARY = 522,
    PRIOR = 523,
    PRIVILEGES = 524,
    PROCEDURAL = 525,
    PROCEDURE = 526,
    QUOTE = 527,
    RANGE = 528,
    READ = 529,
    REAL = 530,
    REASSIGN = 531,
    RECHECK = 532,
    RECURSIVE = 533,
    REFERENCES = 534,
    REINDEX = 535,
    RELATIVE_P = 536,
    RELEASE = 537,
    RENAME = 538,
    REPEATABLE = 539,
    REPLACE = 540,
    REPLICA = 541,
    RESET = 542,
    RESTART = 543,
    RESTRICT = 544,
    RETURNING = 545,
    RETURNS = 546,
    REVOKE = 547,
    RIGHT = 548,
    ROLE = 549,
    ROLLBACK = 550,
    ROW = 551,
    ROWS = 552,
    RULE = 553,
    SAVEPOINT = 554,
    SCHEMA = 555,
    SCROLL = 556,
    SEARCH = 557,
    SECOND_P = 558,
    SECURITY = 559,
    SELECT = 560,
    SEQUENCE = 561,
    SERIALIZABLE = 562,
    SERVER = 563,
    SESSION = 564,
    SESSION_USER = 565,
    SET = 566,
    SETOF = 567,
    SHARE = 568,
    SHOW = 569,
    SIMILAR = 570,
    SIMPLE = 571,
    SMALLINT = 572,
    SOME = 573,
    STABLE = 574,
    STANDALONE_P = 575,
    START = 576,
    STATEMENT = 577,
    STATISTICS = 578,
    STDIN = 579,
    STDOUT = 580,
    STORAGE = 581,
    STRICT_P = 582,
    STRIP_P = 583,
    SUBSTRING = 584,
    SUPERUSER_P = 585,
    SYMMETRIC = 586,
    SYSID = 587,
    SYSTEM_P = 588,
    TABLE = 589,
    TABLESPACE = 590,
    TEMP = 591,
    TEMPLATE = 592,
    TEMPORARY = 593,
    TEXT_P = 594,
    THEN = 595,
    TIME = 596,
    TIMESTAMP = 597,
    TO = 598,
    TRAILING = 599,
    TRANSACTION = 600,
    TREAT = 601,
    TRIGGER = 602,
    TRIM = 603,
    TRUE_P = 604,
    TRUNCATE = 605,
    TRUSTED = 606,
    TYPE_P = 607,
    UNBOUNDED = 608,
    UNCOMMITTED = 609,
    UNENCRYPTED = 610,
    UNION = 611,
    UNIQUE = 612,
    UNKNOWN = 613,
    UNLISTEN = 614,
    UNTIL = 615,
    UPDATE = 616,
    USER = 617,
    USING = 618,
    VACUUM = 619,
    VALID = 620,
    VALIDATOR = 621,
    VALUE_P = 622,
    VALUES = 623,
    VARCHAR = 624,
    VARIADIC = 625,
    VARYING = 626,
    VERBOSE = 627,
    VERSION_P = 628,
    VIEW = 629,
    VOLATILE = 630,
    WHEN = 631,
    WHERE = 632,
    WHITESPACE_P = 633,
    WINDOW = 634,
    WITH = 635,
    WITHOUT = 636,
    WORK = 637,
    WRAPPER = 638,
    WRITE = 639,
    XML_P = 640,
    XMLATTRIBUTES = 641,
    XMLCONCAT = 642,
    XMLELEMENT = 643,
    XMLFOREST = 644,
    XMLPARSE = 645,
    XMLPI = 646,
    XMLROOT = 647,
    XMLSERIALIZE = 648,
    YEAR_P = 649,
    YES_P = 650,
    ZONE = 651,
    NULLS_FIRST = 652,
    NULLS_LAST = 653,
    WITH_TIME = 654,
    IDENT = 655,
    FCONST = 656,
    SCONST = 657,
    BCONST = 658,
    XCONST = 659,
    Op = 660,
    ICONST = 661,
    PARAM = 662,
    POSTFIXOP = 663,
    UMINUS = 664,
    TYPECAST = 665
  };
#endif
/* Tokens.  */
#define ABORT_P 258
#define ABSOLUTE_P 259
#define ACCESS 260
#define ACTION 261
#define ADD_P 262
#define ADMIN 263
#define AFTER 264
#define AGGREGATE 265
#define ALL 266
#define ALSO 267
#define ALTER 268
#define ALWAYS 269
#define ANALYSE 270
#define ANALYZE 271
#define AND 272
#define ANY 273
#define ARRAY 274
#define AS 275
#define ASC 276
#define ASSERTION 277
#define ASSIGNMENT 278
#define ASYMMETRIC 279
#define AT 280
#define AUTHORIZATION 281
#define AUTHSELECT 282
#define AUTHTABLE 283
#define BACKWARD 284
#define BEFORE 285
#define BEGIN_P 286
#define BETWEEN 287
#define BIGINT 288
#define BINARY 289
#define BIT 290
#define BOOLEAN_P 291
#define BOTH 292
#define BY 293
#define CACHE 294
#define CALLED 295
#define CASCADE 296
#define CASCADED 297
#define CASE 298
#define CAST 299
#define CATALOG_P 300
#define CHAIN 301
#define CHAR_P 302
#define CHARACTER 303
#define CHARACTERISTICS 304
#define CHECK 305
#define CHECKPOINT 306
#define CLASS 307
#define CLOSE 308
#define CLUSTER 309
#define COALESCE 310
#define COLLATE 311
#define COLUMN 312
#define COMMENT 313
#define COMMIT 314
#define COMMITTED 315
#define CONCURRENTLY 316
#define CONFIGURATION 317
#define CONNECTION 318
#define CONSTRAINT 319
#define CONSTRAINTS 320
#define CONTENT_P 321
#define CONTINUE_P 322
#define CONVERSION_P 323
#define COPY 324
#define COST 325
#define CREATE 326
#define CREATEDB 327
#define CREATEROLE 328
#define CREATEUSER 329
#define CROSS 330
#define CSV 331
#define CURRENT_P 332
#define CURRENT_CATALOG 333
#define CURRENT_DATE 334
#define CURRENT_ROLE 335
#define CURRENT_SCHEMA 336
#define CURRENT_TIME 337
#define CURRENT_TIMESTAMP 338
#define CURRENT_USER 339
#define CURSOR 340
#define CYCLE 341
#define DATA_P 342
#define DATABASE 343
#define DAY_P 344
#define DEALLOCATE 345
#define DEC 346
#define DECIMAL_P 347
#define DECLARE 348
#define DEFAULT 349
#define DEFAULTS 350
#define DEFERRABLE 351
#define DEFERRED 352
#define DEFINER 353
#define DELETE_P 354
#define DELIMITER 355
#define DELIMITERS 356
#define DESC 357
#define DICTIONARY 358
#define DISABLE_P 359
#define DISCARD 360
#define DISTINCT 361
#define DO 362
#define DOCUMENT_P 363
#define DOMAIN_P 364
#define DOUBLE_P 365
#define DROP 366
#define EACH 367
#define ELSE 368
#define ENABLE_P 369
#define ENCODING 370
#define ENCRYPTED 371
#define END_P 372
#define ENUM_P 373
#define ESCAPE 374
#define EXCEPT 375
#define EXCLUDING 376
#define EXCLUSIVE 377
#define EXECUTE 378
#define EXISTS 379
#define EXPLAIN 380
#define EXTERNAL 381
#define EXTRACT 382
#define FALSE_P 383
#define FAMILY 384
#define FETCH 385
#define FIRST_P 386
#define FLOAT_P 387
#define FOLLOWING 388
#define FOR 389
#define FORCE 390
#define FOREIGN 391
#define FORWARD 392
#define FREEZE 393
#define FROM 394
#define FULL 395
#define FUNCTION 396
#define GLOBAL 397
#define GRANT 398
#define GRANTED 399
#define GREATEST 400
#define GROUP_P 401
#define HANDLER 402
#define HAVING 403
#define HEADER_P 404
#define HOLD 405
#define HOUR_P 406
#define IDENTITY_P 407
#define IF_P 408
#define ILIKE 409
#define IMMEDIATE 410
#define IMMUTABLE 411
#define IMPLICIT_P 412
#define IN_P 413
#define INCLUDING 414
#define INCREMENT 415
#define INDEX 416
#define INDEXES 417
#define INHERIT 418
#define INHERITS 419
#define INITIALLY 420
#define INNER_P 421
#define INOUT 422
#define INPUT_P 423
#define INSENSITIVE 424
#define INSERT 425
#define INSTEAD 426
#define INT_P 427
#define INTEGER 428
#define INTERSECT 429
#define INTERVAL 430
#define INTO 431
#define INVOKER 432
#define IS 433
#define ISNULL 434
#define ISOLATION 435
#define JOIN 436
#define KEY 437
#define LANCOMPILER 438
#define LANGUAGE 439
#define LARGE_P 440
#define LAST_P 441
#define LC_COLLATE_P 442
#define LC_CTYPE_P 443
#define LEADING 444
#define LEAST 445
#define LEFT 446
#define LEVEL 447
#define LIKE 448
#define LIMIT 449
#define LISTEN 450
#define LOAD 451
#define LOCAL 452
#define LOCALTIME 453
#define LOCALTIMESTAMP 454
#define LOCATION 455
#define LOCK_P 456
#define LOGIN_P 457
#define MAPPING 458
#define MATCH 459
#define MAXVALUE 460
#define MINUTE_P 461
#define MINVALUE 462
#define MODE 463
#define MONTH_P 464
#define MOVE 465
#define NAME_P 466
#define NAMES 467
#define NATIONAL 468
#define NATURAL 469
#define NCHAR 470
#define NEW 471
#define NEXT 472
#define NO 473
#define NOCREATEDB 474
#define NOCREATEROLE 475
#define NOCREATEUSER 476
#define NOINHERIT 477
#define NOLOGIN_P 478
#define NONE 479
#define NOSUPERUSER 480
#define NOT 481
#define NOTHING 482
#define NOTIFY 483
#define NOTNULL 484
#define NOWAIT 485
#define NULL_P 486
#define NULLIF 487
#define NULLS_P 488
#define NUMERIC 489
#define OBJECT_P 490
#define OF 491
#define OFF 492
#define OFFSET 493
#define OIDS 494
#define OLD 495
#define ON 496
#define ONLY 497
#define OPERATOR 498
#define OPTION 499
#define OPTIONS 500
#define OR 501
#define ORDER 502
#define OUT_P 503
#define OUTER_P 504
#define OVER 505
#define OVERLAPS 506
#define OVERLAY 507
#define OWNED 508
#define OWNER 509
#define PARSER 510
#define PARTIAL 511
#define PARTITION 512
#define PASSWORD 513
#define PLACING 514
#define PLANS 515
#define POSITION 516
#define PRECEDING 517
#define PRECISION 518
#define PRESERVE 519
#define PREPARE 520
#define PREPARED 521
#define PRIMARY 522
#define PRIOR 523
#define PRIVILEGES 524
#define PROCEDURAL 525
#define PROCEDURE 526
#define QUOTE 527
#define RANGE 528
#define READ 529
#define REAL 530
#define REASSIGN 531
#define RECHECK 532
#define RECURSIVE 533
#define REFERENCES 534
#define REINDEX 535
#define RELATIVE_P 536
#define RELEASE 537
#define RENAME 538
#define REPEATABLE 539
#define REPLACE 540
#define REPLICA 541
#define RESET 542
#define RESTART 543
#define RESTRICT 544
#define RETURNING 545
#define RETURNS 546
#define REVOKE 547
#define RIGHT 548
#define ROLE 549
#define ROLLBACK 550
#define ROW 551
#define ROWS 552
#define RULE 553
#define SAVEPOINT 554
#define SCHEMA 555
#define SCROLL 556
#define SEARCH 557
#define SECOND_P 558
#define SECURITY 559
#define SELECT 560
#define SEQUENCE 561
#define SERIALIZABLE 562
#define SERVER 563
#define SESSION 564
#define SESSION_USER 565
#define SET 566
#define SETOF 567
#define SHARE 568
#define SHOW 569
#define SIMILAR 570
#define SIMPLE 571
#define SMALLINT 572
#define SOME 573
#define STABLE 574
#define STANDALONE_P 575
#define START 576
#define STATEMENT 577
#define STATISTICS 578
#define STDIN 579
#define STDOUT 580
#define STORAGE 581
#define STRICT_P 582
#define STRIP_P 583
#define SUBSTRING 584
#define SUPERUSER_P 585
#define SYMMETRIC 586
#define SYSID 587
#define SYSTEM_P 588
#define TABLE 589
#define TABLESPACE 590
#define TEMP 591
#define TEMPLATE 592
#define TEMPORARY 593
#define TEXT_P 594
#define THEN 595
#define TIME 596
#define TIMESTAMP 597
#define TO 598
#define TRAILING 599
#define TRANSACTION 600
#define TREAT 601
#define TRIGGER 602
#define TRIM 603
#define TRUE_P 604
#define TRUNCATE 605
#define TRUSTED 606
#define TYPE_P 607
#define UNBOUNDED 608
#define UNCOMMITTED 609
#define UNENCRYPTED 610
#define UNION 611
#define UNIQUE 612
#define UNKNOWN 613
#define UNLISTEN 614
#define UNTIL 615
#define UPDATE 616
#define USER 617
#define USING 618
#define VACUUM 619
#define VALID 620
#define VALIDATOR 621
#define VALUE_P 622
#define VALUES 623
#define VARCHAR 624
#define VARIADIC 625
#define VARYING 626
#define VERBOSE 627
#define VERSION_P 628
#define VIEW 629
#define VOLATILE 630
#define WHEN 631
#define WHERE 632
#define WHITESPACE_P 633
#define WINDOW 634
#define WITH 635
#define WITHOUT 636
#define WORK 637
#define WRAPPER 638
#define WRITE 639
#define XML_P 640
#define XMLATTRIBUTES 641
#define XMLCONCAT 642
#define XMLELEMENT 643
#define XMLFOREST 644
#define XMLPARSE 645
#define XMLPI 646
#define XMLROOT 647
#define XMLSERIALIZE 648
#define YEAR_P 649
#define YES_P 650
#define ZONE 651
#define NULLS_FIRST 652
#define NULLS_LAST 653
#define WITH_TIME 654
#define IDENT 655
#define FCONST 656
#define SCONST 657
#define BCONST 658
#define XCONST 659
#define Op 660
#define ICONST 661
#define PARAM 662
#define POSTFIXOP 663
#define UMINUS 664
#define TYPECAST 665

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 148 "gram.y" /* yacc.c:1909  */

	int					ival;
	char				chr;
	char				*str;
	const char			*keyword;
	bool				boolean;
	JoinType			jtype;
	DropBehavior		dbehavior;
	OnCommitAction		oncommit;
	List				*list;
	Node				*node;
	Value				*value;
	ObjectType			objtype;

	TypeName			*typnam;
	FunctionParameter   *fun_param;
	FunctionParameterMode fun_param_mode;
	FuncWithArgs		*funwithargs;
	DefElem				*defelt;
	SortBy				*sortby;
	WindowDef			*windef;
	JoinExpr			*jexpr;
	IndexElem			*ielem;
	Alias				*alias;
	RangeVar			*range;
	IntoClause			*into;
	WithClause			*with;
	A_Indices			*aind;
	ResTarget			*target;
	struct PrivTarget	*privtarget;
	AccessPriv			*accesspriv;

	InsertStmt			*istmt;
	VariableSetStmt		*vsetstmt;

#line 910 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE base_yylval;
extern YYLTYPE base_yylloc;
int base_yyparse (void);

#endif /* !YY_BASE_YY_Y_TAB_H_INCLUDED  */
