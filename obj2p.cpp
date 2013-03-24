#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

// Program to generate a raw zx81.p file and preload machine code into it.
// Main use is that assembler can be written externally in an assembler and the
// object code output is inserted in here.
// It's basically a zx81 wrapper allowing the z80 code to be run on the zx81 emulator

// characters:
// 0-9: 28-37 inc. (0x1C-0x25)
// A-Z: 38-63 inc. Add 128 for inverse

static	const	unsigned char REM = 234;			// 0xEA
static	const	unsigned char SAVE = 248;			// 0xF8
static	const	unsigned char RAND = 249;			// 0xF9
static	const	unsigned char USR = 212;			// 0xD4
static	const	unsigned char NEWLINE = 118;		// 0x76
static	const	unsigned char QUOTE = 11;			// 0x0B
static	const	unsigned char NUMBER_MARKER = 126;	// 0x7E
static	const	unsigned char ZERO = 28;			// 0x1C
static	const	unsigned char LETTER_A = 38;

static	const	int	CODE_START = 16509;

// Save system variables structure. The names in here are a bit clunky
// Saved system vars start at address 16393
#pragma pack(1)					// make sure everything stays byte aligned
struct SystemVars
{
	unsigned char	VERSN;		// Identifies ZX81 BASIC in saved programs.
	unsigned short	E_PPC;		// Number of current line (with program cursor).
	unsigned short	D_FILE;		// Start of display file
	unsigned short	DF_CC;		// Address of PRINT position in display file. Can be poked so that PRINT output is sent elsewhere.
	unsigned short	VARS;		// Start of variables
	unsigned short	DEST;		// Address of variable in assignment.
	unsigned short	E_LINE;		// Address after variable list
	unsigned short	CH_ADD;		// Address of the next character to be interpreted: the character after the argument of PEEK, or the NEWLINE at the end of a POKE statement.
	unsigned short	X_PTR;		// Address of the character preceding the marker.
	unsigned short	STKBOT;		// bottom of calculator stack
	unsigned short	STKEND;		// end of calculator stack
	unsigned char	BERG;		// Calculator's b register
	unsigned short	MEM;		// Address of area used for calculator's memory. (Usually MEMBOT, but not always.)
	unsigned char	Unused1;
	unsigned char	DF_SZ;		// The number of lines (including one blank line) in the lower part of the screen.
	unsigned short	S_TOP;		// The number of the top program line in automatic listings.
	unsigned short	LAST_K;		// Shows which keys pressed.
	unsigned char	DEBOUNCE;	// Debounce status of keyboard.
	unsigned char	MARGIN;		// Number of blank lines above or below picture: 55 in Britain, 31 in America.
	unsigned short	NXTLIN;		// Address of next program line to be executed.
	unsigned short	OLDPPC;		// Line number of which CONT jumps.
	unsigned char	FLAGX;		// Various flags.
	unsigned short	STRLEN; 	// Length of string type destination in assignment.
	unsigned short	T_ADDR;		// Address of next item in syntax table (very unlikely to be useful).
	unsigned short	SEED;		// The seed for RND. This is the variable that is set by RAND.
	unsigned short	FRAMES;		// Counts the frames displayed on the television. Bit 15 is 1. Bits 0 to 14 are decremented for each frame set to the television. This can be used for timing, but PAUSE also uses it. PAUSE resets to 0 bit 15, & puts in bits 0 to 14 the length of the pause. When these have been counted down to zero, the pause stops. If the pause stops because of a key depression, bit 15 is set to 1 again.
	unsigned char	COORDS;		// x-coordinate of last point PLOTted.
	unsigned char	COORDS_Y;	// y-coordinate of last point PLOTted.
	unsigned char	PR_CC;		// Less significant byte of address of next position for LPRINT to print as (in PRBUFF).
	unsigned char	S_POSN;		// Column number for PRINT position.
	unsigned char	S_LINE;		// Line number for PRINT position.
	unsigned char	CDFLAG;		// Various flags. Bit 7 is on (1) during compute & display mode.
	unsigned char	PRBUFF[33];	// Printer buffer (33rd character is NEWLINE).
	unsigned char	MEMBOT[30]; // Calculator's memory area; used to store numbers that cannot conveniently be put on the calculator stack.
	unsigned short	Unused2;
};

SystemVars vars =
{
	0x00,
	0x0002,
	0x6169,			// D_FILE
	0x616A,			// DF_CC
	0x6482,			// VARS
	0x0000,
	0x6483,			// E_LINE
	0x6156,			// CH_ADD
	0xC000,
	0x6483,			// STKBOT
	0x6483,			// STKEND
	0x00,
	0x405D,
	0x00,
	0x02,
	0x0000,
	0xFDBF,
	0xFF,
	0x37,
	0x6157,			// NXTLIN
	0x0000,
	0x00,
	0x0000,
	0x0C8D,
	0x4082,
	0xE8D9,
	0x00,
	0x00,
	0xBC,
	0x21,
	0x18,
	0x40,

	// PRBUFF
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x76,

	// MEMBOT
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	(short)0x0000
};

// exponent will always be 8F (shift of 15 bits, 32768)
// (( addr / 32768) - 0.5) * 2^32
// ((addr * 2) - 32768) * 65536
unsigned long	GetMantissaFromAddress(int address)
{
	unsigned long result = address * 2;
	result -= 32768;
	result *= 65536;
	return result;
}

int	GetAddressFromString(char* addressString)
{
	int	execAddress = atoi(addressString);

	return execAddress;
}

// create the rem line for the assembler to go into. Assumed to be at line 0
int GenerateRemLine(unsigned char* buffer, int remLength)
{
	int lineLength = remLength + 2;		// extra characters for REM and newline
	int count = 0;

	// save line number
	buffer[count++] = 0x00;
	buffer[count++] = 0x00;

	// save line length
	buffer[count++] = lineLength % 256;			// LSB
	buffer[count++] = lineLength / 256;			// MSB

	// REM
	buffer[count++] = REM;
	for (int loop = 0; loop < remLength; loop++)
	{
		buffer[count++] = ZERO;					// '0' in ZX81 character set
	}

	// newline
	buffer[count++] = NEWLINE;

	return count;
}

// Generate the Save line. Assumed to be line 1.
// Returns number of bytes saved to buffer (not a string; some bytes can be 0)
// created line is returned in buffer
int	GenerateSaveLine(unsigned char* buffer, char* filename)
{
	int fileLength = strlen(filename);
	int lineLength = fileLength + 4;	// 2 bytes for ", 1 for SAVE and 1 for newline
	int count = 0;

	// save line number
	buffer[count++] = 0x00;
	buffer[count++] = 0x01;

	// save line length
	buffer[count++] = lineLength % 256;			// LSB
	buffer[count++] = lineLength / 256;			// MSB

	// SAVE
	buffer[count++] = SAVE;
	buffer[count++] = QUOTE;

	// save the file name
	for (int loop = 0; loop < fileLength; loop++)
	{
		unsigned char letter = toupper(filename[loop]);
		if (letter >= '0' && letter <= '9')
		{
			letter -= '0';
			letter += ZERO;
		}
		else
		{
			letter -= 'A';
			letter += LETTER_A;
		}
		if (loop == fileLength - 1)
			letter |= 0x80;					// invert last character
		buffer[count++] = letter;
	}

	buffer[count++] = QUOTE;
	buffer[count++] = NEWLINE;

	return count;
}

// Generate the line to execute the assembler at address. Assumed to be at line 2
// Returns number of bytes saved to buffer (not a string; some bytes can be 0)
// created line is returned in buffer
int	GenerateUsrLine(unsigned char* buffer, char* addressString)
{
	// commands with numbers in (like this one with an address), have a hidden 6 bytes
	// tagged onto the end. The first byte is 0x7E, indicating that the next 5 bytes are
	// a floating point value representing the 5 address bytes. It's an 'optimization' so
	// that the conversion from decimal to exec address can be done quicker.

	int lineLength = 14;
	int count = 0;

	// save line number
	buffer[count++] = 0x00;
	buffer[count++] = 0x02;

	// save line length
	buffer[count++] = lineLength % 256;			// LSB
	buffer[count++] = lineLength / 256;			// MSB

	// RAND USR
	buffer[count++] = RAND;
	buffer[count++] = USR;

	// save address
	int address = GetAddressFromString(addressString);

	for (int loop = 0; loop < 5; loop++)
	{
		buffer[count++] = addressString[loop] - '0' + ZERO;
	}

	// save hidden floating point address
	buffer[count++] = NUMBER_MARKER;
	buffer[count++] = 0x8F;						// Exponent byte (*32768 - 0x0F bits)
	unsigned long	mantissa = GetMantissaFromAddress(address);

	unsigned long	tempMantissa = mantissa >> 24;
	buffer[count++] = (unsigned char) tempMantissa;
	tempMantissa = mantissa >> 16;
	buffer[count++] = (unsigned char) tempMantissa;
	tempMantissa = mantissa >> 8;
	buffer[count++] = (unsigned char) tempMantissa;
	buffer[count++] = (unsigned char) mantissa;

	buffer[count++] = NEWLINE;

	return count;
}

// Generate display file data.
// Returns number of bytes saved to buffer (not a string; some bytes can be 0)
// created line is returned in buffer
int GenerateDisplayFile(unsigned char* buffer)
{
	int count = 0;
	buffer[count++] = NEWLINE;						// start with newline
	for (int yLoop = 0; yLoop < 24; yLoop++)
	{
		for (int xLoop = 0; xLoop < 32; xLoop++)
		{
			buffer[count++] = 0x00;
		}
		buffer[count++] = NEWLINE;
	}
	buffer[count++] = 0x80;							// end of save file marker
	return count;
}

void Usage(void)
{
    fprintf( stderr, "\nobj2zx81 <input object file> -e <exec address> <output file>\n\n" );
	fprintf( stderr, "The exec address is where the code will start from executing immediately\n");
	fprintf( stderr, "after loading.\n");
	fprintf( stderr, "If -e is not specified, the default of 16514 will be used. This address\n");
	fprintf( stderr, "must lie in the range 16383 to 32767.\n\n");
	fprintf( stderr, "The output file doesn't need the .p extension added. It will be added\n");
	fprintf( stderr, "automatically. Output filename can only use alpha-numeric characters.\n");
}

void main (int argc, char *argv[])
{
	// file pointers
	FILE    *fp_object, *fp_out;

	// buffers for the 2 basic lines
	unsigned char	saveBuffer[256];
	unsigned char	usrBuffer[256];
	unsigned char	displayBuffer[1024];

	char	execAddressString[10];
	char	objectFile[24];
	char	outputFile[24];

	// default values
	strcpy( execAddressString, "16514" );
	memset( objectFile, 0, 24);
	memset( outputFile, 0, 24);

    // Check command line options
	int		index = 0;
	int		address = 0;
    char    *s;

	bool	error = false;

    if( argc != 3 && argc != 5)
    {
		error = true;
	}
	else
	{
		strcpy( objectFile, *++argv);

        if ((*++argv)[0] == '-')
        {
            s = argv[0]+1;
            switch(*s)
            {
                case 'e':
                    s = *++argv;
					index = 0;
                    while( *s != '\0' && *s != ' ' )
                    {
						execAddressString[index++] = *s++;
                    }
					execAddressString[index++] = '\0';

					address = GetAddressFromString(execAddressString);
					if (address < 16383 || address > 32767)
					{
						error = true;
					}

					++argv;
                    break;

				default:
					error = true;
					break;

            }
        }

		if (*argv)
		{
			strcpy( outputFile, *argv);
			int length = strlen(outputFile);
			index = 0;
			while (index < length && !error)
			{
				unsigned char letter = toupper(outputFile[index++]);
				bool inRange = ((letter >= '0' && letter <= '9') || (letter >= 'A' && letter <= 'Z'));
				if (!inRange)
				{
					error = true;
				}
			}
		}
		else
		{
			error = true;
		}
	}

	// proceed if no errors reported
	if (!error)
	{
		// open the object file
		fp_object = fopen (objectFile, "rb");
		if (fp_object != NULL)
		{
		    // get size of object file
		    struct stat *st;
			st = (struct stat *)calloc(1,sizeof(struct stat));
	        long theObjectFileSize = stat (objectFile,st);
		    int	objectSize = st->st_size;

			// allocate a buffer for the rem line
			unsigned char* remBuffer = new unsigned char[objectSize+10];

			// calculate how big things are going to be
			int		varsLength = sizeof(SystemVars);
			int		remLength = GenerateRemLine(remBuffer, objectSize);
			int		saveLength = GenerateSaveLine(saveBuffer, outputFile);
			int		usrLength = GenerateUsrLine(usrBuffer, execAddressString);
			int		displayLength = GenerateDisplayFile(displayBuffer);

			// read the object buffer to where it needs to go. Needs to be read 5 bytes in
			// to skip the line number, line size and rem command
			fread( remBuffer+5, 1, objectSize, fp_object );
		    fclose( fp_object );

			// modify the system variables
			int		displayFile = remLength + saveLength + usrLength + CODE_START;

			vars.D_FILE = (unsigned short)displayFile;
			vars.DF_CC = (unsigned short)vars.D_FILE + 1;
			vars.VARS = (unsigned short)vars.D_FILE + displayLength - 1;
			vars.CH_ADD = (unsigned short)vars.D_FILE - 19;
			vars.E_LINE = vars.VARS+1;
			vars.STKBOT = vars.VARS+1;
			vars.STKEND = vars.VARS+1;
			vars.NXTLIN = vars.CH_ADD+1;

			// open the output file and write the data
			char	outFilename[128];
			strcpy(outFilename, outputFile);
			strcat(outFilename, ".p");
			fp_out = fopen (outFilename, "wb");
			if (fp_out != NULL)
			{
				fwrite( &vars, 1, varsLength, fp_out);
				fwrite( remBuffer, 1, remLength, fp_out);
				fwrite( saveBuffer, 1, saveLength, fp_out);
				fwrite( usrBuffer, 1, usrLength, fp_out);
				fwrite( displayBuffer, 1, displayLength, fp_out);
				fclose (fp_out);
			}

			// all done. Clean up

		    delete [] remBuffer;
		}
    }
    else
    {
        Usage();
	}
}
