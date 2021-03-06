// generated by BIFF8 Record List Parser

#ifndef BIFF8_RECORDS_H_
#define BIFF8_RECORDS_H_

typedef enum {
	BIFF8_RECORD_FORMULA = 0x0006,
	BIFF8_RECORD_EOF = 0x000a,
	BIFF8_RECORD_CALCCOUNT = 0x000c,
	BIFF8_RECORD_CALCMODE = 0x000d,
	BIFF8_RECORD_PRECISION = 0x000e,
	BIFF8_RECORD_REFMODE = 0x000f,
	BIFF8_RECORD_DELTA = 0x0010,
	BIFF8_RECORD_ITERATION = 0x0011,
	BIFF8_RECORD_PROTECT = 0x0012,
	BIFF8_RECORD_PASSWORD = 0x0013,
	BIFF8_RECORD_HEADER = 0x0014,
	BIFF8_RECORD_FOOTER = 0x0015,
	BIFF8_RECORD_EXTERNSHEET = 0x0017,
	BIFF8_RECORD_DEFINEDNAME = 0x0018,
	BIFF8_RECORD_WINDOWPROTECT = 0x0019,
	BIFF8_RECORD_VERTICALPAGEBREAKS = 0x001a,
	BIFF8_RECORD_HORIZONTALPAGEBREAKS = 0x001b,
	BIFF8_RECORD_NOTE = 0x001c,
	BIFF8_RECORD_SELECTION = 0x001d,
	BIFF8_RECORD_DATEMODE = 0x0022,
	BIFF8_RECORD_EXTERNALNAME = 0x0023,
	BIFF8_RECORD_LEFTMARGIN = 0x0026,
	BIFF8_RECORD_RIGHTMARGIN = 0x0027,
	BIFF8_RECORD_TOPMARGIN = 0x0028,
	BIFF8_RECORD_BOTTOMMARGIN = 0x0029,
	BIFF8_RECORD_PRINTHEADERS = 0x002a,
	BIFF8_RECORD_PRINTGRIDLINES = 0x002b,
	BIFF8_RECORD_FILEPASS = 0x002f,
	BIFF8_RECORD_FONT = 0x0031,
	BIFF8_RECORD_CONTINUE = 0x003c,
	BIFF8_RECORD_WINDOW1 = 0x003d,
	BIFF8_RECORD_BACKUP = 0x0040,
	BIFF8_RECORD_PANE = 0x0041,
	BIFF8_RECORD_CODEPAGE = 0x0042,
	BIFF8_RECORD_PLS = 0x004d,
	BIFF8_RECORD_DCONREF = 0x0051,
	BIFF8_RECORD_DEFCOLWIDTH = 0x0055,
	BIFF8_RECORD_XCT = 0x0059,
	BIFF8_RECORD_CRN = 0x005a,
	BIFF8_RECORD_FILESHARING = 0x005b,
	BIFF8_RECORD_WRITEACCESS = 0x005c,
	BIFF8_RECORD_UNCALCED = 0x005e,
	BIFF8_RECORD_SAVERECALC = 0x005f,
	BIFF8_RECORD_OBJECTPROTECT = 0x0063,
	BIFF8_RECORD_COLINFO = 0x007d,
	BIFF8_RECORD_GUTS = 0x0080,
	BIFF8_RECORD_SHEETPR = 0x0081,
	BIFF8_RECORD_GRIDSET = 0x0082,
	BIFF8_RECORD_HCENTER = 0x0083,
	BIFF8_RECORD_VCENTER = 0x0084,
	BIFF8_RECORD_SHEET = 0x0085,
	BIFF8_RECORD_WRITEPROT = 0x0086,
	BIFF8_RECORD_COUNTRY = 0x008c,
	BIFF8_RECORD_HIDEOBJ = 0x008d,
	BIFF8_RECORD_SORT = 0x0090,
	BIFF8_RECORD_PALETTE = 0x0092,
	BIFF8_RECORD_STANDARDWIDTH = 0x0099,
	BIFF8_RECORD_SCL = 0x00a0,
	BIFF8_RECORD_PAGESETUP = 0x00a1,
	BIFF8_RECORD_MULRK = 0x00bd,
	BIFF8_RECORD_MULBLANK = 0x00be,
	BIFF8_RECORD_RSTRING = 0x00d6,
	BIFF8_RECORD_DBCELL = 0x00d7,
	BIFF8_RECORD_BOOKBOOL = 0x00da,
	BIFF8_RECORD_SCENPROTECT = 0x00dd,
	BIFF8_RECORD_XF = 0x00e0,
	BIFF8_RECORD_MERGEDCELLS = 0x00e5,
	BIFF8_RECORD_BITMAP = 0x00e9,
	BIFF8_RECORD_PHONETICPR = 0x00ef,
	BIFF8_RECORD_SST = 0x00fc,
	BIFF8_RECORD_LABELSST = 0x00fd,
	BIFF8_RECORD_EXTSST = 0x00ff,
	BIFF8_RECORD_LABELRANGES = 0x015f,
	BIFF8_RECORD_USESELFS = 0x0160,
	BIFF8_RECORD_DSF = 0x0161,
	BIFF8_RECORD_EXTERNALBOOK = 0x01ae,
	BIFF8_RECORD_CFHEADER = 0x01b0,
	BIFF8_RECORD_DATAVALIDATIONS = 0x01b2,
	BIFF8_RECORD_HYPERLINK = 0x01b8,
	BIFF8_RECORD_DATAVALIDATION = 0x01be,
	BIFF8_RECORD_DIMENSION = 0x0200,
	BIFF8_RECORD_BLANK = 0x0201,
	BIFF8_RECORD_NUMBER = 0x0203,
	BIFF8_RECORD_LABEL = 0x0204,
	BIFF8_RECORD_BOOLERR = 0x0205,
	BIFF8_RECORD_STRING = 0x0207,
	BIFF8_RECORD_ROW = 0x0208,
	BIFF8_RECORD_INDEX = 0x020b,
	BIFF8_RECORD_ARRAY = 0x0221,
	BIFF8_RECORD_DEFAULTROWHEIGHT = 0x0225,
	BIFF8_RECORD_DATATABLE = 0x0236,
	BIFF8_RECORD_WINDOW2 = 0x023e,
	BIFF8_RECORD_RK = 0x027e,
	BIFF8_RECORD_STYLE = 0x0293,
	BIFF8_RECORD_FORMAT = 0x041e,
	BIFF8_RECORD_SHAREDFMLA = 0x04bc,
	BIFF8_RECORD_QUICKTIP = 0x0800,
	BIFF8_RECORD_BOF = 0x0809,
	BIFF8_RECORD_SHEETLAYOUT = 0x0862,
	BIFF8_RECORD_SHEETPROTECTION = 0x0867,
	BIFF8_RECORD_RANGEPROTECTION = 0x0868,
} BIFF8_RECORD_LIST;

void biff8RecordNamePrint(int record);

#endif // BIFF8_RECORDS_H_
