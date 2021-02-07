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
void secondPass(FILE * lInfile, FILE * outFile);
int isOpcode(char * opcode);
void buildMachineCode(char * pOpcode, char * pArg1, char * pArg2, char * pArg3, char * pArg4, FILE * outfile);
int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
);
int isRegister(char * reg);
char * toRegister(char * reg);
void intToBin(int progarg, int * outArr);
char * bin2hex(int instrs[16]);
char * bin2hexchar(char * input);

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
    secondPass(infile, outfile);

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
    //FILE * lInfile;

    //lInfile = fopen( "data.in", "r" );	/* open the input file */

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
                        //TODO: error stuff for duplicate symbol
                    }
                }
                //TODO: figure out if you need to malloc for the symbol table, and see if this correctly adds stuff to it.
                strcpy(symbolTable[i].label, lLabel); //possible error here but idk
                i++;
                symbolTable[i].address = PC;
            }
            PC += 2;
        }
    } while( lRet != DONE );

    rewind(lInfile);
}

void secondPass(FILE * lInfile, FILE * outFile){
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4, *instrstring;
    int lRet;
    int PC = 0, i = 0;
    do{
        lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        buildMachineCode(lOpcode, lArg1, lArg2, lArg3, lArg4, outFile);


    }while (lRet != DONE);
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
    if( !(lPtr = strtok( pLine, "\t\n\r ," ) ) )
        return( EMPTY_LINE );

    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
    {
        *pLabel = lPtr;
        if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );
    }

    *pOpcode = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

    *pArg1 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

    *pArg2 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

    *pArg3 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

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
    else if(strcmp(opcode, "not")==0){
        return 0;
    }
    else if(strcmp(opcode, "ret")==0){
        return 0;
    }
    else if(strcmp(opcode, "rti")==0){
        return 0;
    }
    else if(strcmp(opcode, "lshf")==0){
        return 0;
    }
    else if(strcmp(opcode, "rshfl")==0){
        return 0;
    }
    else if(strcmp(opcode, "rshfa")==0){
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
    else{
        return -1;
    }
}

void buildMachineCode(char * pOpcode, char * pArg1, char * pArg2, char * pArg3, char * pArg4, FILE * outfile) {
    //make sure opcodes are valid

    if(isOpcode(pOpcode) == -1){
        printf("invalid opcode detected!");
        exit(2);
    }

    //check if 4th instruction exists (too many)
    if(strcmp(pArg4,"")!=0){
        exit(4);
    }
    char *tempArg1, *tempArg2, *tempArg3;
    int binInst[16];
    for(int i = 0; i <16; i++){
        binInst[i] = 0;
    }
    int imm_1_val = -9999;
    int imm_1[11];
    int *imm1_t;
    imm1_t = imm_1;

    int imm_2_val;
    int imm_2[11];
    int *imm2_t;
    imm2_t = imm_2;

    int imm_3_val = -9999;
    int imm_3[11];
    int *imm3_t;
    imm3_t = imm_3;

    if (isRegister(pArg1) == 0) {
        tempArg1 = toRegister(pArg1);
    }
    if (isRegister(pArg2) == 0) {
        tempArg2 = toRegister(pArg2);
    }
    if (isRegister(pArg3) == 0) {
        tempArg3 = toRegister(pArg3);
    }

    if(pArg1[0] == '#' || pArg1[0] == 'x') {
        imm_1_val = toNum(pArg1);
        intToBin(imm_1_val, imm1_t);
    }

    if(pArg2[0] == '#' || pArg2[0] == 'x') {
        imm_2_val = toNum(pArg2);
        intToBin(imm_2_val, imm2_t);
    }

    if(pArg3[0] == '#' || pArg3[0] == 'x') {
        imm_3_val = toNum(pArg3);
        intToBin(imm_3_val, imm3_t);
    }



    if (strcmp(pOpcode, ".orig") == 0) {
        if(strcmp(pArg2, "")+strcmp(pArg3, "")+strcmp(pArg4, "")!=0){
            printf("too many parameters");
            exit(4);
        }
        printf("%s\n", pArg1);
        return;

    }

    else if (strcmp(pOpcode, "nop") == 0) {
        printf("0x0000\n");
    }

        //****CODE FOR ADD BELOW****
    else if (strcmp(pOpcode, "add") == 0) {
        if(strcmp(pArg1,"")==0 || strcmp(pArg2,"")==0 || strcmp(pArg3,"")==0){
            printf("error in argument count!");
            exit(4);
        }

        if (isRegister(pArg1) == -1 || isRegister(pArg2) == -1 ) {
            exit(4);
        }

        binInst[0] = 0;
        binInst[1] = 0;
        binInst[2] = 0;
        binInst[3] = 1;
        binInst[4] = tempArg1[0] - 48;
        binInst[5] = tempArg1[1] - 48;
        binInst[6] = tempArg1[2] - 48;


        binInst[7] = tempArg2[0] - 48;
        binInst[8] = tempArg2[1] - 48;
        binInst[9] = tempArg2[2] - 48;


        if (isRegister(pArg3) == 0) {
            binInst[10] = 0;
            binInst[11] = 0;
            binInst[12] = 0;
            binInst[13] = tempArg3[0] - 48;
            binInst[14] = tempArg3[1] - 48;
            binInst[15] = tempArg3[2] - 48;
        } else if ((isRegister(pArg3) == -1)) {
            if (imm_3_val > 15 || imm_3_val < -16) exit(3);
            binInst[10] = 1;
            binInst[11] = imm_3[6];
            binInst[12] = imm_3[7];
            binInst[13] = imm_3[8];
            binInst[14] = imm_3[9];
            binInst[15] = imm_3[10];
        } else exit(4);

    }



    //****CODE FOR AND BELOW****
    else if (strcmp(pOpcode, "and") == 0) {
        binInst[0] = 0;
        binInst[1] = 1;
        binInst[2] = 0;
        binInst[3] = 1;
        if (isRegister(pArg1) == -1) {
            exit(4);
        }
        binInst[4] = tempArg1[0] - 48;
        binInst[5] = tempArg1[1] - 48;
        binInst[6] = tempArg1[2] - 48;

        if (isRegister(pArg2) == -1) { //SR1
            exit(4);
        } else {
            binInst[7] = tempArg2[0] - 48;
            binInst[8] = tempArg2[1] - 48;
            binInst[9] = tempArg2[2] - 48;
        }

        if (isRegister(pArg3) == 0) {
            binInst[10] = 0;
            binInst[11] = 0;
            binInst[12] = 0;
            binInst[13] = tempArg3[0] - 48;
            binInst[14] = tempArg3[1] - 48;
            binInst[15] = tempArg3[2] - 48;
        } else if ((isRegister(pArg3) == -1)) {
            if (imm_3_val > 15 || imm_3_val < -16) exit(1);
            binInst[10] = 1;
            binInst[11] = imm_3[6];
            binInst[12] = imm_3[7];
            binInst[13] = imm_3[8];
            binInst[14] = imm_3[9];
            binInst[15] = imm_3[10];
        } else exit(4);
        for (int i = 1; i <= 16; i++) {
            printf("%x", binInst[i - 1]);
            if (i % 4 == 0) printf(" ");
        }
        printf("\n");
    }

        //****CODE FOR XOR BELOW****
    else if (strcmp(pOpcode, "xor") == 0) {
        if(strcmp(pArg1,"")==0 || strcmp(pArg2,"")==0 || strcmp(pArg3,"")==0){
            printf("error in argument count!");
            exit(4);
        }
        binInst[0] = 1;
        binInst[1] = 0;
        binInst[2] = 0;
        binInst[3] = 1;
        if (isRegister(pArg1) == -1) {
            exit(4);
        }
        binInst[4] = tempArg1[0] - 48;
        binInst[5] = tempArg1[1] - 48;
        binInst[6] = tempArg1[2] - 48;

        if (isRegister(pArg2) == -1) { //SR1
            exit(4);
        } else {
            binInst[7] = tempArg2[0] - 48;
            binInst[8] = tempArg2[1] - 48;
            binInst[9] = tempArg2[2] - 48;
        }

        if (isRegister(pArg3) == 0) {
            binInst[10] = 0;
            binInst[11] = 0;
            binInst[12] = 0;
            binInst[13] = tempArg3[0] - 48;
            binInst[14] = tempArg3[1] - 48;
            binInst[15] = tempArg3[2] - 48;
        } else if ((isRegister(pArg3) == -1) && (imm_3_val != -9999)) {
            if (imm_3_val > 15 || imm_3_val < -16) exit(3);
            binInst[10] = 1;
            binInst[11] = imm_3[6];
            binInst[12] = imm_3[7];
            binInst[13] = imm_3[8];
            binInst[14] = imm_3[9];
            binInst[15] = imm_3[10];
        } else exit(4);

    }

    //**** CODE FOR RTI BELOW****
    else if (strcmp(pOpcode, "rti") == 0) {
        if(strcmp(pArg1, "")+strcmp(pArg2, "")+strcmp(pArg3, "")+strcmp(pArg4, "")!=0){
            printf("too many parameters");
            exit(4);
        }
        binInst[0] = 1;
        binInst[1] = 0;
        binInst[2] = 0;
        binInst[3] = 0;
        binInst[4] = 0;
        binInst[5] = 0;
        binInst[6] = 0;
        binInst[7] = 0;
        binInst[8] = 0;
        binInst[9] = 0;
        binInst[10] = 0;
        binInst[11] = 0;
        binInst[12] = 0;
        binInst[13] = 0;
        binInst[14] = 0;
        binInst[15] = 0;
    }

    //****CODE FOR JMP BELOW****
    else if(strcmp(pOpcode, "jmp")==0){
        if(strcmp(pArg2, "")+strcmp(pArg3, "")+strcmp(pArg4, "")!=0){
            printf("too many parameters");
            exit(4);
        }
        else if(strcmp(pArg1,"")==0 || isRegister(pArg1)== -1){
            printf("error in arguments!");
            exit(4);
        }
        binInst[0] = 1;
        binInst[1] = 1;
        binInst[2] = 0;
        binInst[3] = 0;

        binInst[4] = 0;
        binInst[5] = 0;
        binInst[6] = 0;

        binInst[7] = tempArg1[0]-48;
        binInst[8] = tempArg1[1]-48;
        binInst[9] = tempArg1[2]-48;

        binInst[10] = 0;
        binInst[11] = 0;
        binInst[12] = 0;
        binInst[13] = 0;
        binInst[14] = 0;
        binInst[15] = 0;

    }

    //****CODE FOR JSRR BELOW****
    else if(strcmp(pOpcode, "jsrr")==0){
        if(strcmp(pArg2, "")+strcmp(pArg3, "")+strcmp(pArg4, "")!=0){
            printf("too many parameters");
            exit(4);
        }
        else if(strcmp(pArg1,"")==0 || isRegister(pArg1)== -1){
            printf("error in arguments!");
            exit(4);
        }
        binInst[0] = 0;
        binInst[1] = 1;
        binInst[2] = 0;
        binInst[3] = 0;

        binInst[4] = 0;
        binInst[5] = 0;
        binInst[6] = 0;

        binInst[7] = tempArg1[0]-48;
        binInst[8] = tempArg1[1]-48;
        binInst[9] = tempArg1[2]-48;

        binInst[10] = 0;
        binInst[11] = 0;
        binInst[12] = 0;
        binInst[13] = 0;
        binInst[14] = 0;
        binInst[15] = 0;

    }

    //**** CODE FOR LDB BELOW****
    else if(strcmp(pOpcode, "ldb")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || imm_3_val == -9999){
            printf("dr sr not specified or no offset");
            exit(4);
        }
        else if(imm_3_val > 31 || imm_3_val < -32){
            printf("out of allowed range");
            exit(3);
        }
        binInst[0] = 0;
        binInst[1] = 0;
        binInst[2] = 1;
        binInst[3] = 0;

        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        binInst[10] = imm_3[5];
        binInst[11] = imm_3[6];
        binInst[12] = imm_3[7];
        binInst[13] = imm_3[8];
        binInst[14] = imm_3[9];
        binInst[15] = imm_3[10];
    }

    //**** CODE FOR LDW BELOW****
    else if(strcmp(pOpcode, "ldw")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || imm_3_val == -9999){
            printf("dr sr not specified or no offset");
            exit(4);
        }
        else if(imm_3_val > 31 || imm_3_val < -32){
            printf("out of allowed range");
            exit(3);
        }
        binInst[0] = 0;
        binInst[1] = 1;
        binInst[2] = 1;
        binInst[3] = 0;

        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        binInst[10] = imm_3[5];
        binInst[11] = imm_3[6];
        binInst[12] = imm_3[7];
        binInst[13] = imm_3[8];
        binInst[14] = imm_3[9];
        binInst[15] = imm_3[10];
    }

    //****CODE FOR NOT BELOW****
    else if(strcmp(pOpcode,"not")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || strcmp(pArg3,"")!=0){
            printf("incorrect parameters");
            exit(4);
        }
        binInst[0] = 1;
        binInst[1] = 0;
        binInst[2] = 0;
        binInst[3] = 1;

        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        binInst[10] = 1;
        binInst[11] = 1;
        binInst[12] = 1;
        binInst[13] = 1;
        binInst[14] = 1;
        binInst[15] = 1;
    }

    //**** CODE FOR RET BELOW****
    else if (strcmp(pOpcode, "ret") == 0) {
        if(strcmp(pArg1, "")+strcmp(pArg2, "")+strcmp(pArg3, "")!=0){
            printf("too many parameters");
            exit(4);
        }
        binInst[0] = 1;
        binInst[1] = 1;
        binInst[2] = 0;
        binInst[3] = 0;
        binInst[4] = 0;
        binInst[5] = 0;
        binInst[6] = 0;
        binInst[7] = 1;
        binInst[8] = 1;
        binInst[9] = 1;
        binInst[10] = 0;
        binInst[11] = 0;
        binInst[12] = 0;
        binInst[13] = 0;
        binInst[14] = 0;
        binInst[15] = 0;
    }

    //**** CODE FOR LSHF BELOW****
    else if(strcmp(pOpcode, "lshf")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || imm_3_val == -9999){
            printf("dr sr not specified or no offset");
            exit(4);
        }
        else if(imm_3_val > 15 || imm_3_val < 0){
            printf("out of allowed range");
            exit(3);
        }

        binInst[0] = 1;
        binInst[1] = 1;
        binInst[2] = 0;
        binInst[3] = 1;

        //DR
        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        //SR
        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        //ctrl
        binInst[10] = 0;
        binInst[11] = 0;

        //amount4
        binInst[12] = imm_3[7];
        binInst[13] = imm_3[8];
        binInst[14] = imm_3[9];
        binInst[15] = imm_3[10];
    }

    //**** CODE FOR RSHFL BELOW****
    else if(strcmp(pOpcode, "rshfl")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || imm_3_val == -9999){
            printf("dr sr not specified or no offset");
            exit(4);
        }
        else if(imm_3_val > 15 || imm_3_val < 0){
            printf("out of allowed range");
            exit(3);
        }

        binInst[0] = 1;
        binInst[1] = 1;
        binInst[2] = 0;
        binInst[3] = 1;

        //DR
        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        //SR
        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        //ctrl
        binInst[10] = 0;
        binInst[11] = 1;

        //amount4
        binInst[12] = imm_3[7];
        binInst[13] = imm_3[8];
        binInst[14] = imm_3[9];
        binInst[15] = imm_3[10];

    }

    //****CODE FOR RSHFA BELOW****
    else if(strcmp(pOpcode, "rshfa")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || imm_3_val == -9999){
            printf("dr sr not specified or no offset");
            exit(4);
        }
        else if(imm_3_val > 15 || imm_3_val < 0){
            printf("out of allowed range");
            exit(3);
        }

        binInst[0] = 1;
        binInst[1] = 1;
        binInst[2] = 0;
        binInst[3] = 1;

        //DR
        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        //SR
        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        //ctrl
        binInst[10] = 1;
        binInst[11] = 1;

        //amount4
        binInst[12] = imm_3[7];
        binInst[13] = imm_3[8];
        binInst[14] = imm_3[9];
        binInst[15] = imm_3[10];
    }

    //**** CODE FOR STB BELOW****
    else if(strcmp(pOpcode, "STB")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || imm_3_val == -9999){
            printf("dr sr not specified or no offset");
            exit(4);
        }
        else if(imm_3_val > 31 || imm_3_val < -32){
            printf("out of allowed range");
            exit(3);
        }
        binInst[0] = 0;
        binInst[1] = 0;
        binInst[2] = 1;
        binInst[3] = 1;

        //dr
        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        //sr
        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        //boffset6
        binInst[10] = imm_3[5];
        binInst[11] = imm_3[6];
        binInst[12] = imm_3[7];
        binInst[13] = imm_3[8];
        binInst[14] = imm_3[9];
        binInst[15] = imm_3[10];
    }

    //**** CODE FOR STW BELOW****
    else if(strcmp(pOpcode, "stw")==0){
        if(isRegister(pArg1)== -1 || isRegister(pArg2)==-1 || imm_3_val == -9999){
            printf("dr sr not specified or no offset");
            exit(4);
        }
        else if(imm_3_val > 31 || imm_3_val < -32){
            printf("out of allowed range");
            exit(3);
        }
        binInst[0] = 0;
        binInst[1] = 1;
        binInst[2] = 1;
        binInst[3] = 1;

        //dr
        binInst[4] = tempArg1[0]-48;
        binInst[5] = tempArg1[1]-48;
        binInst[6] = tempArg1[2]-48;

        //sr
        binInst[7] = tempArg2[0]-48;
        binInst[8] = tempArg2[1]-48;
        binInst[9] = tempArg2[2]-48;

        //boffset6
        binInst[10] = imm_3[5];
        binInst[11] = imm_3[6];
        binInst[12] = imm_3[7];
        binInst[13] = imm_3[8];
        binInst[14] = imm_3[9];
        binInst[15] = imm_3[10];
        for (int i = 1; i <= 16; i++) {
            printf("%x", binInst[i - 1]);
            if (i % 4 == 0) printf(" ");
        }
        printf("\n");
    }

    //****CODE FOR TRAP BELOW****
    else if(strcmp(pOpcode, "trap")==0){
        if(strcmp(pArg2,"")!=0 || strcmp(pArg3,"")!=0 || strcmp(pArg1,"") == 0){
            printf("invalid paramaters");
            exit(4);
        }
        else if(imm_1_val < 0 || imm_1_val > 255){
            printf("invalid trap vector");
            exit(3);
        }
        binInst[0] = 1;
        binInst[1] = 1;
        binInst[2] = 1;
        binInst[3] = 1;
        binInst[4] = 0;
        binInst[5] = 0;
        binInst[6] = 0;
        binInst[7] = 0;

        //trap vector
        binInst[8] = imm_1[3];
        binInst[9] = imm_1[4];
        binInst[10] = imm_1[5];
        binInst[11] = imm_1[6];
        binInst[12] = imm_1[7];
        binInst[13] = imm_1[8];
        binInst[14] = imm_1[9];
        binInst[15] = imm_1[10];

    }

    if(strcmp(pOpcode, ".end")==0){
        exit(0);
    }
    char * hexInst;


    if(strcmp(pOpcode, ".orig")!=0){
        //TODO: CONVERSION OF 16BIT ARR TO STRING GOES HERE
    }
    //fprintf(outfile, "\n");
    printf("\n");
}

int isRegister(char * reg){
    if(strcmp(reg, "r0") == 0) return 0;
    else if(strcmp(reg, "r1") == 0) return 0;
    else if(strcmp(reg, "r2") == 0) return 0;
    else if(strcmp(reg, "r3") == 0) return 0;
    else if(strcmp(reg, "r4") == 0) return 0;
    else if(strcmp(reg, "r5") == 0) return 0;
    else if(strcmp(reg, "r6") == 0) return 0;
    else if(strcmp(reg, "r7") == 0) return 0;
    else return -1;
}

char * toRegister(char * reg){
    if(strcmp(reg, "r0") == 0) return "000";
    else if(strcmp(reg, "r1") == 0) return "001";
    else if(strcmp(reg, "r2") == 0) return "010";
    else if(strcmp(reg, "r3") == 0) return "011";
    else if(strcmp(reg, "r4") == 0) return "100";
    else if(strcmp(reg, "r5") == 0) return "101";
    else if(strcmp(reg, "r6") == 0) return "110";
    else if(strcmp(reg, "r7") == 0) return "111";
    else return "";
}

void intToBin(int progarg, int * outArr){
    int trialnum;

    trialnum = progarg;
    if(progarg < 0){
        trialnum = progarg* -1;
    }

    //printf("trialnum: %d\n", trialnum);

    int binArr[11];
    for(int i = 0; i < 11; i++){
        binArr[i] = 0;
    }
    int pos = 10;
    while(trialnum >0){
        binArr[pos] = trialnum % 2;
        trialnum = trialnum / 2;
        pos--;
    }

    if(progarg < 0){
        int twocomp[11];
        int carry = 1;

        //Convert to One's Comp
        for(int i = 0; i < 11; i++){
            if(binArr[i] == 0){
                binArr[i] = 1;
            }
            else if(binArr[i] == 1){
                binArr[i] = 0;
            }
            //printf("%d", binArr[i]);
        }
        //printf("\n");

        for(int i = 10; i >= 0; i--){
            if(binArr[i] == 1 && carry == 1) twocomp[i] = 0;
            else if(binArr[i] == 0 && carry == 1){
                twocomp[i] = 1; carry = 0;
            }
            else twocomp[i] = binArr[i];
        }
        for(int j = 0; j < 11; j++){
            *(outArr+j) = twocomp[j];
        }

    }
    else{
        for(int j = 0; j < 11; j++){
            *(outArr+j) = binArr[j];
        }
    }

}
/*
char * bin2hex(int instrs[16]){
    char nibble[4];
    char * nibble_t;
    //char * instrstring = NULL;
    char* instrstring = "0x";
    for(int i = 0; i < 4; i++){
        nibble[i] = instrs[i]+48;
    }
    strcat(instrstring, bin2hexchar(nibble_t));/*

    for(int i = 4; i < 7; i++){
        nibble[i] = instrs[i]+48;
    }
    strcat(instrstring, bin2hexchar(nibble));

    for(int i = 8; i < 11; i++){
        nibble[i] = instrs[i]+48;
    }
    strcat(instrstring, bin2hexchar(nibble));

    for(int i = 12; i < 15; i++){
        nibble[i] = instrs[i]+48;
    }
    strcat(instrstring, bin2hexchar(nibble));
    printf("%s\n", instrstring);

    return instrstring;
}

char * bin2hexchar(char * input){
    if(strcmp(input, "0000")) return "0";
    if(strcmp(input, "0000")) return "1";
    if(strcmp(input, "0000")) return "2";
    if(strcmp(input, "0000")) return "3";
    if(strcmp(input, "0000")) return "4";
    if(strcmp(input, "0000")) return "5";
    if(strcmp(input, "0000")) return "6";
    if(strcmp(input, "0000")) return "7";
    if(strcmp(input, "0000")) return "8";
    if(strcmp(input, "0000")) return "9";
    if(strcmp(input, "0000")) return "A";
    if(strcmp(input, "0000")) return "B";
    if(strcmp(input, "0000")) return "C";
    if(strcmp(input, "0000")) return "D";
    if(strcmp(input, "0000")) return "E";
    if(strcmp(input, "0000")) return "F";


}
*/