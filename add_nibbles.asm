.ORIG x3000
    LEA R0,A;
    LDW R0,R0,x0;
    LDB R1,R0,x0;

    LEA R2,M1;
    LDW R2,R2,x0;
    AND R3,R2,R1;
    RSHFA R3,R3,#4;

    LEA R2,M2;
    LDW R2,R2,x0;
    AND R4,R2,R1;

    ADD R3,R3,R4;
    AND R4,R3,x8;
    ADD R4,R4,x-8;
    BRNP DONE
    ADD R3,R4,R3;

DONE LEA R4,Y;
     LDW R4,R4,x0;
     STB R3,R4,x0;
     HALT

A   .FILL x3050;
Y   .FILL x3051;
M1  .FILL x00F0;
M2  .FILL x000F;
M3  .FILL xFFF0;
.END