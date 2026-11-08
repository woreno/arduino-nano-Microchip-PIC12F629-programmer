enum ADDR_TYPE { ADDR_UNKNOWN=-1, ADDR_PROGMEM = 0, ADDR_CONF=1, ADDR_EPROMMEM = 2};

const SW8* picMemStrs[] = { "programa", "config", "eeprom"};

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
const SW8* picmodeStrs[] = 
	{ "-", 
	  "consultar info", 
	  "dump de programa", 
	  "dump de dados", 
	  "escrever ficheiro hex", 
	  "escrever config manualmente",
      "apagar memorias de programa e dados",
	  "apagar memoria de dados",
      "apagar memoria de programa"};

SW8* configStrs[] = 
{
	"  fosc.......funcao de pinos 2 e 3",
	"  wdte..temp. monitorizacao ligado",
	"~pwrte.......temp. arranque ligado",
	" mclre...pin 4 tem funcao de ~mclr",
	" boden...........detecao de apagao",
	"   ~cp....memoria codigo protegida",
	"  ~cpd.....memoria dados protegida",
	"    bg.....bandgap bits calibracao",
	0
};

SW8* configFoscStrs[] = 
{"[2,3: Oscilador low-power]",
 "[2,3: Cristal]",
 "[2,3: Hi-speed Cristal]",
 "[2: I/O] [3: CLK IN]",
 "[2,3: I/O]",
 "[2: Clock Out] [3: I/O]",
 "[2: I/O] [3: RC]", 
 "[2: Clock Out] [3: Clock In]"};
