;**********************************************************************
; HAPPY BIRTHDAY TUNE.                                         
; 7THE DATA TABLES SET THE FREQUENCY AND THE DURATION OF THE NOTE.
; TABLE TBLT5 IS THE PAUSE BETWEEN NOTES.
; DELAY1 IS 50MS, IT SIMULATES END OF NOTE.                                 
; THE OUTPUT IS FROM 2 PORTS AT OPOSITE PHASE TO GIVE DOUBLE THE VOLTAGE.
;**********************************************************************
list	p=12F675
#INCLUDE <p12F675.inc>
;==========================================================================
; Configuration Bits
_CPD_ON                      EQU     H'3EFF'
_CPD_OFF                     EQU     H'3FFF'
_CP_ON                       EQU     H'3F7F'
_CP_OFF                      EQU     H'3FFF'
_BODEN_ON                    EQU     H'3FFF'
_BODEN_OFF                   EQU     H'3FBF'
_MCLRE_ON                    EQU     H'3FFF'
_MCLRE_OFF                   EQU     H'3FDF'
_PWRTE_OFF                   EQU     H'3FFF'
_PWRTE_ON                    EQU     H'3FEF'
_WDT_ON                      EQU     H'3FFF'
_WDT_OFF                     EQU     H'3FF7'
_LP_OSC                      EQU     H'3FF8'
_XT_OSC                      EQU     H'3FF9'
_HS_OSC                      EQU     H'3FFA'
_EC_OSC                      EQU     H'3FFB'
_INTRC_OSC_NOCLKOUT          EQU     H'3FFC'
_INTRC_OSC_CLKOUT            EQU     H'3FFD'
_EXTRC_OSC_NOCLKOUT          EQU     H'3FFE'
_EXTRC_OSC_CLKOUT            EQU     H'3FFF'

 __CONFIG   _CP_OFF & _CPD_OFF & _BODEN_OFF & _MCLRE_OFF & _WDT_OFF & _PWRTE_ON & _INTRC_OSC_NOCLKOUT 

#define	OP1	      GPIO,0     ;OUTPUT
#define	OP2	      GPIO,1     ;OUTPUT

TIMER1  	EQU	.32  		
TIMER2  	EQU	.33		
LONG        EQU .34
PCOUNT      EQU .35
PAUSE       EQU .36

LN		EQU .8      ;LONG NOTE
SN      EQU .5      ;SHORT NOTE 
NG		EQU .166    ;NOTE G FREQUENCY
NA		EQU .120    ;NOTE A FREQUENCY=440Hz
NB      EQU .79
NC		EQU .60
ND      EQU .25
NE		EQU .252
NF      EQU .238
NG1     EQU .212

		ORG     0x000             ; processor reset vector
		goto    init              ; go to beginning of program
	
DELAY5                         ;0.5 SEC DELAY   
        MOVLW	LN
        MOVWF   LONG
        BSF		T1CON,TMR1ON
LOOP5        
        BTFSS	PIR1,TMR1IF
		GOTO	LOOP5
		BCF		PIR1,TMR1IF
		DECFSZ  LONG,F
		GOTO	LOOP5
		BCF		T1CON,TMR1ON
		CLRF    TMR1L
		CLRF    TMR1H
		RETURN
		
DELAY1        
        movlw	 .238     ;50 MS DELAY
    	movwf	 TIMER1
    	movlw	 .65
    	movwf	 TIMER2
LP1     decfsz	 TIMER1, f
		goto	 LP1
		decfsz	 TIMER2, f
		goto	 LP1
		RETURN					
        
TBLDUR   ;NOTES DURATION DATA 
        addwf   PCL,f         
        dt      SN,SN,LN,LN,LN,LN, SN,SN,LN,LN,LN,LN, SN,SN,LN,LN,LN,LN,LN, SN,SN,LN,LN,LN,LN

TBLT1   ;TIMER1 VALUES RESPECTIVE TO NOTES 
        addwf   PCL,f         
        dt		NG,NG,NA,NG,NC,NB, NG,NG,NA,NG,ND,NC, NG,NG,NG1,NE,NC,NB,NA, NF,NF,NE,NC,ND,NC  
              
TBLT2   ;TIMER2 VALUES RESPECTIVE TO NOTES 
        addwf   PCL,f         
        dt		.2,.2,.2,.2,.2,.2, .2,.2,.2,.2,.2,.2, .2,.2,.1,.1,.2,.2,.2, .1,.1,.1,.2,.2,.2 
        
TBLT5   ;.1=DELAY5
        addwf   PCL,f         
        dt		.0,.0,.0,.0,.0,.1, .0,.0,.0,.0,.0,.1, .0,.0,.0,.0,.0,.0,.1, .0,.0,.0,.0,.0,.1
        
init
		bsf     STATUS,RP0        ; set file register bank to 1
		clrf	ANSEL			  ; ANSEL (12F675 only)
		call    0x3FF             ; retrieve factory calibration value 
		movwf   OSCCAL            ; update register with factory cal value 
		MOVLW	B'0000'           
		MOVWF	TRISIO		      ;GP2,4 outputs. 1=input 0=output
		BCF	    STATUS,RP0	      ;Switch Back to reg. Bank 0
		MOVLW	B'00000111'	      ;Disable Comparator module's
		MOVWF	CMCON		
		CLRF    T1CON
			
		BCF		PIR1,TMR1IF
		CLRF    PCOUNT
		CLRF    PAUSE
MUSIC
		MOVFW	PCOUNT      ;NOTES PLAY COUNT
		CALL    TBLDUR		;SET DURATION
        MOVWF   LONG
        MOVFW	PCOUNT
		CALL    TBLT5		;CHECK IF DELAY5
        MOVWF   PAUSE
        BSF		T1CON,TMR1ON
LOOP1        
        BSF     OP1			;OUTPUT
        BCF		OP2
        MOVFW	PCOUNT      ;SET NOTE FREQUENCY
		CALL    TBLT1
        MOVWF   TIMER1
        MOVFW	PCOUNT
		CALL    TBLT2
        MOVWF   TIMER2
LOOP2   decfsz	 TIMER1, f
		goto	 LOOP2
		decfsz	 TIMER2, f
		goto	 LOOP2
		
		BCF		OP1			;OUTPUT
		BSF		OP2			
	    MOVFW	PCOUNT      ;SET NOTE FREQUENCY
		CALL    TBLT1
        MOVWF   TIMER1
        MOVFW	PCOUNT
		CALL    TBLT2
        MOVWF   TIMER2
LOOP3   decfsz	 TIMER1, f
		goto	 LOOP3
		decfsz	 TIMER2, f
		goto	 LOOP3
		
		BTFSS	PIR1,TMR1IF	;LOOP FOR 64MS
		GOTO	LOOP1
		BCF		PIR1,TMR1IF
		DECFSZ  LONG,F
		GOTO	LOOP1			;LOOP FOR 64 * NOTE DURATION 
		BCF		T1CON,TMR1ON
		CLRF    TMR1L
		CLRF    TMR1H		
		CALL    DELAY1
		BTFSC   PAUSE,0			;IS DELAY5 ON 
		CALL    DELAY5
		
		INCF    PCOUNT,F		;INCR NOTES COUNTER
		MOVLW   .25				;NUMBER OF NOTES IN THE TUNE	
		SUBWF	PCOUNT,W
		BTFSS   STATUS,Z        ;
		GOTO    MUSIC
		CLRF    PCOUNT
		GOTO    MUSIC
		END

