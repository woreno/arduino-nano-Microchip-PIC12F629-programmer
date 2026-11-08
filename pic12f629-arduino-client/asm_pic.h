
SW8* ASM_pic_case0_strs[]= {
	"MOV",			//0000
	"CLEAR",		//0001
	"SUB",			//0010
	"DEC",			//0011
	"OR "			//0100
	"AND",			//0101
	"XOR",			//0110
	"ADD",			//1000
	"MOVE",			//1000
	"COMF",			//1001
	"INCF",			//1010
	"DECFSZ",		//1011
	"RRF",			//1100
	"RLF",			//1101
	"SWAPF",		//1110
	"INCFSZ"		//1111
};
SW8* ASM_pic_case1_strs[]=
{
	"BCF", 
	"BSF", 
	"BTFSC", 
	"BTFSS" 
};
SW8* ASM_pic_case2_strs[]=
{
	 "CALL", "GOTO"
};

W0 ASM_pic_prn0(SW8* cstr_, W16 opcode_)
{
	switch(opcode_)
	{
		case 0x8:  sprintf(cstr_, "RET"); return;
		case 0x64: sprintf(cstr_, "CLRWDT"); return;
		case 0x63: sprintf(cstr_, "SLEEP"); return;
	}
	if ((opcode_&6)==0) { sprintf(cstr_,"NOP"); return; }

	W16 id = (opcode_>>8)&0xF;
	SW8 *str = ASM_pic_case0_strs[id];
	W8 dest = (opcode_>>7) & 1;
	W8 reg = opcode_ & 0x7F;

	if (id==1)
	{
		if (dest)
			sprintf(cstr_,"%s (f%u)", str, reg);
		else
			sprintf(cstr_,"%s W", str);
	}
	else
	{ 
		if (dest)
			sprintf(cstr_,"f%u = %s (f%u)", reg, str, reg);
		else
			sprintf(cstr_,"W = %s (f%u)", str, reg);
	}
}
W0 ASM_pic_prn1(SW8* cstr_, W16 opcode_)
{
	W16 id = (opcode_>>10)&3;
	SW8 *str = ASM_pic_case1_strs[id];
	W8 bits = (opcode_>>7) & 3;
	W8 reg = opcode_ & 0x7F;

	sprintf(cstr_,"%s (f%u,%u)", str, reg, bits);
}
W0 ASM_pic_prn2(SW8* cstr_, W16 opcode_)
{
	W16 lit = opcode_&0x7FF;
	W16 id  = (opcode_>>11)&1;
	SW8 *str = ASM_pic_case2_strs[id];
	sprintf(cstr_,"%s 0x%X",  str, lit);
}
W0 ASM_pic_prn3(SW8* cstr_, W16 opcode_)
{
	W16 lit = opcode_&0xFF;
	opcode_ = (opcode_>>8)&0xF;
	if (opcode_<=3)
		sprintf(cstr_,"MOVLW 0x%X", lit);
	else if (opcode_<=7)
		sprintf(cstr_,"RETLW 0x%X", lit);
	else if (opcode_==8)
		sprintf(cstr_,"IORLW 0x%X", lit);
	else if (opcode_==9)
		sprintf(cstr_,"ANDLW 0x%X", lit);
	else if (opcode_==10)
		sprintf(cstr_,"XORLW 0x%X", lit);
	else if (opcode_>=12&&opcode_<=13)
		sprintf(cstr_,"SUBLW 0x%X", lit);
	else if (opcode_>=14&&opcode_<=15)
		sprintf(cstr_,"ADDLW 0x%X", lit);
	else
		sprintf(cstr_,"?");
}
W0 ASM_pic_decode(W16 opcode, SW8* cstr_)
{
	cstr_[0]=0;
	if (opcode&0xC000)
	{
		sprintf(cstr_,"<!> o opcode %04X nao tem 14 bits!",opcode);
		return;
	}
	W8  opcodeKey = opcode>>12;
	switch(opcodeKey)
	{
		case 0: //file reg ops
			ASM_pic_prn0(cstr_,opcode);
		break;
		case 1: //bit oriented ops
			ASM_pic_prn1(cstr_,opcode);
		break;
		case 2:  // literal and control ops
			ASM_pic_prn2(cstr_,opcode);
		break;
		case 3:  // literal and control ops
			ASM_pic_prn3(cstr_,opcode);
		break;
	}
}