//#include <avr/pgmspace.h>

#define DGSTR_SZ					128 //tamanho do buffer de strings temp

#define W0							void
#define W1							bool
#define W8							unsigned char
#define SW8							char
#define FP							float
#define W32							unsigned long
#define SW32						signed long
#define SW16						int
#define W16							unsigned
#define VL_PACK_BEGIN 
#define VL_PACK_MIDDLE              
#define VL_PACK_END		            __attribute__((packed))  
#define VL_FLASH_READ_BYTE(X_,IDX_)	pgm_read_byte((X_)+(IDX_)) 
#define VL_FLASH_READ_WORD(X_,IDX_)	pgm_read_word((X_)+(IDX_)) 
#define VL_ALIGN(X_)				__attribute__((aligned(X_))) 
#define VL_STORE_IN_FLASH			PROGMEM
#define DELAY_US(X_)				_delay_us(X_); 
#define DELAY_MS(X_)				delay(X_);

#define IO_PIN_INFO_2		PORTD
#define IO_PIN_INFO_BIT2	2
#define IO_PIN_INFO_3		PORTD
#define IO_PIN_INFO_BIT3	3
#define IO_PIN_INFO_4		PORTD
#define IO_PIN_INFO_BIT4	4
#define IO_PIN_INFO_5		PORTD
#define IO_PIN_INFO_BIT5	5
#define IO_PIN_INFO_6		PORTD
#define IO_PIN_INFO_BIT6	6
#define IO_PIN_INFO_7		PORTD
#define IO_PIN_INFO_BIT7	7
#define IO_PIN_INFO_8		PORTB
#define IO_PIN_INFO_BIT8	0
#define IO_PIN_INFO_9		PORTB
#define IO_PIN_INFO_BIT9	1
#define IO_PIN_INFO_10		PORTB
#define IO_PIN_INFO_BIT10	2
#define IO_PIN_INFO_11		PORTB
#define IO_PIN_INFO_BIT11	3
#define IO_PIN_INFO_12		PORTB
#define IO_PIN_INFO_BIT12	4
#define IO_PIN_INFO_13		PORTB
#define IO_PIN_INFO_BIT13	5
#define IO_PIN_INFO_19		PORTA
#define IO_PIN_INFO_BIT19	0
#define IO_PIN_INFO_20		PORTA
#define IO_PIN_INFO_BIT20	1
#define IO_PIN_INFO_21		PORTA
#define IO_PIN_INFO_BIT21	2
#define IO_PIN_INFO_22		PORTA
#define IO_PIN_INFO_BIT22	3
#define IO_PIN_INFO_23		PORTA
#define IO_PIN_INFO_BIT23	4
#define IO_PIN_INFO_24		PORTA
#define IO_PIN_INFO_BIT24	5
#define IO_PIN_INFO_25		PORTA
#define IO_PIN_INFO_BIT25	6
#define IO_PIN_INFO_26		PORTA
#define IO_PIN_INFO_BIT25	7
#define IO_PIN_INFO_28		PORTB
#define IO_PIN_INFO_BIT28	6

							// n. no arduino
							// |  nome porta
							// |	|	bit da porta
							// |	|	 |	
							// v	v	 V	
#define IO_PIN_2	2
#define IO_PIN_3	3
#define IO_PIN_4	4
#define IO_PIN_5	5
#define IO_PIN_6	6
#define IO_PIN_7	7
#define IO_PIN_8	8
#define IO_PIN_9	9   
#define IO_PIN_10	10
#define IO_PIN_11	11
#define IO_PIN_12	12
#define IO_PIN_13	13
#define IO_PIN_19	A0//19
#define IO_PIN_20	A1//20
#define IO_PIN_21	A2//21
#define IO_PIN_22	A3//22
#define IO_PIN_23	A4//23
#define IO_PIN_24	A5//24
#define IO_PIN_25	A6//25
#define IO_PIN_26	A7//26
#define IO_PIN_28	28
#define PIN_LED		IO_PIN_13  //LED ... primeiro pino da esquerda


#define PIN_CHECK_IF_IRQ_POSSIBLE_2
#define PIN_CHECK_IF_IRQ_POSSIBLE_3

//GPIO_IRQ_LEVEL... , GPIO_IRQ_EDGE_...
#define PIN_CHECK_IF_IRQ_POSSIBLE_(NUM_)			PIN_CHECK_IF_IRQ_POSSIBLE_##NUM_ 
#define PIN_INSTALL_IRQ_ONFALL(NUM_, CALLBACK_)		attachInterrupt(digitalPinToInterrupt(NUM_), CALLBACK_, FALLING);
#define	PIN_UNINSTALL_IRQ_ONFALL(NUM_,CALLBACK_)	detachInterrupt(digitalPinToInterrupt(NUM_));
#define PIN_IRQ_HANDLER(NAME_)						W0 NAME_()

#define PIN_MODE_OUTPUT				OUTPUT
#define  PIN_MODE_INPUT				INPUT
	
#define PIN_MODE(NUM_,VAL_)			pinMode(NUM_,VAL_);
#define PIN_WRITE(NUM_,VAL_)		digitalWrite(NUM_, VAL_)
#define PIN_READ(NUM_)				digitalRead(NUM_)
#define PIN_NUMBER(NUM_)			NUM_
#define PIN_LO_(NUM_)				IO_PIN_INFO_##NUM_ &= ~(1<< IO_PIN_INFO_BIT##NUM_);
#define PIN_LO(NUM_)				PIN_LO_(NUM_)
#define PIN_HI_(NUM_)				IO_PIN_INFO_##NUM_ |= 1<< IO_PIN_INFO_BIT##NUM_;
#define PIN_HI(NUM_)				PIN_HI_(NUM_)
#define PIN_INIT(NUM_)				
	
#define PIN_STR_(NAME_)				"" #NAME_ ""
#define PIN_PORT_NAME_(NUM_)		0 //"" //PIN_STR_(IO_PIN_INFO_##NUM_) PIN_STR_(IO_PIN_INFO_BIT_##NUM_) 	  
#define PIN_TEST_BEGIN_				test_pin( 
#define PIN_TEST_END_(INFO_)		0);
#define PIN_PORT_(NUM_)             PIN_PORT_NAME_(NUM_)
#define PIN_TEST_(NUM_, INFO_)		PIN_TEST_BEGIN_	NUM_, "" #NUM_ "",  PIN_PORT_(NUM_), PIN_TEST_END_(NUM_)
#define TEST_PINS(PINS_DEF_)		while(1){ test_pin_init(); PINS_DEF_(PIN_TEST_) }

W0 mcu_init()
{
	Serial.begin(9600);
	pinMode(13, OUTPUT);
}