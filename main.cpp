#include <conio.h>
#include "vl_conf.hpp"
#include "vl_types.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "vl_con.hpp"
#include "vl_util.hpp"
#include "vl_cmdline.hpp"


#include "asm_pic.h"
#include "hexfmt.h"
#include "pic12F6XX.h"
SW8 programTitle[] = "Programador Microchip PIC12F629/75";
SW8 client_str[]   = "pic12f62975-moreno-client";
SW8 server_str[]   = "pic12f62975-moreno-server";
#define LOG printf
#include "arduino.hpp"


W16 *buff16;
SW8  buff[2048], buffIn[2048];
SW8  cstr[1024];
W32  currAddr = 0;
SW8  dumpDataFilename[] = "out-data";
SW8  dumpProgFilename[] = "out-prog";
//
struct Options
{
	W1 printAsm;
	PICMODE mode;
	W32     comPort;
	SW8    *filename;
	W1     verbose;

	Options()
	{
		mode = PICMODE_UNKNOWN;
		filename = 0;
		comPort = 9;
		printAsm = false;
		verbose = false;
	}
}options;
//
W0    pic_help()
{
	printf("\n Esquema de ligacao arduino e o pic:    ");
	printf("\n       __________________ ");
	printf("\n      |        _.-.__     \\ ");
	printf("\n      |     13| |_|  |12  |");  
	printf("\n      |       | ard  |11  |");
	printf("\n      |       | nano |10 pb2");
	printf("\n      |       |      |9  pb1<--CLK---.  ");
	printf("\n      |       |      |8  pb0<--DATA-. | ");
	printf("\n      |  GND->|      |               || ");
	printf("\n      |     X |______|               || ");
	printf("\n      |                              || ");
	printf("\n   [transistor]                      || ");
	printf("\n        |     ______                 || ");
	printf("\n         `->1 |      | 8<--GND       || ");
	printf("\n            2 | PIC  | 7<--DATA------´| ");
	printf("\n            3 |      | 6<--CLK--------´ ");
	printf("\n     +12V-->4 |______| 5                ");
	printf("\n                                        ");
	printf("\n * Ligar 'a alimentacao (o LED grande vai acender)");
	printf("\n * Esperar que o led do arduino fique intermitente");
	printf("\n  ");
	printf("\n executar com um destes argumentos:");
	printf("\n    rdconf                 ...le config");
	printf("\n    wrconf                 ...editar manualmente a config e escrever");
	printf("\n    dumpprog [-f=X]        ...faz dump a memoria de programa (def. para %s)",dumpProgFilename);
	printf("\n    dumpdata [-f=X]        ...faz dump a memoria eeprom (def. para %s)",dumpDataFilename);
	printf("\n    eraseprog              ...apaga memoria de programa");
	printf("\n    erasedata              ...apaga memoria de dados");
	printf("\n    erase                  ...apaga ambas memorias");
	printf("\n    write -f=X [-asm] [-v] ...escreve ficheiro; tem de estar no formato HEX");
	printf("\n opcoes: ");
	printf("\n      -com=X qualquer comando aceita a porta [def. COM%u]", options.comPort);
	printf("\n      -f=X   X e' o ficheiro");
	printf("\n      -asm   imprime a descodificacao das instrucoes");
	printf("\n      -v     verboso");
	printf("\n <!> atencao - valor OSCAL");
	printf("\n    - eraseprog/erase apaga tambem o valor OSCAL! (nao o mantem)");
	printf("\n    - wrconf para escrever o valor OSCAL apaga a memoria de programa antes");
	printf("\n    - write apaga as memorias se o ficheiro indicar escrita em algumas delas");
	printf("\n      se o ficheiro nao indicar um valor OSCAL para escrita,");
	printf("\n      este e' mantido por defeito; se o ficheiro indicar o valor OSCAL,");
	printf("\n      entao e' perguntado se deseja manter, substituir ou digitar um novo");
}
W1    pic_verify(W32 currAddr_, SW8* b0, SW8* b1, W32 sz_)
{
	W1 ok = 0;
	for (W32 i=0; i< sz_; i++)
	{
		if (b0[i]!=b1[i]) { printf("\n 0x%04X 0x%02X=?0x%02X", currAddr_+i, (W8)b0[i], (W8)b1[i]); ok = 1; printf(" <x>"); };
	}
	//printf("\n");
	return ok;
}
W32   pic_getArgs(SW32 nargs, const SW8 **argVec)
{
	nargs--;
	argVec++;

	if(nargs==0){ printf("\n*** faltam argumentos\n"); pic_help();  }
	 
	const SW8 *arg = argVec[0];
	if (!strcmp(arg,"dumpdata"))
	{
		options.mode = PICMODE_DUMPDATA;
		options.filename = dumpDataFilename;
	}
	else if (!strcmp(arg,"dumpprog"))
	{
		options.mode = PICMODE_DUMPPROG;
		options.filename = dumpProgFilename;
	}
	else if (!strcmp(arg,"rdconf"))
	{
		options.mode = PICMODE_RDCONF;
	}
	else if (!strcmp(arg,"wrconf"))
	{
		options.mode = PICMODE_WRCONF;
	}
	else if (!strcmp(arg,"erase"))
	{
		options.mode = PICMODE_ERASE;
	}
	else if (!strcmp(arg,"erasedata"))
	{
		options.mode = PICMODE_ERASEDATA;
	}
	else if (!strcmp(arg,"eraseprog"))
	{
		options.mode = PICMODE_ERASEPROG;
	}
	else if (!strcmp(arg,"write"))
	{
		options.mode = PICMODE_WRITE;
	}
	else 
	{
		FAIL_IF(1,"opcao desconhecida")
	}
	nargs--;
	argVec++;
	
	for (SW32 i=0; i< nargs; i++)
	{
		const SW8* arg = argVec[i];
		FAIL_IF((arg[0]!='-') , "opcao invalida")
		arg++;
			
		if (!strncmp(arg,"f=",2) && (options.mode==PICMODE_DUMPPROG||options.mode==PICMODE_DUMPDATA||options.mode==PICMODE_WRITE))
		{
			options.filename=(SW8*)arg+2;
		}
		else if (!strcmp(arg,"v") && (options.mode==PICMODE_WRITE))
		{
			options.verbose = true;
		}
		else if (!strcmp(arg,"asm") && (options.mode==PICMODE_WRITE))
		{
			options.printAsm = true;
		}
		else if (!strncmp(arg,"com=",4))
		{
			arg+=5;
			parseUnsigned<W32>((SW8*)arg,&options.comPort);
		}
		else 
		{
			FAIL_IF(1,"opcao desconhecida")
		}
	}

	FAIL_IF((!options.filename && options.mode==PICMODE_WRITE),"nao especificado nome do ficheiro entrada")
	return 0;
}

//
W32   pic_erase(PICMODE mode_)
{
	return 0;
}
W32   pic_prn_conf(W16 oscal, W16 v2006, W16 v2007, 
					W1 showDevId=true, W1 showOscal=true, W1 showConf=true)
{
	if (showOscal)
		printf("\n  0x%04X: 0x%04X (valor OSCAL)",0x3FF, oscal);
	if (showDevId)
	{
		W8 devid = v2006>>5;
		printf("\n  0x2006 - produto:");
		printf("\n    revisao: 0x%02X", v2006&0x1F);
		printf("\n        dev: 0x%02X (PIC %s)", devid, (devid==0x7A)?"12F629":(devid==0x7E)?"12F675":"desconhecido");
	}
	if (showConf)
	{
		printf("\n  0x2007 - config: 0x%04X", v2007);
		W16 cb        = v2007;
		W16 fosc      = cb&7;
		W16 wdte      = (cb>>3)&1;
		W16 pwrte_    = (cb>>4)&1;
		W16 mclre     = (cb>>5)&1;
		W16 boden     = (cb>>6)&1;
		W16 cp_       = (cb>>7)&1;
		W16 cpd_      = (cb>>8)&1;
		W16 bg        = (cb>>12)&3;
		printf("\n     %s: %02u  %s", configStrs[0], fosc, configFoscStrs[fosc]);
		printf("\n     %s: %02u  %s", configStrs[1], wdte, wdte?"SIM":"NAO");
		printf("\n     %s: %02u  %s", configStrs[2], pwrte_,pwrte_?"NAO":"SIM");
		printf("\n     %s: %02u  %s", configStrs[3], mclre, mclre?"SIM":"NAO");
		printf("\n     %s: %02u  %s", configStrs[4], boden, boden?"SIM":"NAO");
		printf("\n     %s: %02u  %s", configStrs[5], cp_, cp_?"NAO":"SIM");
		printf("\n     %s: %02u  %s", configStrs[6], cpd_, cpd_?"NAO":"SIM");
		printf("\n     %s: %02u"    , configStrs[7], bg);
	}
	return 0;
}
W32   pic_rd_conf(W16& oscal, W16& devid, W16& conf)
{
	printf("\n*** a consultar memoria de programa...");
	RXN(buff, 1024);
	RXN((buff+1024), 1024);
	oscal = buff16[0x3FF];
	
	printf("\n*** a consultar memoria de configuracao...");
	RXN(buff, 8*2);
	//for (W32 i=0; i< 8; i++)
	//	printf("\n  0x2%03X: 0x%04X", (W16)i, (W16)buff16[i]);

	devid = buff16[6];
	conf  = buff16[7];
	return 0;
}
W32   pic_wr_conf()
{
	W8  haveOscal, haveConf;
	W16 conf, devid, oscal;
	W8  key;
	W8 d[8];

	printf("\n Escrever OSCAL [OUTRA: sim] [ESC: cancelar]?");
	printf("\n <!> vai apagar a memoria existente de programa");
	key = getch();
	haveOscal = key!=27;
	if (haveOscal)
	{
		printf("\n OSCAL?"); scanf("%s", cstr);
		parseUnsigned<W16>(cstr, &oscal);
	}

	printf("\n Escrever Configuracao [OUTRA: sim] [ESC: cancelar]?");
	key = getch();
	haveConf = key!=27;
	if (haveConf)
	{
		printf("\n Configuracao:");
		for (W16 i=0; i< 8; i++)
		{
			printf("\n  %s", configStrs[i]); 
			if (i==0)
			{
				for (W16 j=0; j< 8; j++) printf("\n   %u ..%s ", j,configFoscStrs[j]);
				printf("\n   ..");
			}
			printf("?");
			scanf("%s", cstr);
			parseUnsigned<W8>(cstr, d+i);
		}
		conf = (d[7]<<12)|(d[6]<<8)|(d[5]<<7)|(d[4]<<6)|(d[3]<<5)|(d[2]<<4)|(d[1]<<3)|(d[0]&7);
	}

	printf("\n Configuracao ----------------------------------------");
	pic_prn_conf(oscal, 0, conf, false, haveOscal, haveConf);
	printf("\n -----------------------------------------------------");
	printf("\n Aceitar para escrita [OUTRA: sim] [ESC: cancelar]?");
	key = getch();
	if (key==27) { printf ("\n*** Cancelado "); TX(DEV_CANCELBYTE) return 0; }
	TX8(haveOscal);
	TXN(&oscal, 2);
	TX8(haveConf);
	TXN(&conf, 2);
	pic_rd_conf(oscal,devid,conf);
	pic_prn_conf(oscal,devid,conf);
	return 0;
}
W32   pic_write()
{
	W8 key;
	HexReg reg;
	FILE* f = fopen(options.filename, "rb");
	if (f==0)
	{	
		printf("\n<!> nao consigo abrir ficheiro '%s'", options.filename);
		reg.dataSize = 0;
		TXN(&reg.address, 2);
		TXN(&reg.dataSize, 2);
		return 0;
	}
	
	W1 haveOscal = false; W16 devid=0, prevOscal=0, oscal=0;
	W1 haveConf = false;  W16 prevConf=0, conf=0;

	// ler o valor de oscal e conf.
	pic_rd_conf(prevOscal, devid, prevConf);

	while(reg.read(f, currAddr))
	{
		// se o registo não for de dados,
		// avançar
		if (reg.regType!=RT_DATA)
		{
			currAddr = reg.address;
			if (options.verbose) reg.print();
			continue;
		}
		fseek(f,reg.dataOffset, SEEK_SET);
		
		// converter dados
		reg.convertToBuffer(f, buff); 
		 
		// enviar dados para escrever no PIC
		TXN(&reg.address, 2);
		TXN(&reg.dataSize, 2);

		W8 memChanged = 0;
		RX8(memChanged)

		if (options.verbose)
			reg.print();

		if (memChanged!=0xFF || options.printAsm) 
			printf("\n 0x%04X",reg.address);

		if (memChanged!=0xFF) printf(" - memoria de destino mudada para %s", picMemStrs[memChanged]);
		
		// se endereço baixo, é ASM - imprimir	
		if (options.printAsm)
		{
			if (reg.address<0x7FE)
			{
				for (W32 i=0; i< (W32)(reg.dataSize>>1); i++)
				{
					ASM_pic_decode(buff16[i], cstr);
					printf("\n  0x%04X: ", reg.address+i);
					printf(cstr);
				}
			}
		}
		if ((0x7FE>=reg.address) && (0x7FE<=(reg.address+reg.dataSize)))
		{
			W32 id = (reg.address-0x7FE)>>1;
			haveOscal = true;
			oscal = buff16[id];
			key = 0;
			while(key!=27)
			{
				printf("\n\n  <!> o ficheiro tem OSCAL 0x%04X, e o previo e' 0x%04X", oscal, prevOscal);
				printf("\n    [M manter previo] [A aceitar novo] [E escolher outro]");
				key = getch();
				switch(key)
				{
					case 'A': case 'a':	key=27; break;
					case 'E': case 'e': 
					case 'M': case 'm':	
					{
						if (key=='E'||key=='e')
						{
							printf("\n    OSCAL?"); scanf("%s", cstr);
							parseUnsigned<W16>(cstr, &prevOscal);
							key=27; 
						}
						buff16[id] = prevOscal; oscal = prevOscal; key=27; 
					}
					break;
				}
				printf("\n");
			}
		}
		if ((0x400E>=reg.address) && (0x400E<=(reg.address+reg.dataSize)))
		{
			W32 id = (reg.address-0x400E)>>1;
			haveConf = true;
			buff16[id] = ((buff16[id]&0x0FFF)|(prevConf&0x3000)) & 0x31FF;
			conf = buff16[id];
			// manter bits BG da anterior config.
			// cancelar ultimos 2 bits
		}
		
		TXN(buff, reg.dataSize);

		// receber dados lidos do PIC (temporário)
		W16 rxAddr, rxDataSize; 
		RXN(&rxAddr, 2);
		RXN(&rxDataSize, 2);
		RXN(buffIn, rxDataSize);

		W32 err = 0;
		if (rxAddr!=reg.address) {err|=1; printf("\n<!> endereco nao o mesmo %X!=%X", reg.address, rxAddr);}
		if (rxDataSize!=reg.dataSize) { err|=2; printf("\n<!> tamanho nao o mesmo %X!=%X", reg.dataSize, rxDataSize);}
		if (pic_verify(reg.address, buff, buffIn, reg.dataSize)) {err|=4; printf("\n<!> falhou verificacao de dados");};

		if (err)
		{
			//TX(DEV_CANCELBYTE); return 1;
		}	
		TX(DEV_READYBYTE);
		fseek(f,reg.nextRecordOffset, SEEK_SET);
	}

	// dizer que não há mais dados...
	reg.dataSize = 0;
	TXN(&reg.address, 2);
	TXN(&reg.dataSize, 2);
	fclose(f);
	Sleep(100);
	
	TX8(haveOscal)
	if (!haveOscal)
	{
		printf("\n*** a configuracao nao tem OSCAL, a escrever o previo 0x%04X...", prevOscal);
		TXN(&prevOscal,2)
	}
	if (haveOscal || haveConf)
	{
		printf("\n\n*** configuracao escrita: ");
		pic_prn_conf(oscal, 0, conf, false, haveOscal, haveConf);
	}

	printf("\n*** fim da escrita");
	printf("\n---------------------------------------------");
	// 
	pic_rd_conf(oscal,devid,conf);
	pic_prn_conf(oscal,devid,conf);
	return 0;
}
W32   pic_dump(PICMODE mode_)
{
	FILE* fout = fopen(options.filename,"w");
	if (mode_==PICMODE_DUMPDATA)
	{
			printf("\n*** dumping da memoria de dados...");
			RXN(buff, 128);
			for (W32 z=0; z< 128; z++)
			{
				W8 ch = buff[z]&0xFF;
				fprintf(fout, "\n0x%04X: 0x%02X '%c'", (W16)z, ch, VL_IS_PRINTABLE(ch)?ch:'_');
			}
	}
	else
	{
		printf("\n*** dumping da memoria de programa...");
		for (W32 i=0, j=0; i< 2; i++)
		{
			RXN(buff, 1024);
			for (W32 z=0; z< 512; z++, j++)
			{
				fprintf(fout, "\n0x%04X: 0x%04X", (W16)j, buff16[z]);
			}
			printf(".");
		}
	}
	fclose(fout);
	return 0;
}

SW32 main(SW32 nargs, const SW8 **argVec)
{
	buff16 = (W16*)buff;
	using namespace vl::util;
	W16 res = 1;

	printf("\n%s\n",programTitle);
	if (pic_getArgs(nargs,argVec)) return 1;
	if (open_port()) return 1;  

	TX8(options.mode);
	Sleep(100);
	switch(options.mode)
	{ 
		case PICMODE_ERASE:     res = pic_erase(PICMODE_ERASE); break;
		case PICMODE_ERASEDATA: res = pic_erase(PICMODE_ERASEDATA); break;
		case PICMODE_ERASEPROG: res = pic_erase(PICMODE_ERASEPROG); break;
		case PICMODE_WRITE:     res = pic_write(); break;
		case PICMODE_DUMPDATA:  res = pic_dump(PICMODE_DUMPDATA); break;
		case PICMODE_DUMPPROG:  res = pic_dump(PICMODE_DUMPPROG); break;
		case PICMODE_RDCONF:    
		{
			W16 oscal,devid,conf; 
			res = pic_rd_conf(oscal,devid,conf); 
			pic_prn_conf(oscal,devid,conf);
			break;
		}
		case PICMODE_WRCONF:    res = pic_wr_conf(); break;
	}
	if (res==0) printf("\n*** OK"); else printf("\n*** algo falhou");
	port.close();
}