      RSEG CODE:CODE
      CODE32

  ;*ss++=((k&0xf80000)>>8)|((k&0xfc00)>>5)|((k&0xf8)>>3);      
      PUBLIC redraw ;R0 TO, R1 FROM, R2 cnt  ;, R2 FLAG, R4=I, R5=J, R6=K
redraw      
      PUSH {R3-R5,LR}
redraw1  
      LDR R3,[R1],#4
      AND R4,R3,#0xf80000
      MOV R5,R4,LSR #8
      
      AND R4,R3,#0xfc00
      LSR R4,R4,#5
      ORR R5,R5,R4

      AND R4,R3,#0xf8
      LSR R4,R4,#3
      ORR R5,R5,R4
      
      STRH R5,[R0],#2
      SUBS R2,R2,#1
      BNE  redraw1

      POP {R3-R5,PC}


      PUBLIC filter ;R0 TO, R1 FROM, ;//R2 cnt  ;, R2 FLAG, R4=I, R5=J, R6=K
filter      
      PUSH {R2-R10,LR}
      MOV R4,#660
      MOV R3,#0
fillbb      
      STRH R3,[R0],#2
      SUBS R4,R4,#1
      BNE fillbb
      
      
      MOV R4,#120
fil1
      MOV R3,#128
fil2      
      ADD R2, R1, #512
      LDRH R5,[R1],#2
      LDRH R6,[R1],#2
      LDRH R7,[R2],#2
      LDRH R8,[R2]
      
      AND R9,R5,#0xf800
      AND R2,R6,#0xf800
      ADD R9,R9,R2
      AND R2,R7,#0xf800
      ADD R9,R9,R2
      AND R2,R8,#0xf800
      ADD R9,R9,R2
      LSR R9,R9,#2
      AND R10,R9,#0xf800
      
      AND R9,R5,#0x7c0
      AND R2,R6,#0x7c0
      ADD R9,R9,R2
      AND R2,R7,#0x7c0
      ADD R9,R9,R2
      AND R2,R8,#0x7c0
      ADD R9,R9,R2
      LSR R9,R9,#2
      AND R9,R9,#0x7c0
      ADD R10,R10,R9

      AND R9,R5,#0x1f
      AND R2,R6,#0x1f
      ADD R9,R9,R2
      AND R2,R7,#0x1f
      ADD R9,R9,R2
      AND R2,R8,#0x1f
      ADD R9,R9,R2
      LSR R9,R9,#2
      AND R9,R9,#0x1f
      ADD R10,R10,R9

      STRH R10,[R0],#2

      SUBS R3,R3,#1
      BNE  fil2
      
      ADD R0,R0,#8
      ADD R1,R1,#512
      SUBS R4,R4,#1
      BNE  fil1

      POP {R2-R10,PC}

   
   PUBLIC filter2 ;R0 TO, R1 FROM, ;//R2 cnt  ;, R2 FLAG, R4=I, R5=J, R6=K
filter2      
      PUSH {R2-R10,LR}
      MOV R4,#528
      MOV R3,#0
fillb      
      STR R3,[R0],#4
      SUBS R4,R4,#1
      BNE fillb
      
      MOV R4,#160
fil1a
      MOV R3,#128
fil2a      
      ADD R2, R1, #512
      LDRH R5,[R1],#2
      LDRH R6,[R1],#2
      LDRH R7,[R2],#2
      LDRH R8,[R2]
      
      AND R9,R5,#0xf800
      AND R2,R6,#0xf800
      ADD R9,R9,R2
      AND R2,R7,#0xf800
      ADD R9,R9,R2
      AND R2,R8,#0xf800
      ADD R9,R9,R2
      LSR R9,R9,#2
      AND R10,R9,#0xf800
      
      AND R9,R5,#0x7c0
      AND R2,R6,#0x7c0
      ADD R9,R9,R2
      AND R2,R7,#0x7c0
      ADD R9,R9,R2
      AND R2,R8,#0x7c0
      ADD R9,R9,R2
      LSR R9,R9,#2
      AND R9,R9,#0x7c0
      ADD R10,R10,R9

      AND R9,R5,#0x1f
      AND R2,R6,#0x1f
      ADD R9,R9,R2
      AND R2,R7,#0x1f
      ADD R9,R9,R2
      AND R2,R8,#0x1f
      ADD R9,R9,R2
      LSR R9,R9,#2
      AND R9,R9,#0x1f
      ADD R10,R10,R9

      STRH R10,[R0],#2

      SUBS R3,R3,#1
      BNE  fil2a
      
      ADD R0,R0,#8
      ANDS R9,R4,#1
      ADDNE R1,R1,#512
      //ADDEQ R1,R1,#256

      SUBS R4,R4,#1
      BNE  fil1a

      POP {R2-R10,PC}


   END