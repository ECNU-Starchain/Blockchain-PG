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

#ifndef YY_BASE_YY_PREPROC_H_INCLUDED
# define YY_BASE_YY_PREPROC_H_INCLUDED
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
    SQL_ALLOCATE = 258,
    SQL_AUTOCOMMIT = 259,
    SQL_BOOL = 260,
    SQL_BREAK = 261,
    SQL_CALL = 262,
    SQL_CARDINALITY = 263,
    SQL_CONNECT = 264,
    SQL_COUNT = 265,
    SQL_DATETIME_INTERVAL_CODE = 266,
    SQL_DATETIME_INTERVAL_PRECISION = 267,
    SQL_DESCRIBE = 268,
    SQL_DESCRIPTOR = 269,
    SQL_DISCONNECT = 270,
    SQL_FOUND = 271,
    SQL_FREE = 272,
    SQL_GET = 273,
    SQL_GO = 274,
    SQL_GOTO = 275,
    SQL_IDENTIFIED = 276,
    SQL_INDICATOR = 277,
    SQL_KEY_MEMBER = 278,
    SQL_LENGTH = 279,
    SQL_LONG = 280,
    SQL_NULLABLE = 281,
    SQL_OCTET_LENGTH = 282,
    SQL_OPEN = 283,
    SQL_OUTPUT = 284,
    SQL_REFERENCE = 285,
    SQL_RETURNED_LENGTH = 286,
    SQL_RETURNED_OCTET_LENGTH = 287,
    SQL_SCALE = 288,
    SQL_SECTION = 289,
    SQL_SHORT = 290,
    SQL_SIGNED = 291,
    SQL_SQL = 292,
    SQL_SQLERROR = 293,
    SQL_SQLPRINT = 294,
    SQL_SQLWARNING = 295,
    SQL_START = 296,
    SQL_STOP = 297,
    SQL_STRUCT = 298,
    SQL_UNSIGNED = 299,
    SQL_VAR = 300,
    SQL_WHENEVER = 301,
    S_ADD = 302,
    S_AND = 303,
    S_ANYTHING = 304,
    S_AUTO = 305,
    S_CONST = 306,
    S_DEC = 307,
    S_DIV = 308,
    S_DOTPOINT = 309,
    S_EQUAL = 310,
    S_EXTERN = 311,
    S_INC = 312,
    S_LSHIFT = 313,
    S_MEMPOINT = 314,
    S_MEMBER = 315,
    S_MOD = 316,
    S_MUL = 317,
    S_NEQUAL = 318,
    S_OR = 319,
    S_REGISTER = 320,
    S_RSHIFT = 321,
    S_STATIC = 322,
    S_SUB = 323,
    S_VOLATILE = 324,
    S_TYPEDEF = 325,
    TYPECAST = 326,
    CSTRING = 327,
    CVARIABLE = 328,
    CPP_LINE = 329,
    IP = 330,
    DOLCONST = 331,
    ECONST = 332,
    NCONST = 333,
    UCONST = 334,
    UIDENT = 335,
    ABORT_P = 336,
    ABSOLUTE_P = 337,
    ACCESS = 338,
    ACTION = 339,
    ADD_P = 340,
    ADMIN = 341,
    AFTER = 342,
    AGGREGATE = 343,
    ALL = 344,
    ALSO = 345,
    ALTER = 346,
    ALWAYS = 347,
    ANALYSE = 348,
    ANALYZE = 349,
    AND = 350,
    ANY = 351,
    ARRAY = 352,
    AS = 353,
    ASC = 354,
    ASSERTION = 355,
    ASSIGNMENT = 356,
    ASYMMETRIC = 357,
    AT = 358,
    AUTHORIZATION = 359,
    AUTHSELECT = 360,
    AUTHTABLE = 361,
    BACKWARD = 362,
    BEFORE = 363,
    BEGIN_P = 364,
    BETWEEN = 365,
    BIGINT = 366,
    BINARY = 367,
    BIT = 368,
    BOOLEAN_P = 369,
    BOTH = 370,
    BY = 371,
    CACHE = 372,
    CALLED = 373,
    CASCADE = 374,
    CASCADED = 375,
    CASE = 376,
    CAST = 377,
    CATALOG_P = 378,
    CHAIN = 379,
    CHAR_P = 380,
    CHARACTER = 381,
    CHARACTERISTICS = 382,
    CHECK = 383,
    CHECKPOINT = 384,
    CLASS = 385,
    CLOSE = 386,
    CLUSTER = 387,
    COALESCE = 388,
    COLLATE = 389,
    COLUMN = 390,
    COMMENT = 391,
    COMMIT = 392,
    COMMITTED = 393,
    CONCURRENTLY = 394,
    CONFIGURATION = 395,
    CONNECTION = 396,
    CONSTRAINT = 397,
    CONSTRAINTS = 398,
    CONTENT_P = 399,
    CONTINUE_P = 400,
    CONVERSION_P = 401,
    COPY = 402,
    COST = 403,
    CREATE = 404,
    CREATEDB = 405,
    CREATEROLE = 406,
    CREATEUSER = 407,
    CROSS = 408,
    CSV = 409,
    CURRENT_P = 410,
    CURRENT_CATALOG = 411,
    CURRENT_DATE = 412,
    CURRENT_ROLE = 413,
    CURRENT_SCHEMA = 414,
    CURRENT_TIME = 415,
    CURRENT_TIMESTAMP = 416,
    CURRENT_USER = 417,
    CURSOR = 418,
    CYCLE = 419,
    DATA_P = 420,
    DATABASE = 421,
    DAY_P = 422,
    DEALLOCATE = 423,
    DEC = 424,
    DECIMAL_P = 425,
    DECLARE = 426,
    DEFAULT = 427,
    DEFAULTS = 428,
    DEFERRABLE = 429,
    DEFERRED = 430,
    DEFINER = 431,
    DELETE_P = 432,
    DELIMITER = 433,
    DELIMITERS = 434,
    DESC = 435,
    DICTIONARY = 436,
    DISABLE_P = 437,
    DISCARD = 438,
    DISTINCT = 439,
    DO = 440,
    DOCUMENT_P = 441,
    DOMAIN_P = 442,
    DOUBLE_P = 443,
    DROP = 444,
    EACH = 445,
    ELSE = 446,
    ENABLE_P = 447,
    ENCODING = 448,
    ENCRYPTED = 449,
    END_P = 450,
    ENUM_P = 451,
    ESCAPE = 452,
    EXCEPT = 453,
    EXCLUDING = 454,
    EXCLUSIVE = 455,
    EXECUTE = 456,
    EXISTS = 457,
    EXPLAIN = 458,
    EXTERNAL = 459,
    EXTRACT = 460,
    FALSE_P = 461,
    FAMILY = 462,
    FETCH = 463,
    FIRST_P = 464,
    FLOAT_P = 465,
    FOLLOWING = 466,
    FOR = 467,
    FORCE = 468,
    FOREIGN = 469,
    FORWARD = 470,
    FREEZE = 471,
    FROM = 472,
    FULL = 473,
    FUNCTION = 474,
    GLOBAL = 475,
    GRANT = 476,
    GRANTED = 477,
    GREATEST = 478,
    GROUP_P = 479,
    HANDLER = 480,
    HAVING = 481,
    HEADER_P = 482,
    HOLD = 483,
    HOUR_P = 484,
    IDENTITY_P = 485,
    IF_P = 486,
    ILIKE = 487,
    IMMEDIATE = 488,
    IMMUTABLE = 489,
    IMPLICIT_P = 490,
    IN_P = 491,
    INCLUDING = 492,
    INCREMENT = 493,
    INDEX = 494,
    INDEXES = 495,
    INHERIT = 496,
    INHERITS = 497,
    INITIALLY = 498,
    INNER_P = 499,
    INOUT = 500,
    INPUT_P = 501,
    INSENSITIVE = 502,
    INSERT = 503,
    INSTEAD = 504,
    INT_P = 505,
    INTEGER = 506,
    INTERSECT = 507,
    INTERVAL = 508,
    INTO = 509,
    INVOKER = 510,
    IS = 511,
    ISNULL = 512,
    ISOLATION = 513,
    JOIN = 514,
    KEY = 515,
    LANCOMPILER = 516,
    LANGUAGE = 517,
    LARGE_P = 518,
    LAST_P = 519,
    LC_COLLATE_P = 520,
    LC_CTYPE_P = 521,
    LEADING = 522,
    LEAST = 523,
    LEFT = 524,
    LEVEL = 525,
    LIKE = 526,
    LIMIT = 527,
    LISTEN = 528,
    LOAD = 529,
    LOCAL = 530,
    LOCALTIME = 531,
    LOCALTIMESTAMP = 532,
    LOCATION = 533,
    LOCK_P = 534,
    LOGIN_P = 535,
    MAPPING = 536,
    MATCH = 537,
    MAXVALUE = 538,
    MINUTE_P = 539,
    MINVALUE = 540,
    MODE = 541,
    MONTH_P = 542,
    MOVE = 543,
    NAME_P = 544,
    NAMES = 545,
    NATIONAL = 546,
    NATURAL = 547,
    NCHAR = 548,
    NEW = 549,
    NEXT = 550,
    NO = 551,
    NOCREATEDB = 552,
    NOCREATEROLE = 553,
    NOCREATEUSER = 554,
    NOINHERIT = 555,
    NOLOGIN_P = 556,
    NONE = 557,
    NOSUPERUSER = 558,
    NOT = 559,
    NOTHING = 560,
    NOTIFY = 561,
    NOTNULL = 562,
    NOWAIT = 563,
    NULL_P = 564,
    NULLIF = 565,
    NULLS_P = 566,
    NUMERIC = 567,
    OBJECT_P = 568,
    OF = 569,
    OFF = 570,
    OFFSET = 571,
    OIDS = 572,
    OLD = 573,
    ON = 574,
    ONLY = 575,
    OPERATOR = 576,
    OPTION = 577,
    OPTIONS = 578,
    OR = 579,
    ORDER = 580,
    OUT_P = 581,
    OUTER_P = 582,
    OVER = 583,
    OVERLAPS = 584,
    OVERLAY = 585,
    OWNED = 586,
    OWNER = 587,
    PARSER = 588,
    PARTIAL = 589,
    PARTITION = 590,
    PASSWORD = 591,
    PLACING = 592,
    PLANS = 593,
    POSITION = 594,
    PRECEDING = 595,
    PRECISION = 596,
    PRESERVE = 597,
    PREPARE = 598,
    PREPARED = 599,
    PRIMARY = 600,
    PRIOR = 601,
    PRIVILEGES = 602,
    PROCEDURAL = 603,
    PROCEDURE = 604,
    QUOTE = 605,
    RANGE = 606,
    READ = 607,
    REAL = 608,
    REASSIGN = 609,
    RECHECK = 610,
    RECURSIVE = 611,
    REFERENCES = 612,
    REINDEX = 613,
    RELATIVE_P = 614,
    RELEASE = 615,
    RENAME = 616,
    REPEATABLE = 617,
    REPLACE = 618,
    REPLICA = 619,
    RESET = 620,
    RESTART = 621,
    RESTRICT = 622,
    RETURNING = 623,
    RETURNS = 624,
    REVOKE = 625,
    RIGHT = 626,
    ROLE = 627,
    ROLLBACK = 628,
    ROW = 629,
    ROWS = 630,
    RULE = 631,
    SAVEPOINT = 632,
    SCHEMA = 633,
    SCROLL = 634,
    SEARCH = 635,
    SECOND_P = 636,
    SECURITY = 637,
    SELECT = 638,
    SEQUENCE = 639,
    SERIALIZABLE = 640,
    SERVER = 641,
    SESSION = 642,
    SESSION_USER = 643,
    SET = 644,
    SETOF = 645,
    SHARE = 646,
    SHOW = 647,
    SIMILAR = 648,
    SIMPLE = 649,
    SMALLINT = 650,
    SOME = 651,
    STABLE = 652,
    STANDALONE_P = 653,
    START = 654,
    STATEMENT = 655,
    STATISTICS = 656,
    STDIN = 657,
    STDOUT = 658,
    STORAGE = 659,
    STRICT_P = 660,
    STRIP_P = 661,
    SUBSTRING = 662,
    SUPERUSER_P = 663,
    SYMMETRIC = 664,
    SYSID = 665,
    SYSTEM_P = 666,
    TABLE = 667,
    TABLESPACE = 668,
    TEMP = 669,
    TEMPLATE = 670,
    TEMPORARY = 671,
    TEXT_P = 672,
    THEN = 673,
    TIME = 674,
    TIMESTAMP = 675,
    TO = 676,
    TRAILING = 677,
    TRANSACTION = 678,
    TREAT = 679,
    TRIGGER = 680,
    TRIM = 681,
    TRUE_P = 682,
    TRUNCATE = 683,
    TRUSTED = 684,
    TYPE_P = 685,
    UNBOUNDED = 686,
    UNCOMMITTED = 687,
    UNENCRYPTED = 688,
    UNION = 689,
    UNIQUE = 690,
    UNKNOWN = 691,
    UNLISTEN = 692,
    UNTIL = 693,
    UPDATE = 694,
    USER = 695,
    USING = 696,
    VACUUM = 697,
    VALID = 698,
    VALIDATOR = 699,
    VALUE_P = 700,
    VALUES = 701,
    VARCHAR = 702,
    VARIADIC = 703,
    VARYING = 704,
    VERBOSE = 705,
    VERSION_P = 706,
    VIEW = 707,
    VOLATILE = 708,
    WHEN = 709,
    WHERE = 710,
    WHITESPACE_P = 711,
    WINDOW = 712,
    WITH = 713,
    WITHOUT = 714,
    WORK = 715,
    WRAPPER = 716,
    WRITE = 717,
    XML_P = 718,
    XMLATTRIBUTES = 719,
    XMLCONCAT = 720,
    XMLELEMENT = 721,
    XMLFOREST = 722,
    XMLPARSE = 723,
    XMLPI = 724,
    XMLROOT = 725,
    XMLSERIALIZE = 726,
    YEAR_P = 727,
    YES_P = 728,
    ZONE = 729,
    NULLS_FIRST = 730,
    NULLS_LAST = 731,
    WITH_TIME = 732,
    IDENT = 733,
    FCONST = 734,
    SCONST = 735,
    BCONST = 736,
    XCONST = 737,
    Op = 738,
    ICONST = 739,
    PARAM = 740,
    POSTFIXOP = 741,
    UMINUS = 742
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 387 "preproc.y" /* yacc.c:1909  */

	double	dval;
	char	*str;
	int     ival;
	struct	when		action;
	struct	index		index;
	int		tagname;
	struct	this_type	type;
	enum	ECPGttype	type_enum;
	enum	ECPGdtype	dtype_enum;
	struct	fetch_desc	descriptor;
	struct  su_symbol	struct_union;
	struct	prep		prep;

#line 557 "preproc.h" /* yacc.c:1909  */
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

#endif /* !YY_BASE_YY_PREPROC_H_INCLUDED  */
