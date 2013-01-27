; **************************************************************
;
; PicProbe - Yet Another Logic Probe
;
; This piece of code is Copyright 1997 
; by Alessandro Zummo, azummo@ita.flashnet.it
; and should not be used in any commercial
; application without my written permission.
;
; It uses a led to tell you the logic state on the probe.
; If the led is fully on, then you've a logic one
; on the input, if is off you obviusly have a zero
; and if is dimly at a 50% pwm rate the input is hi-z.
; This type of implementation requires an interrupt
; capable pic. I can change the code to use two
; leds or even a 7-segment display...just ask me.
;
; The schematic is quite simple:
; Connect the probe to PSENSE and a 100kohm resistor
; between it and PHZS. Then connect a led and
; his resistor between PLED and GND
; (Obviusly led anode is connected to PLED).
;
; Bugs: The high-z state is sometimes not recognized
;
;
; Now it's the greetings time:
;
; Peter H. Anderson:	His site was a great help 
;						when i had troubles in
; 						pic programming. 
;
; The Piclisters:		Their help was and still is 
;						invaluable... The PicList is probably 
;						the best mailing list i've ever seen.
;
; $VER: PicProbe 1.0 (26.1.98)								
; ************************************************************** 

	list p=16C84, s=off

    device wdt_off,xt_osc


	include <PINCLUDE:p16c84.inc>


; Some definitions, 

PPORT	equ	PORTB
PHZS	equ	7
PSENSE	equ	6
PLED	equ	5


	ORG 0x000				; Reset vector
	goto	main

	ORG 0x004				; Interrupt vector

	btfss	PPORT, PLED		; Blink the led
	bsf		PPORT, PLED		
	btfsc	PPORT, PLED
	bcf		PPORT, PLED

	bsf		STATUS, RP0		; Switch to page 1
	bcf		INTCON, T0IF	; Clear the tmr0 interrupt flag
	bcf		STATUS, RP0		; Back to page 0

	movlw	0xFF/2			; Set-up a 50% duty cycle
	movwf	TMR0

	retfie

main:

	bsf		STATUS, RP0		; Switch to page 1

	bcf		PPORT, PHZS		; out
	bsf		PPORT, PSENSE	; in
	bcf		PPORT, PLED 	; out

	bcf		OPTION_REG, T0CS; Select Timer mode

	bcf		INTCON, T0IF	; Clear tmr0 interrupt flag
	bsf		INTCON, GIE		; General interrupt enable

	bcf		STATUS, RP0		; Back to page 0

:loop

	bcf		PPORT, PHZS		; PHZS is now low...

	btfsc	PPORT, PSENSE	; Test if PSENSE is equal to PHZS
	goto	:nothz		

	bsf		PPORT, PHZS 	; ...and then high.

	btfss	PPORT, PSENSE	; Test if PSENSE is equal to PHZS
	goto	:nothz		

:hz							; Input is Hi-Z

	bsf		STATUS, RP0		; Switch to page 1
	bsf		INTCON, T0IE	; Enable timer overflow interrupt
	bcf		STATUS, RP0		; Back to page 0

	goto	:loop

:nothz						; Input isn't Hi-Z

	bcf		PPORT, PHZS		; Put PHZS low, to allow further tests

	bsf		STATUS, RP0		; Switch to page 1
	bcf		INTCON, T0IE	; Disable timer overflow interrupt
	bcf		STATUS, RP0		; Back to page 0

	btfss	PPORT, PSENSE
	goto	:low

:high
	
	bsf		PPORT, PLED		; Input is high, led is full on
	goto	:loop

:low

	bcf		PPORT, PLED		; Inout is low, led is off
	goto	:loop

	end
