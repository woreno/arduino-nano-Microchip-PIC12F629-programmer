
// Formato HEX - 1 registo:
// :           | preambulo
// 2 bytes     | número de bytes (N)  ~                       ~
// 4 bytes     | endereço 			  ~ todos estes valores	  ~
// 2 bytes     | tipo de registo	  ~ sao ascii e escritos  ~
// 2 bytes     | dados				  ~ em hexadecimal		  ~
// 2 bytes     | checksum			  ~ 					  ~

// tipo de registo
enum HexRegType {  RT_DATA=0, RT_EOF, RT_EXTSEGADDR, RT_STSEGADDR, RT_EXLINADDR, RT_STLINADDR };
SW8* HexRegTypeStrs[] = { "data", 
	                      "eof", 
						  "extended segment address"
						  "start segment address", 
						  "extended linear address",
						  "start linear address"};
struct HexReg
{
	W16 dataSize;
	W32 address;
	W8 regType;
	W32 dataOffset;
	W8 checksum;
	W32 nextRecordOffset;
	W0 print()
	{
		printf("\n tipo: 0x%X (%s) tamanho:%u bytes checksum:%X ", 
					regType, HexRegTypeStrs[regType], dataSize, checksum);
		if (regType!=1)
			printf(" endereco:%X ", address);
	}
	W1 read(FILE* f, W32& currAddress_)
	{
		SW8 b[256];
		while(1)
		{
			fread(b,1,1,f);
			if (b[0]==':') break;
			if (feof(f)) return 1;
		}
		if (feof(f)) return 1;

		#define READFIELD(WT, VAR) \
			fread(b,1,sizeof(WT)*2,f); \
			parseHex_<WT>(b, b+(sizeof(WT)*2), ((WT*)&VAR));

		dataSize = 0;
		READFIELD(W8, dataSize)
		address = 0;
		READFIELD(W16, address)
		READFIELD(W8,  regType)

		if(regType>=2)
		{
			switch(regType)
			{
				case RT_EXTSEGADDR: 
				{
					W16 seg;
					READFIELD(W16, seg)
					address = seg<<4;
				}break;
				case RT_STSEGADDR: 
				{
					READFIELD(W32, address)
					W16 seg = address>>16;
					address&=0xFFFF;
					address+=(seg<<4);
				}break;
				case RT_EXLINADDR:
				{
					W16 hi;
					READFIELD(W16, hi)
					address = (W16_GET_FLIPBYTES(hi))<<16; //bigendian
					// parte de cima de 32bits
				}break;
				case 5:
				{
					// 32bits
					READFIELD(W32, address)
					address = W32_GET_FLIPBYTES(address); //bigendian
				}break;
		
			}
		}
		else
		{
			dataOffset = ftell(f);
			fseek(f, dataSize*2, SEEK_CUR);
		}
		
		READFIELD(W8,  checksum)
		nextRecordOffset = ftell(f);

		//print();
		return regType!=RT_EOF;
		#undef READFIELD
	}
	W0 convertToBuffer(FILE* f, SW8* b_)
	{
		for (W32 i=0; i < dataSize; i++)
		{
			SW8 ch[2];
			fread(ch, 1, 2, f);  
			parseHex_<SW8>(ch, ch+2, b_+i);
		} 
	}
};