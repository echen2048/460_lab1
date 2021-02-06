#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdbool.h>


#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255

typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1]; /* Question for the reader: Why do we need to add 1? */
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

enum{
    DONE, OK, EMPTY_LINE
};

FILE* infile = NULL;
FILE* outfile = NULL;

int toNum(char * pStr);
void firstPass(FILE * lInFile);
int isOpcode(char * opcode);

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
);

int main(int argc, char* argv[]) {
    if(argc != 4){
        printf("Insufficient Arguments");
        return -1;
    }
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName   = argv[1];
    iFileName = argv[2];
    oFileName = argv[3];

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);

    /* open the source file */
    infile = fopen(argv[2], "r");
    outfile = fopen(argv[3], "w");

    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[3]);
        exit(4);
    }

    firstPass(infile);
    fclose(infile);
    fclose(outfile);

}

int toNum( char * pStr )
{
    char * t_ptr;
    char * orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if( *pStr == '#' )				/* decimal */
    {
        pStr++;
        if( *pStr == '-' )				/* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    }
    else if( *pStr == 'x' )	/* hex     */
    {
        pStr++;
        if( *pStr == '-' )				/* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
        if( lNeg )
            lNum = -lNum;
        return lNum;
    }
    else
    {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

void firstPass(FILE * lInfile){
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;
    int PC = 0, i = 0;

    do
    {
        lRet = readAndParse( lInfile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            //check .ORIG for address, each line increment PC + 2
            if(strcmp(lOpcode, ".orig") == 0) PC = toNum(lArg1);
            if(strcmp(lLabel, "\0") != 0) {
                for (int j = 0; j < 21; j++) {
                    if (strcmp(lLabel, symbolTable[j].label) == 0) {
                        exit(4);
                    }
                }
                strcpy(symbolTable[i].label, lLabel);
                i++;
                symbolTable[i].address = PC;
            }
            PC += 2;
        }
    } while( lRet != DONE );

    rewind(lInfile);
}

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
)
{
    char * lRet, * lPtr;
    int i;
    if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
        return (DONE);
    for( i = 0; i < strlen( pLine ); i++ )
        pLine[i] = tolower( pLine[i] );
    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while( *lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n' )
        lPtr++;

    *lPtr = '\0';
    if( !(lPtr = strtok( pLine, "\t\r\n ," ) ) )
        return( EMPTY_LINE );

    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
    {
        *pLabel = lPtr;
        if( !( lPtr = strtok( NULL, "\t\r\n ," ) ) ) return( OK );
    }

    *pOpcode = lPtr;

    if( !( lPtr = strtok( NULL, "\t\r\n ," ) ) ) return( OK );

    *pArg1 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\r\n ," ) ) ) return( OK );

    *pArg2 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\r\n ," ) ) ) return( OK );

    *pArg3 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\r\n ," ) ) ) return( OK );

    *pArg4 = lPtr;

    return( OK );
}
int isOpcode(char * opcode){
    if(strcmp(opcode, "add")==0){
        return 0;
    }
    else if(strcmp(opcode, "and")==0){
        return 0;
    }
    else if(strcmp(opcode, "br")==0){
        return 0;
    }
    else if(strcmp(opcode, "brn")==0){
        return 0;
    }
    else if(strcmp(opcode, "brz")==0){
        return 0;
    }
    else if(strcmp(opcode, "brp")==0){
        return 0;
    }
    else if(strcmp(opcode, "brnz")==0){
        return 0;
    }
    else if(strcmp(opcode, "brzp")==0){
        return 0;
    }
    else if(strcmp(opcode, "brnp")==0){
        return 0;
    }
    else if(strcmp(opcode, "brnzp")==0){
        return 0;
    }
    else if(strcmp(opcode, "jmp")==0){
        return 0;
    }
    else if(strcmp(opcode, "jsr")==0){
        return 0;
    }
    else if(strcmp(opcode, "ldb")==0){
        return 0;
    }
    else if(strcmp(opcode, "ldw")==0){
        return 0;
    }
    else if(strcmp(opcode, "lea")==0){
        return 0;
    }
    else if(strcmp(opcode, "rti")==0){
        return 0;
    }
    else if(strcmp(opcode, "shf")==0){
        return 0;
    }
    else if(strcmp(opcode, "stb")==0){
        return 0;
    }
    else if(strcmp(opcode, "stw")==0){
        return 0;
    }
    else if(strcmp(opcode, "trap")==0){
        return 0;
    }
    else if(strcmp(opcode, "xor")==0){
        return 0;
    }
    else if(strcmp(opcode, ".orig")==0){
        return 0;
    }
    else if(strcmp(opcode, ".fill")==0){
        return 0;
    }
    else if(strcmp(opcode, ".end")==0){
        return 0;
    }
    else if(strcmp(opcode, "halt")==0){
        return 0;
    }
    else{
        return -1;
    }
}


/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */