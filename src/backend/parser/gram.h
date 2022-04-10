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

#ifndef YY_BASE_YY_GRAM_H_INCLUDED
# define YY_BASE_YY_GRAM_H_INCLUDED
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

#line 501 "gram.h" /* yacc.c:1909  */
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

#endif /* !YY_BASE_YY_GRAM_H_INCLUDED  */
