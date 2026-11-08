// PLACA: Arduino Nano "OLD BOOTLOADER"

#define W1  bool
#define W0  void 
#define W8  unsigned char 
#define SW8 char 
#define W16 unsigned short
#define W32 unsigned int
#define FOREVER while(1){ blink(1,1000); }

                                    // atrasos 
#define ATOMIC_DELAY           __asm__ __volatile__ ("nop");
#define DO_TX_CMD_ST_DELAY   ATOMIC_DELAY //por o pino para escrita
#define DO_RX_CMD_ST_DELAY   ATOMIC_DELAY //por o pino para leitura
#define DO_TX_CMD_DELAY      delayMicroseconds(1);
#define DO_TX_FLIP_DELAY     delayMicroseconds(1);
#define DO_RX_FLIP_DELAY     delayMicroseconds(1); //ok 
#define DO_WRT_DELAY         delay(8); // tprog0/1 2-6ms
#define DO_SETDATA_TX_DELAY  ATOMIC_DELAY
								                      // forward
W0 reset_serial();
W1 handShake();
W1 ack();
W0 blink(W16 times, W16 dly);
W0 setup();

                                     // RX/TX
#define BAUD_RATE   38400
#define rxWait      while (Serial.available() == 0){}
#define rx8(X_)     rxWait X_=Serial.read()
#define rxN(X_,N_)  rxWait Serial.readBytes((W8*)(X_),N_)
#define tx8(X_)     Serial.write(X_)
#define txN(X_,N_)  Serial.write((W8*)(X_),N_)

                                    // protocolo
#define ID_STR_SZ    18
#define DEV_READYBYTE 0xAA
#define DEV_CANCELBYTE 0xBB
SW8 client_str[] = "pic-moreno-client";
SW8 server_str[] = "pic-moreno-server";

                                    // buffers
W8  buff[1024]; W16 zero; W16 *buff16;
W8  cmd, fastMode=0;
W16 data;

                                    // pinos, 
									// arduino nano porta série        no PIC pino 1
                                    // apontar para cima...          apontar para cima.. 
#define PIN_LED           13  //LED ... 1º pino da esquerda
#define PIN_TGATE         10   //PB2 ... ativador da gate do transistor
#define PIN_SCLOCK        9   //PB1 ... 4º pino da direita CLK  --> 6  3º pino direita
#define PIN_SDATA         8   //PB0 ... 5º pino da direita DATA --> 7  2º pino direita
#define SETUP_DATA_FOR_READ  { DDRB&=0xFE; }
#define SETUP_DATA_FOR_WRITE { DDRB|=0xFF; }

									                  // trocar bits dos pinos de CLOCK e DATA
#define DO_GETDATA        (digitalRead(PIN_SDATA)&1)   //PORTB&1..infelizmente nao funciona
#define DO_SETDATA(X_)    PORTB&=0xFE; PORTB|=((X_)&1);
#define DO_CLK_FLIP       PORTB^=2;
#define DO_TGATE_OFF      PORTB&=0xFB;
#define DO_TGATE_ON       PORTB|=0x4;
#define DO_SETBOTH_ZERO   PORTB=0;
									                  // comandos do PIC
#define CMD_WRT_CONF          (0x0) 
#define CMD_WRT_PROGMEM       (0x2) 
#define CMD_WRT_DATAMEM       (0x3) 
#define CMD_RD_PROGMEM        (0x4) 
#define CMD_RD_DATAMEM        (0x5) 
#define CMD_INC_ADDR          (0x6)
#define CMD_WRT_AUTO          (0x8) // 
#define CMD_WRT_BEGIN         (0x18)
#define CMD_WRT_END           (0xA)
#define CMD_ERASE_PROGMEM     (0x9)
#define CMD_ERASE_DATAMEM     (0xB)


								                  	// emitir um bit com um clock
#define TX_DATA_CLK(DATAWORD_,I_) \  
	DO_SETDATA((DATAWORD_)>>I_) \
  DO_SETDATA_TX_DELAY     \
	DO_CLK_FLIP \
	DO_TX_FLIP_DELAY \
	DO_CLK_FLIP \
	DO_TX_FLIP_DELAY    \     
  DO_SETDATA_TX_DELAY     
                                    // receber um bit com um clock
#define RX_DATA_CLK(DATAWORD_,I_) \  
  DO_CLK_FLIP \
  DO_RX_FLIP_DELAY \
  DO_CLK_FLIP \
  DO_RX_FLIP_DELAY \
  DATAWORD_ |= DO_GETDATA << I_; \
  DO_RX_FLIP_DELAY
  
									          // enviar comando ao PIC (6 bits)
#define TX_CMD(NAME_)   \
  DO_TX_CMD_DELAY       \
  SETUP_DATA_FOR_WRITE  \
  DO_TX_CMD_ST_DELAY    \
	TX_DATA_CLK(NAME_,0)  \
	TX_DATA_CLK(NAME_,1)  \
	TX_DATA_CLK(NAME_,2)  \
	TX_DATA_CLK(NAME_,3)  \
	TX_DATA_CLK(NAME_,4)  \
	TX_DATA_CLK(NAME_,5)  \
	DO_TX_CMD_DELAY

							           // enviar 14bits para o PIC
#define TX_DATAWORD(DATAWORD_)  \
	TX_DATA_CLK(0,0)         \
	TX_DATA_CLK(DATAWORD_,0)    \
	TX_DATA_CLK(DATAWORD_,1)    \
	TX_DATA_CLK(DATAWORD_,2)    \
	TX_DATA_CLK(DATAWORD_,3)    \
	TX_DATA_CLK(DATAWORD_,4)    \
	TX_DATA_CLK(DATAWORD_,5)    \
	TX_DATA_CLK(DATAWORD_,6)    \
	TX_DATA_CLK(DATAWORD_,7)    \
	TX_DATA_CLK(DATAWORD_,8)    \
	TX_DATA_CLK(DATAWORD_,9)    \
	TX_DATA_CLK(DATAWORD_,10)   \
	TX_DATA_CLK(DATAWORD_,11)   \
	TX_DATA_CLK(DATAWORD_,12)   \
	TX_DATA_CLK(DATAWORD_,13)   \
	TX_DATA_CLK(0,0)         \
	DO_TX_CMD_DELAY

								                    // receber 14bits do PIC
#define RX_DATAWORD(DATAWORD_)   \
  SETUP_DATA_FOR_READ            \
  DO_RX_CMD_ST_DELAY            \
	DATAWORD_ = 0;               \
	RX_DATA_CLK(zero,0)	         \
	RX_DATA_CLK(DATAWORD_,0)	 \
	RX_DATA_CLK(DATAWORD_,1)	 \
	RX_DATA_CLK(DATAWORD_,2)	 \
	RX_DATA_CLK(DATAWORD_,3)	 \
	RX_DATA_CLK(DATAWORD_,4)	 \
	RX_DATA_CLK(DATAWORD_,5)	 \
	RX_DATA_CLK(DATAWORD_,6)	 \
	RX_DATA_CLK(DATAWORD_,7)	 \
	RX_DATA_CLK(DATAWORD_,8)	 \
	RX_DATA_CLK(DATAWORD_,9)	 \
	RX_DATA_CLK(DATAWORD_,10)    \
	RX_DATA_CLK(DATAWORD_,11)    \
	RX_DATA_CLK(DATAWORD_,12)    \
	RX_DATA_CLK(DATAWORD_,13)    \
	RX_DATA_CLK(zero,0)          \
	DO_TX_CMD_DELAY
 
enum  PICMODE  
  { PICMODE_UNKNOWN = 0,
    PICMODE_RDCONF    ,
  PICMODE_WRCONF    ,
  PICMODE_DUMPPROG  ,
  PICMODE_DUMPDATA  ,
  PICMODE_WRITE     ,
  PICMODE_ERASE     ,
  PICMODE_ERASEPROG ,
  PICMODE_ERASEDATA ,
  PICMODE_END
};  
                             // variaveis de programa
W16 currAddr = -1, dataAddr = 0;
W16 dataSize = 0;
enum ADDR_TYPE { ADDR_UNKNOWN=-1, ADDR_PROGMEM = 0, ADDR_CONF=1, ADDR_EPROMMEM = 2};
ADDR_TYPE addrType = ADDR_UNKNOWN;

W0 pic_reset_programming()
{
  DO_SETBOTH_ZERO
  // a gate é posta a ON, o transistor faz CURTO,
  // coletor fica AO GND, e ao GND fica o pino VDD do PIC
  // só usar depois do LED baixar
  digitalWrite(13,HIGH); 
  DO_TGATE_ON
  delay(200);
  // a gate é posta a OFF, o transistor abre e o PIC
  // recebe a corrente no coletor
  digitalWrite(13,LOW);
  DO_TGATE_OFF
  delay(100);
  digitalWrite(13,HIGH); 
}
W0 pic_writeOscal(W16 oscal_){

    pic_reset_programming();
    TX_CMD(CMD_RD_PROGMEM)  
    TX_DATAWORD(0)
    // iterar
    for (W16 i=0; i< 0x3FF; i++)
    {
      TX_CMD(CMD_INC_ADDR)
    }
    // escrever OSCAL
    TX_CMD(CMD_WRT_PROGMEM)  
    TX_DATAWORD(oscal_)
    TX_CMD(CMD_WRT_AUTO)
    DO_WRT_DELAY
}
  
W0 pic_dumpprog(){
  for (W16 j=0; j< 2; j++)
  {
    for (W16 i=0; i< 512; i++)
    {
      TX_CMD(CMD_RD_PROGMEM)  
      RX_DATAWORD(data)   
      buff16[i]=data;  
      TX_CMD(CMD_INC_ADDR)
    }
    txN(buff,1024);
  }
}

W0 pic_dumpdata() {
  for (W16 j=0; j< 128; j++)
  {
   TX_CMD(CMD_RD_DATAMEM)
   RX_DATAWORD(data)
   buff[j] = data;
   TX_CMD(CMD_INC_ADDR)
  }
  txN(buff, 128);
}

W0 pic_erase(PICMODE mode_){
  
    switch(mode_)
    {
      case PICMODE_ERASE:
      {
        TX_CMD(CMD_ERASE_PROGMEM)
        delay(300);
        TX_CMD(CMD_ERASE_DATAMEM)
        delay(300);
      }
      break;     
      case PICMODE_ERASEPROG:
      {
        TX_CMD(CMD_ERASE_PROGMEM)
        delay(300);
      }
      case PICMODE_ERASEDATA:
      {
        TX_CMD(CMD_ERASE_DATAMEM)
        delay(300);
      }
      break;     
    }
}

W0 pic_rd_conf(){ 

  // transferir memoria programa
  pic_dumpprog();
  
  // situar-se na configuracao
  TX_CMD(CMD_WRT_CONF)
  TX_DATAWORD(data)

  // transferir IDs e config
  for (W16 i=0; i< 8; i++)
  {
     TX_CMD(CMD_RD_PROGMEM)
     RX_DATAWORD(data)
     buff16[i] = data;
     TX_CMD(CMD_INC_ADDR)
  }
  txN(buff, 8*2);
}

W0 pic_wr_conf(){
    W8 haveOscal, haveConf;
    W16 oscal, v2007;
    rx8(haveOscal);
    if (haveOscal==DEV_CANCELBYTE) return;
    rxN(&oscal, 2);
    rx8(haveConf);
    rxN(&v2007, 2);

    if (haveOscal)
    {
      pic_erase(PICMODE_ERASEPROG);
      pic_writeOscal(oscal);
    }
    
    pic_reset_programming();
    
    if (haveConf)
    {
      pic_erase(PICMODE_ERASEDATA);
      // ir para o espaco de configuracao
      // iterar até 2004
      TX_CMD(CMD_WRT_CONF)
      TX_DATAWORD(0)
      for (W16 i=0; i< 4; i++)
      {
        TX_CMD(CMD_WRT_PROGMEM)
        TX_DATAWORD(i)
        //TX_CMD(CMD_WRT_AUTO)
        TX_CMD(CMD_WRT_BEGIN)
        DO_WRT_DELAY
        DO_WRT_DELAY
        TX_CMD(CMD_WRT_END)
        TX_CMD(CMD_INC_ADDR)
      } //2004
      TX_CMD(CMD_INC_ADDR) //2005
      TX_CMD(CMD_INC_ADDR) //2006
      TX_CMD(CMD_INC_ADDR) //2007
      // escrever configuracao
      TX_CMD(CMD_WRT_PROGMEM)
      TX_DATAWORD(v2007)
      TX_CMD(CMD_WRT_BEGIN)
      // TX_CMD(CMD_WRT_AUTO)
      DO_WRT_DELAY
      DO_WRT_DELAY
      //TX_CMD(CMD_WRT_END)
      DO_WRT_DELAY
    }  
    pic_reset_programming();
    // ler configuracao de novo
    pic_rd_conf();
}

W0 pic_write(){

    // enviar a configuracao
    pic_rd_conf();
    
    pic_reset_programming();
    
       #define DO_WRITE(CMD_NAME_)  \
          data = buff16[i];    \
          TX_CMD(CMD_NAME_)    \
          TX_DATAWORD(data)    \
          TX_CMD(CMD_WRT_AUTO) \
          DO_WRT_DELAY
          
       #define DO_READ(CMD_NAME_)  \
          W16 data2=0xAAEE;        \
          TX_CMD(CMD_NAME_)        \
          RX_DATAWORD(data2)       \
          buff16[i]=data2;     \
          TX_CMD(CMD_INC_ADDR)
          
   while(1)
   {
    // receber endereco, tamanho e dados
    // o endereço está ao byte
    rxN(&dataAddr,2);
    rxN(&dataSize,2);
    if (dataSize==0) break;

    // determinar a que zona pertence esse endereco
    ADDR_TYPE newAddrType;
    if (dataAddr < 0x4000) 
      newAddrType = ADDR_PROGMEM; 
    else if (dataAddr >= 0x4000 && dataAddr < 0x4200)
      newAddrType = ADDR_CONF;
    else
      newAddrType = ADDR_EPROMMEM;

    // se a zona for diferente do atual, situar-se 
    // no inicio com uma leitura dummy
    if (addrType!=newAddrType)
    { 
      tx8(newAddrType);
      blink(6,100);
      addrType=newAddrType;
      switch(addrType)
      {
        case ADDR_PROGMEM:  
          pic_erase(PICMODE_ERASEPROG);
          currAddr = 0;
          TX_CMD(CMD_RD_PROGMEM)
          RX_DATAWORD(data)
        break;
        case ADDR_CONF:
          currAddr = 0x4000;
          TX_CMD(CMD_WRT_CONF)
          TX_DATAWORD(data)
        break;
        case ADDR_EPROMMEM:
          currAddr = 0x4200;
          pic_erase(PICMODE_ERASEDATA);
          TX_CMD(CMD_RD_DATAMEM)
          RX_DATAWORD(data)
        break;
      } 
    }
    else { tx8(ADDR_UNKNOWN);}
    // iterar o endereço que está ao byte 
    // como está ao dobro, iterar +=2;
    for (W16 i=currAddr; i< dataAddr; i+=2)
    {  TX_CMD(CMD_INC_ADDR) }
    currAddr = dataAddr;

    // receber dados
    rxN(buff,dataSize);
    // escrever e verificar escrita no PIC; os dados enviados pelo PC
    // para escrita no PIC "em buff" vao ser sobrescritos pela leitura
    // se separam os 3 casos para nao usar variaveis e assim poupar
    // as operacoes bit-a-bit que sao feitas, feitas em tempo de compilacao
    // a excepcao para os dados que se vao enviar/receber
    const W16 dataSize16 = dataSize>>1;
    switch(addrType)
    {
      case ADDR_PROGMEM:
      case ADDR_CONF:
        for (W16 i=0; i< dataSize16; i++)
        {  
          DO_WRITE(CMD_WRT_PROGMEM)
          DO_READ(CMD_RD_PROGMEM)
        }
      break;
      case ADDR_EPROMMEM:
        for (W16 i=0; i< dataSize16; i++)
        {
          DO_WRITE(CMD_WRT_DATAMEM)
          DO_READ(CMD_RD_DATAMEM)
        }
      break;
    }
    // enviar para o PC os dados lidos para validacao (temporario)
    // a fazer diretamente aqui a validacao
    txN(&currAddr,2);
    txN(&dataSize,2);
    txN(buff,dataSize);
    if (!ack()) return;
    // iterar o endereco currente
    currAddr+=dataSize;
   }
   pic_reset_programming();
   // mandar configuração

   W8 haveOscal; W16 prevOscal;
   rx8(haveOscal);
   if (!haveOscal)
   {
      rxN(&prevOscal, 2); 
      pic_writeOscal(prevOscal);
   }
   pic_reset_programming();
   
   pic_rd_conf();
}

  
W0 loop() {

  reset_serial();
  while (Serial.available() == 0)
  {
    blink(1,100);  
  }
  
  // esperar pela string conhecida
  if (handShake()) goto fail;
  
  // receber ordem
  rx8(cmd);

  switch(cmd)
  {
    case PICMODE_ERASE:    pic_erase(cmd); break;
    case PICMODE_WRITE:    pic_write(); break;
    case PICMODE_DUMPDATA: pic_dumpdata(); break;
    case PICMODE_DUMPPROG: pic_dumpprog(); break;
    case PICMODE_RDCONF:   pic_rd_conf(); break; 
    case PICMODE_WRCONF:   pic_wr_conf(); break; 
  }
  fail:
    Serial.end();
}

////////////////////////////////////////////////////////////////

W0 reset_serial(){
  
  // buffer entupido?
  for (W16 i=0; i< 10; i++)
  while(Serial.available()) {  rx8(buff[0]); delay(1); }

  // reiniciar
  Serial.end();
  Serial.begin(BAUD_RATE, SERIAL_8N1);
  Serial.flush();
  Serial.setTimeout(4000);
   
}

W1 ack(){
  
  buff[0] = 0;
  rx8(buff[0]); 
  return (buff[0]==DEV_READYBYTE);
}

W1 handShake(){   
  
    rxN(buff,ID_STR_SZ);
    if (strncmp((const SW8*)buff,client_str,ID_STR_SZ))
    {
      return 1;
    }
    // enviar id;
    txN(server_str,ID_STR_SZ);
    rxN(buff,1);
    if (buff[0] == DEV_READYBYTE)
    {
      return 0;
    }
    return 1;
}

W0 blink(W16 times, W16 dly){
  
    for (W16 i=0; i< times; i++)
    {
      digitalWrite(13,HIGH);
      delay(dly);
      digitalWrite(13,LOW);
      delay(dly);
    }
}

W0 setup() { 
  zero = 0;
  buff16 = (W16*)buff;
  pinMode (PIN_LED,     OUTPUT);
  pinMode (PIN_SDATA,   OUTPUT);
  pinMode (PIN_SCLOCK,  OUTPUT);  
  pinMode (PIN_TGATE, OUTPUT);
  pic_reset_programming();
}


  /* verifica no osciloscopio que ambos pinos sobem/descem
   * já aconteceu o arduino estava mal posto...
  while(1)
  {
     TX_CMD(CMD_RD_PROGMEM)
      delay(1);
  }*/
  
