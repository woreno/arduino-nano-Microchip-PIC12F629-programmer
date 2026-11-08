#ifndef VL_UTILS_
#define VL_UTILS_
#include "vl_system.hpp"
#include "vl_array.hpp"
#include "vl_types.hpp"
#if (VL_SYSTEM_SYS == VL_SYSTEM_SYS_UNIX)
	#include <dirent.h>
#endif
#if (VL_SYSTEM_SYS == VL_SYSTEM_SYS_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
	#include <TlHelp32.h>
#endif 
//STD 
#include <cctype>	
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <istream>
#include <string>  

#ifndef MIN
	#define MIN(a,b)	((a)<(b)?(a):(b))
#endif
#ifndef MAX
	#define MAX(a,b)	((a)>(b)?(a):(b))
#endif

#include "vl_structs.hpp"

namespace vl { namespace util { 

	
template <typename T>
W0 getBitsInit(T* ptr, T& stream0, T& streamPos)
{
	// inicializar
	stream0 = *ptr;
	streamPos = 0;
}
template <typename T>
T getBits(W8 BITS_, T** ptr, T& stream0, T& streamPos)							
{
	/*
  
  [54321098'76543210] [54321098'76543210] [54321098'76543210]
								                      streamPos
								BITS_-N0
								  |-----|--N0---|
  [54321098'76543210] [54321098'76543210] [54321098'76543210]
								  ^             ^
								  |             |
							streamPos+BITS_	 streamPos
	
	*/
	// se os bits pedidos superarem a posição do 1º elemento
	// em ptr
	if ( (streamPos + BITS_) < ((sizeof(T)*8) )  ) 							  
	{		
		// mascarar o que está em stream
		T data = (stream0) &  ((1<<BITS_)-1);				
		stream0>>=BITS_;								
		streamPos+=BITS_;		
		return data;
	}													
	const W8 N0 = (sizeof(T)*8) - streamPos;						
	streamPos = BITS_-N0;							
	++(*ptr);											
	const T stream1 = **ptr;	
	T data = ((stream1<<N0)|stream0) & ((1<<BITS_)-1);
	stream0 = stream1 >>streamPos;					
	return data;
}

			
	static const W8 base64Lut[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 64, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0};
	
	SW32 base64decode(std::string in_, std::string& out_)
	{
		SW32 N = in_.length();
		out_.resize(N);
		W8* str_ = (W8*)in_.c_str();
		for (SW32 i=0; i< N; i++, str_+=4, out_+=3)
		{
			const W8 sxt1 = base64Lut[str_[0]];
			const W8 sxt2 = base64Lut[str_[1]];
			const W8 sxt3 = base64Lut[str_[2]];
			const W8 sxt4 = base64Lut[str_[3]];
			out_[0] = (sxt2<<6) |  sxt1 ;		// 2bit+6bit=8
			out_[1] = (sxt3<<4) | (sxt2>>2);	// 4bit+4bit=8
			out_[2] = (sxt4<<2) | (sxt3>>4);	// 6bit+2bit=8 
		}
		return 0;
	}
	

	template<typename T>
	W32 base2BitSize(T bs)
	{
		W32 lastBit = 0;
		for (W32 i=0; i< sizeof(T)*8; i++) 
		{ if (bs&1) lastBit=i; bs>>=1; }
		lastBit++;
		return lastBit;
	}
	void splitAtDelims(std::vector<std::string>& strs, std::string& str, char* delims, SW32 delimsN);
 
	/*
	//PNG
	#include <png.h>
	// load extra
		// le tambem imagens png em ushort atraves de libpng
		// e muda o endian do ushort
		// (opencv abria estas imagens como 3 canais com depth 8bits)
		cv::Mat imreadPNG(std::string filename, bool noAlpha=1);
		*/
	  
// coisas parvas com strings  I
	// a <-> A
	inline std::string toupper(std::string& str)
	{
		std::string str2(str.length(),' ');
		std::transform(str.begin(), str.end(), 
						str2.begin(), std::toupper);
		return str2;
	}

	inline std::string tolower(std::string& str)
	{
		std::string str2(str.length(),' ');
		std::transform(str.begin(), str.end(), 
						str2.begin(), std::tolower);
		return str2;
	}
	// is a...
	inline bool isUIntegerNumber(std::string str)
	{
		if (str.length()==0)
			return false;
		for (int i=0; i< str.size(); i++)
			if ( !( str[i]>='0' && str[i]<='9') )
				return false;
		return true;
	}
	inline bool isIntegerNumber(std::string str)
	{
		if (str.length()==0)
			return false;

		if ((str[0]=='-') && str.length()>1)
		{
			return isUIntegerNumber(str.substr(1, str.length()));
		}
		else
			return isUIntegerNumber(str);

		return true;
	}
	inline bool isFloatNumber(std::string str)
	{
		if (str.length()==0)
			return false;
		
		int st ;
		st = ( ( !( str[0]>='0' && str[0]<='9') ) && str[0]=='-')? 1: 0;
		 
		if (st == str.length())
			return 0;

		bool fpOcorrence = false;
		for (int i=st; i< str.size(); i++)
			if (  (!( str[i]>='0' && str[i]<='9')) )
			{
				if (str[i]=='.') 
				{
					if (fpOcorrence)
						return false;
					else
						fpOcorrence = true;
				}
				else if (str[i]=='e' || str[i]=='E')
				{
					size_t rem = str.size()-1;
					// seguir expoente
					if (i == (str.size()-1) )
						return false;
						
					int stE = i+1 ;
					if  ( (!( str[stE]>='0' && str[stE]<='9')) && str[stE]=='-') 
						stE++;

					//rem = str.size()-1;

					for (int j=stE; j< str.size(); j++)
					{
						if ((!( str[j]>='0' && str[j]<='9')) )
						{
							return false;
						}
					}
					return true;
				}else
				{
					return false;
				}
			} 
		return true;
	}
	// transforma o wildcard basico para regra regex
	inline std::string  doswildcard2regex(std::string instr__)
	{
		char delims[]={';', ','};
		std::vector<std::string> strs; 
		splitAtDelims(strs, instr__, delims, 2);

		std::string finalStr; 
		for (SW32 j=0; j< strs.size(); j++)
		{
			std::string instr_ = strs[j];
			std::string instr =""; //instr_;
			//std::string  nstr="", instr = "a?b*"; 
			std::size_t found=0;
			std::string wilds[3] = {"?","*","@"}, wildsR[3] = {".","(.+)?","(?i)"};

	 
			for (SW32 i=0; i< instr_.length(); i++)
			{
				if (instr_[i]=='*' || instr_[i]=='?')
				{
					std::string substr = instr_.substr(found,i-found);
					if (substr.length())
						instr = instr + "@("+substr+")";
					instr += instr_[i];
					found = i+1;
				}
			}
			if (found!=instr_.length())
			{
				std::string substr = instr_.substr(found,instr_.length()-found);
				instr = instr + "@("+substr+")";
			}

			for (SW32 i=0; i<3;i++)
				while (1)
				{
					found = instr.find(wilds[i]);
					if (found!=std::string::npos)
						instr.replace(found, 1, wildsR[i]); 
					else
						break;
				}

			if (strs.size()==1)
				finalStr = instr;
			else if (j < (strs.size()-1))
				finalStr+= "("+ instr + ")|";
			else
				finalStr+= "("+ instr + ")";
		}
		return finalStr;
	}		 
	// string <-> num
	template <typename T>
	inline T str2t ( const std::string &t )
	{
		std::istringstream  ss(t);
		T result;
		return  ss >> result ? result : 0;
	}
	template <class T>
	inline std::string  t2str(T  n)
	{
		std::ostringstream convert;
		convert << n;
		return convert.str();
	}

	// \ -> /
	std::string toLeftSlashes( std::string str_ );
	std::string toRightSlashes( std::string str_ );
	
	inline void splitAtDelims(std::vector<std::string>& strs, 
						std::string& str, char* delims, SW32 delimsN=1)
	{
		strs.clear();
		SW32 tokenN = 0;
		SW32 open = -1;
		for (SW32 i=0; i < str.length(); i++)
		{	
			bool isDelim = false;
			for (SW32 j=0; j< delimsN; j++)
				if (delims[j]==str[i])
				{	isDelim = true; break; }

			if (isDelim)
			{
				if (open!=-1)
				{
					strs.push_back(str.substr(open,i-open));
					open = -1;
				}
			}
			else
			{
				if (open==-1)
					open = i;
			}
		}
		if (open!=-1)
			strs.push_back(str.substr(open));
	}

	// 
	inline std::string getSmallString(const char* str_, ...)
	{ 
  	   char buff[2048];
	   va_list arglist;
	   va_start( arglist, str_ );
	   vsnprintf_s(buff, 2048, str_, arglist );
	   va_end( arglist );
	   return std::string(buff);
	}
	// proxima string 
	void nextStr(std::string path_, 
						std::string& curr, 
						std::string& next,
						char delimitador);

	// inserir caracteres de n em n
	inline std::string insertCharsRL(std::string str, int n, char c, bool insertFirst=true)
	{
		std::string dst( str.length() + (SW32)(floor((VL_FP)str.length()/n)), ' ');
		for (int i=0, j=0; j < dst.length(); j++)
		{
			if (j%(n+1)==0)
			{
				dst[j] = c;
				continue;
			}
			dst[j] = str[i];
			i++;
		}
		if (insertFirst)
			dst = dst + c;
		return dst;
	}

	// apagar e partir em white spaces
	//espaços exteriores 
	void stripWS(std::string& str_, int *nLeft=0, int *nRight=0);

	//espaços interiores e exteriores
	inline void stripAllWS(std::string& str_)
	{
		std::string str = str_;
		int st = 0;

		for (VL_SZ i=0; i< str_.length(); i++)
		{
			if ( str_[i]!=32 && str_[i]!=10 && str_[i]!=13 && str_[i]!=9 )
			{
				str[st]=str_[i];
				st++;
			}
		}
		str_ = str.substr(0,st);
	}

	void splitSpcTab(std::vector<std::string>& strs, std::string& str); 
	// partir pelo delimitador 
	std::vector<std::string> &split (const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);

// coisas parvas com strings  II
	// string hex -> char, short
	template <typename T>
	inline T  fromHexStr(std::string s)
	{
		unsigned long long sum = 0;
		for (SW32 j=0, i=(SW32)s.length()-1; i>-1; i--,j+=4)
		{
			int n = 0;
			unsigned char ch = s[i];
			
			if (ch>=48 && ch<=59) // 0..9
				n = ch-48;
			else
			{
				ch = ::toupper(ch); 
				if (ch>=65 && ch<=70)
					n = 10+ch-65; 
			}
			if (n)
				sum+=(1ULL<<j)*n;
				//sum+= ( pow(16.0f,j) )*n; 
		}
		return ((T)sum);
	} 

	// valor para string binária	15 -> "00001111"
	template<typename T>
	inline std::string toBitStr(T b_, bool rightToLeft_=true, int n=sizeof(T)*8)
	{
		std::string s = std::string(n,'0');
		T b = b_;
		if (rightToLeft_)
		{
			for (int pos=n-1; pos>-1; pos--)
			{
				if ((b & 0x1)==1)
					s[pos] = '1';
				b = b >> 1;	
			}
		}else
		{
			for (int pos=0; pos<n-1; pos++)
			{
				if ((b & 0x1)==1)
					s[pos] = '1';
				b = b >> 1;	
			}
		}
		return s;
	}
	
	template<typename T>
	T mapBitsWithDstMap(W32* map_ , T value_, W32 bits_=sizeof(T))
	{
		T word = 0;
		for (W32 i=0; i<bits_ ; i++)
			word |= ((value_>>i)&1)<<map_[i];
		return word;
	}
	template<typename T>
	T mapBitsWithSrcMap(W32* map_ , T value_, W32 bits_=sizeof(T))
	{
		T word = 0;
		for (W32 i=0; i<bits_ ; i++)
			word |= ((value_>>map_[i])&1)<<i;
		return word;
	}



	inline std::string arrToBitStr(char* b_, int strn, bool rightToLeft_=true)
	{
		const int n = 8;
		std::string s = "";
		for (int i=0; i< strn; i++)
		{
			char b = b_[i];
			s = toBitStr(b, rightToLeft_) + s;
		}
		return s;
	}

	// valor para string binária, mas:
	// na 1ª linha insere uma barra a cada nibble com o indice
	// na 2ª linha o valor binario com as barras
	// se nArgs!=0, os valores a seguir sao indices que...
	// na 3ª linha aparecem como indicadores "^" nos respetivos indices
	template<typename DT>
	inline std::string toBitStringWithNibbleDiv(DT var, W8 nArgs = 0, ...)
	{
		std::string strN = "";
		W8 nl;
		for (W8 i=0; i< (sizeof(DT)*8)+4; i+=4)
		{
			std::string numStr = vl::util::t2str(i)+"|";
			nl = 5 - numStr.length();
			strN = std::string(nl,' ') + numStr + strN;
		}
		std::string str = vl::util::toBitStr<DT>(var);
		str =  strN + "\n" + std::string(4,' ') +  
			   vl::util::insertCharsRL(str,4,'|');
		if (nArgs)
		{
			std::string strK = std::string(sizeof(DT)*8, ' ');
			va_list vl; 
			va_start(vl,nArgs); 
			for (W8 i=0; i< nArgs; i++) 
				strK[strK.size()-va_arg(vl, W8)-1] = '^';
			va_end(vl);
			strK = std::string(4,' ') + vl::util::insertCharsRL(strK,4,' ');
			str +=  "\n" + strK;
		}
		return str;
	}
	
	inline std::string arrToBitStringWithNibbleDiv(char* var, W8 varL, W8 nArgs=0, ...)
	{
		std::string strN = "";
		W8 nl;
		for (W8 i=0; i< (varL*8)+4; i+=4)
		{
			std::string numStr = vl::util::t2str(i)+"|";
			nl = 5 - (W8)numStr.length();
			strN = std::string(nl,' ') + numStr + strN;
		}

		std::string str = vl::util::arrToBitStr(var, varL);
		str =  strN + "\n" + std::string(4,' ') + vl::util::insertCharsRL(str,4,'|');

		if (nArgs)
		{
			std::string strK = std::string((varL*8), ' ');
			va_list vl; 
			va_start(vl,nArgs); 
			for (W8 i=0; i< nArgs; i++) 
				strK[strK.size()-va_arg(vl, W8)-1] = '^';
			va_end(vl);
			strK = std::string(4,' ') + vl::util::insertCharsRL(strK,4,' ');
			str +=  "\n" + strK;
		}
		return str;
	}
	
	
	// sem complemento 2, mesmo typename signed!
	// de string binária para valor	"00000111" -> 15 
	template<typename T>
	inline T fromBitStr(std::string str_)
	{
		T num = 0; 
		for (int pos=str_.size()-1, idx=0; pos>-1; pos--, idx++)
		{
			if ( str_[pos]=='1' ) 
				num+=(1 << idx);
		}
		return num;
	}

	// valor para hexadecimal		15 -> "0x0F"
	template<typename T>
	inline std::string toHexStr(T b_, bool padWithZeros=true, W8 addPrefix=1)
	{
		const std::string prefixes[] = {"","0x","$"};
		std::ostringstream convert;
		convert << std::hex << (unsigned long long)b_;
		std::string str = convert.str();
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		if (padWithZeros==true)
		{
			std::string remain(((sizeof(T)<<1) -str.length()),'0');
			return prefixes[addPrefix]+remain+str;
		}
		else
		{
			return prefixes[addPrefix]+str;
		}
	}
	// valor para hexadecimal		15 -> "0x0F"
	template<typename T>
	inline std::string toHexStr(T b_, int padWithZeros, W8 addPrefix=1)
	{
		const std::string prefixes[] = {"","0x","$"};
		std::ostringstream convert;
		convert << std::hex << (unsigned long long)b_;
		std::string str = convert.str();
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);

		if (padWithZeros==-1)
		{
			std::string remain(((sizeof(T)<<1) -str.length()),'0');
			prefixes[addPrefix]+remain+str;
		}
		else if ( (padWithZeros>0) && ( padWithZeros>str.length() ) )
		{
			std::string remain( padWithZeros -str.length(),'0');
			return prefixes[addPrefix]+remain+str;
		}
		return prefixes[addPrefix]+str;
	}
	// confirma hex e retira os prefixos/sufixos de hex
	// intervalo na string onde se encontra o valor hex: [stIdx_,eIdx_[
	// 0: no 1:sure 2:maybe(so numero) 
	inline int  isUNumberCodedAsHex(std::string str_, int* stIdx_=0, int* eIdx_=0)
	{
		if (str_.length()==0)
			return 0;
		
		int stIdx, eIdx;
		if (vl::util::isUIntegerNumber(str_))
		{
			stIdx = 0;
			eIdx  = (int)str_.length();
			return 2;
		}

		std::string str = vl::util::toupper(str_);
		std::string::iterator it;
		it = std::find(str.begin(), str.end(), 'X');
		if (it!=str.end()) //x ?
		{
			if ( it-str.begin() > 1 )
				return 0;
			stIdx = (int)(it-str.begin()+1);
			if (stIdx==2)
			{
				if (str[0]!='0')
					return 0;
			}
			eIdx  = (int)(str.end()-str.begin());
		}
		else
		{
			it = std::find(str.begin(), str.end(), 'H');
			if ( it != str.end())
			{
				if ( str.end()-1 != it ) //tem h mas nao no fim
					return 0;
				stIdx = 0;
				eIdx  = (int)str.length()-1;
			}else
			{
				stIdx = 0;
				eIdx  = (int)str.length();
			}
		}

		for (int i=stIdx; i< eIdx; i++)
		{ 
			if (!((str[i]>='A' && str[i]<='F') || 
				  (str[i]>='0' && str[i]<='9')))
				  return 0;
		}
		if (stIdx_) *stIdx_ = stIdx;
		if (eIdx_)  *eIdx_  = eIdx;
		return 1;
	}
	
	// confirma numero ou hex e retira os prefixos/sufixos se hex 
	// igual a isUNumberCodedAsHex [stIdx_,eIdx_[ ;
	// confirma se no intervalo [rangeStart,rangeEnd]
	// 
	// return: 
	//	invalid: -1: fora do intervalo	0: não é numero sem sinal ou hex
	//    valid:  1: numero sem sinal	2: numero em hex
	inline int  isUNumberOrCodedAsHexInRange(std::string argIn, 
											 long long rangeStart, 
											 long long rangeEnd, 
											 int* stIdx_=0, int* eIdx_=0)
	{
		bool isNumber = vl::util::isUIntegerNumber(argIn);
		long long  number;
		int	 type;
		if (isNumber)
		{
			number = vl::util::str2t<long long>(argIn);
			type = 1;
		}
		else
		{ 
			int stIdx,eIdx; //[st,end[
			int hextype  = isUNumberCodedAsHex(argIn, &stIdx, &eIdx);
			if (hextype != 1)
			{
				//sprintf(cstr, "o argumento hexadecimal %i tem de estar compreendido entre 0 e %i", i, MAX_USHORT );
				//cause = std::string(cstr);
				return 0;
			}
			std::string subStr = std::string(argIn.begin()+stIdx, argIn.begin()+eIdx);
			number = vl::util::fromHexStr<long long>(subStr); 
			type = 2;
			if (stIdx_) *stIdx_ = stIdx;
			if (eIdx_)  *eIdx_	= eIdx;
		}
		if (!( number >=rangeStart && number <= rangeEnd ))
		{
			//sprintf(cstr, "o argumento hexadecimal %i tem de estar compreendido entre 0 e %i", i, MAX_USHORT );
			//cause = std::string(cstr);
			return -1;
		}
		return type;
	}
	// faz uma string com o resumo dum valor char/int ou long long
	// nas 4 represent: (H)ex, (U)nsigned, (S)igned, (B)inária 
	template<typename T>
	inline std::string getValueRepresentations(T b)
	{
		std::string str;
		str+="H:"+toHexStr<T>(b)+" ";
		switch (sizeof(T))
		{
			case 8:
				str+= "U:"+t2str<int>((unsigned char)b)+" " +
					  "S:"+t2str<int>((char)b)+" ";
				break;
			case 16:
				str+= "U:"+t2str<int>((unsigned short)b)+" " +
					  "S:"+t2str<int>((short)b)+" ";
				break;
			case 32:
				str+= "U:"+t2str<int>((unsigned int)b)+" " +
					  "S:"+t2str<int>((int)b)+" ";
				break;
			case 64:
				str+= "U:"+t2str<int>((unsigned long long)b)+" " +
					  "S:"+t2str<int>((long long)b)+" ";
				break;
		}
		str+="B:"+toBitStr<T>(b); 
		return str;
	}
	
	// formatar tamanho nas unidades da informação
	const char  byteScalePrefixes[] = {' ','K', 'M', 'G', 'T' };
	void formatBytes(unsigned int  bytes, 
							float&  q, 
							char& scalePrefix)
	{
		W32 b = bytes;
		SW32 i = 0;
		for (; i< 5; i++)
		{
			if ( b<1024 )
				break;
			b = b>>10;
		}
		if (i!=0)
		{
			q = (float)bytes/(1<<(i*10));
			scalePrefix = byteScalePrefixes[i];
		}
		else
		{
			scalePrefix = 0;
			q = bytes;
		}
	}
	std::string formatBytes(unsigned int bytes)
	{
		char prefix = 0; 
		char cstr[150];
		float  q;
		formatBytes(bytes, q, prefix);
		if(q - (SW32)q == 0) 
		{
			if (prefix==0)
				sprintf(cstr,"%0.0fB",q);
			else
				sprintf(cstr,"%0.0f%cB",q,prefix);
		}
		else
		{
			if (prefix==0)
				sprintf(cstr,"%0.3fB",q);
			else
				sprintf(cstr,"%0.3f%cB",q,prefix);
		}
		return( std::string(cstr) );
	}

// coisas parvas com strings  III
	inline std::string fitString(std::string s, VL_SZ nChars, std::string trail="", char fillChar=' ', W8 dir_=0)
	{
		//justo
		if (s.size()==nChars) 
			return s;
		//menos caracteres que o espaco
		if (s.size() < nChars) 
		{
			switch(dir_)
			{
				case 0:
						return (s+std::string(nChars - s.size(), fillChar));
				case 1:
				{
						W8 n = (W8)(nChars - s.size())>>1;
						std::string L = std::string(n, fillChar);
						std::string R = std::string(nChars-n-s.size(), fillChar);
						return ( L + s + R  );
				}
				case 2:
						return (std::string(nChars - s.size(), fillChar)+s);
			}
		}
		//supera
		return ( s.substr(0,nChars-trail.size()) + trail );
	}

	// retorna os intervalos diferentes na forma
	// <(pos,tam),(pos,tam),(pos,tam),..>
	inline void getStringsDiffs(std::string&a, std::string& b,
							   std::vector<std::pair<SW32, SW32>>& diffs,
							   W1 ignoreCaps = 1)
	{
		const SW32 l = MIN((SW32)a.length(),(SW32)b.length());
		diffs.clear();
		diffs.reserve(10);
		SW32 open = -1;
		if (ignoreCaps)
		{
			for (int i=0; i< l; i++)
			{
				if  ( VL_TO_UPPER(a[i]) == VL_TO_UPPER(b[i]) )
				{
					if (open!=-1)
					{
						diffs.push_back(std::pair<SW32,SW32>(open,i-open));
						open = -1;
					}
				}else
				{
					if (open==-1) open = i; 
				}
			} 
		}else
		{
			for (int i=0; i< l; i++)
			{
				if  ( a[i] ==  b[i] )
				{
					if (open!=-1)
					{
						diffs.push_back(std::pair<SW32,SW32>(open,i-open));
						open = -1;
					}
				}else
				{
					if (open==-1) open = i; 
				}
			}
		}
		if (open!=-1)
			diffs.push_back(std::pair<SW32,SW32>(open,l-open));
	}
	
	// retorna os intervalos semelhantes na forma
	// <(pos,tam),(pos,tam),(pos,tam),..>
	inline void getStringsSims(std::string&a, std::string& b,
							   std::vector<std::pair<SW32, SW32>>& diffs,
							   W1 ignoreCaps = 1)
	{
		const SW32 l = MIN((SW32)a.length(),(SW32)b.length());
		diffs.clear();
		diffs.reserve(10);
		SW32 open = -1;
		if (ignoreCaps)
		{
			for (int i=0; i< l; i++)
			{
				if  ( VL_TO_UPPER(a[i]) != VL_TO_UPPER(b[i]) )
				{
					if (open!=-1)
					{
						diffs.push_back(std::pair<int,int>(open,i-open));
						open = -1;
					}
				}else
				{
					if (open==-1) open = i; 
				}
			} 
		}else
		{
			for (int i=0; i< l; i++)
			{
				if  ( a[i] !=  b[i] )
				{
					if (open!=-1)
					{
						diffs.push_back(std::pair<SW32,SW32>(open,i-open));
						open = -1;
					}
				}else
				{
					if (open==-1) open = i; 
				}
			}
		}
		if (open!=-1)
			diffs.push_back(std::pair<SW32,SW32>(open,l-open));
	}
	
	// intercala a string a so' nos espacos (caracter sp_) de b
	inline std::string stringMix(std::string& a, std::string& b, char sp_=' ')
	{
		std::string res = (a.length()>b.length())?a:b;
		for  (VL_SZ i=0; i< MIN(b.length(),a.length()); i++)
		{
			res[i] = (b[i]==sp_)? a[i]: b[i];
		}
		return res;
	}

	// ler linhas dum ficheiro
	template< class OutputIt>
	inline void loadLines(std::string strFile, 
							OutputIt it)
	{
		//std::string strFile = "c:/development/ui/net/links.txt";
		FILE* f = fopen(strFile.c_str(), "r");
		//lines.reserve(reserve);
		char cstr[1000];
		std::string str;
		while(!feof(f))
		{
			cstr[0]=0;
			fgets(cstr, 1000, f);
			str = std::string(cstr);
			if (str!="")
			{
				if ( str[str.length()-1] ==  13)
					str = str.substr(0, str.length()-1);
				*it++ = str;
			}
		}
		fclose(f);
	}
	
	// -1 nao existe; -2 sem conteudo; 1 ok
	inline int  loadFile(const char*  filename, 
									char** b, 
									int* length_, 
									bool alloc_=true,		
									bool terminate=false)
	{
		FILE* file;
		file = fopen(filename,  "rb");
		if(!file) return -1;
    
		// tamanho do ficheiro
		fseek(file, 0, SEEK_END); 
		unsigned int leng = ftell (file);
		
		if (leng==0) 
			return -2;  
    
		fseek(file, 0, SEEK_SET); 
		char  *buffer ;

		if (alloc_)
			buffer =  new char[leng+(terminate?1:0)];
		else
			buffer = *b;

		fread(buffer, leng, 1, file);
		if (terminate)
			buffer[leng] = 0;

		*length_  = leng;

		if (alloc_)
			*b = buffer;
		
		fclose(file);
		return 0;
	}
	// um placeholder/ desenrasca so sabendo que a string e canonica 8, 
	// passa para 16
	 //UTF8 <=> WINDOWS MULTIBYTE
	#if (VL_SYSTEM_SYS==VL_SYSTEM_SYS_WINDOWS)
	inline std::wstring utf82wstr( std::string &str)
	{
		if( str.empty() ) return std::wstring();
		SW32 size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (SW32)str.size(), NULL, 0);
		std::wstring wstrTo( size_needed, 0 );
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (SW32)str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}
	inline std::string  wstr2utf8(const std::wstring &wstr)
	{
		if( wstr.empty() ) return std::string();
		SW32 size_needed = WideCharToMultiByte
			(CP_UTF8, 0, &wstr[0], (SW32)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo( size_needed, 0 );
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (SW32)wstr.size(), &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}
	#endif

	void loadLines(std::string strFile, 
							std::vector<std::string>& lines,
							int reserve = 50);
	
	// desmanchar url: nome do dominio, caminho, ficheiro e extensao
	void getHostAndPath(std::string str_, std::string& hostname_, std::string& path_, std::string& filename_, std::string& ext_)
	{
		size_t pos = str_.find('/');
		pos = str_.find('/',pos+1);
		pos = str_.find('/',pos+1);
		if (pos!=-1)
		{
			hostname_	=  str_.substr(0,pos); 
			path_		=  str_.substr(pos,str_.length()); 

			size_t  lastPos=-1;
			while (pos!=-1)
			{
				lastPos = pos;
				pos = str_.find('/',pos+1);
			}
			if (lastPos!=-1)
			{
				// ultima barra
				filename_	=  str_.substr(lastPos+1,str_.length()); 
				// extensao se houver, ultimo ponto
				if (filename_.length())
				{
					pos = -1; 
					do
					{
						lastPos = pos;
						pos = filename_.find('.', lastPos+1);	
					}while(pos!=-1);

					if (lastPos!=-1)
						ext_=  filename_.substr(lastPos+1,str_.length());
					else
						ext_ = "";
				}
			}
			return;
		}
		path_ ="";
		hostname_ = str_;
	}

	namespace gui
	{
	#if (VL_SYSTEM_SYS==VL_SYSTEM_SYS_WINDOWS)
			inline void setCursorPos(int X_, int Y_)
			{
				::SetCursorPos(X_,Y_);
			}
			inline void setHandCursor(W8 close_)
			{ 
				::SetCursor(vl::system::Enviro::get()->gui.grabCursor[close_]);
			}
			inline void setResizeWindowsCursor(int idx)
			{
				LPSTR cursor;
				switch(idx)
				{
					case 0:
						cursor = IDC_SIZENWSE;
					break;
					case 1:
						cursor = IDC_SIZENS;
					break;
					case 2:
						cursor = IDC_SIZENESW;
					break;
					case 3:
						cursor = IDC_SIZEWE;
					break;
					case 4:
						cursor = IDC_SIZENWSE;
					break;
					case 5:
						cursor = IDC_SIZENS;
					break;
					case 6:
						cursor = IDC_SIZENESW;
					break;
					case 7:
						cursor = IDC_SIZEWE;
					break;
					case -2:
						cursor = IDC_ARROW;
					break;
					default:
						cursor = IDC_HAND;
					break;
				}
				HCURSOR  hCursor = LoadCursor(NULL, cursor);
				SetCursor(hCursor);
			}
			struct handle_data 
			{
				unsigned long process_id;
				std::vector<HWND> handles;
			};
			inline BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
			{
				handle_data& data = *(handle_data*)lParam;
				unsigned long process_id = 0;
				GetWindowThreadProcessId(handle, &process_id);
				if (data.process_id != process_id)//|| !is_main_window(handle)) {
					return TRUE;
				data.handles.push_back(handle);
				return TRUE;   
			}
			inline std::vector<HWND> findWindows(SW32 process_id_=-1)
			{
				SW32 process_id;
				if (process_id_==-1)
					process_id = ::GetCurrentProcessId();
				else
					process_id = process_id_;

				handle_data data;
				data.process_id = process_id;
				EnumWindows(enum_windows_callback, (LPARAM)&data);
				return data.handles;
			}
			//OK: ret 1
			inline bool findProcessWindow(std::wstring name, HWND& hnd)
			{
				std::vector<HWND> hnds = findWindows();
				char buff[1000];
				for (VL_SZ i=0; i< hnds.size(); i++)
				{
					::GetWindowTextW(hnds[i], (LPWSTR)(buff), 1000);
					buff[999]=0;
					//std::cout << "\n"<< hnds[i] << std::wstring(buff);

					if ( !(wcscmp( (const wchar_t*)name.c_str(), (const wchar_t*)buff)) )
					{
						hnd = hnds[i];
						return 1;
					}
				}
				return 0;
			}
			inline std::vector<HWND> findVisibleWindows(std::string name)
			{
				std::vector<HWND> visibleWind; 
				PROCESSENTRY32 entry;	
				HANDLE snapshot =  CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
				while (Process32Next(snapshot, &entry) == TRUE) // navegar
				{
					if (  stricmp(entry.szExeFile, name.c_str()) == 0  ) 
					{
						std::vector<HWND> handles =  findWindows(entry.th32ProcessID);
						for (VL_SZ i =0; i< handles.size(); i++)
						{
							if (::IsWindowVisible(handles[i]) ) 
								visibleWind.push_back(handles[i]);  
						}
					}
				}
				return visibleWind;
			}
			inline bool copy(unsigned char* data_, int len)
			{
				HGLOBAL clipMem =  GlobalAlloc(GMEM_MOVEABLE, len);
				if (!clipMem)
					return false;
				HGLOBAL gmem = GlobalLock(clipMem);
				if (!gmem)
				{
					GlobalUnlock(clipMem);
					return false;
				}
				memcpy(gmem, data_, len);
				GlobalUnlock(clipMem);
				OpenClipboard(0);
				EmptyClipboard();
				SetClipboardData(CF_TEXT, clipMem);
				CloseClipboard();
				return true;
			}
			inline W64 askPasteLenght()
			{
				BOOL  clipAv = IsClipboardFormatAvailable(CF_TEXT);
				if (!clipAv)
					return 0;
				if (!OpenClipboard(NULL)) 
					return 0;
				HGLOBAL clipH = GetClipboardData(CF_TEXT);
				if (!clipH) 
				{
					CloseClipboard(); 
					return 0;
				}
				LPVOID clipStr = GlobalLock(clipH); 
				W64 l = strlen((char*)clipStr)+1;
				GlobalUnlock(clipH); 
				CloseClipboard(); 
				return l;
			}
			inline bool paste(unsigned char* buffer_, VL_SZ maxSz_)
			{
				BOOL  clipAv = IsClipboardFormatAvailable(CF_TEXT);
				if (!clipAv)
					return false;
				if (!OpenClipboard(NULL)) 
					return false; 
				HGLOBAL clipH = GetClipboardData(CF_TEXT);
				if (clipH) 
				{ 
					LPVOID clipStr = GlobalLock(clipH); 

					VL_SZ clipStrL = strlen((char*)clipStr);

					clipStrL = (clipStrL < maxSz_)?
								clipStrL: maxSz_-1;

					memcpy((char*)buffer_, (char*)clipStr, clipStrL);
		
					buffer_[clipStrL] = 0;
			   
					GlobalUnlock(clipH); 
				} 
				CloseClipboard(); 
				return true;
			}
	#endif

	};
// coisas parvas da consola
	namespace term
	{
		
		void getCursor(SW32 *x, SW32 *y)
		{
				CONSOLE_SCREEN_BUFFER_INFO csbi;
	 
				if (!GetConsoleScreenBufferInfo(
					GetStdHandle( STD_OUTPUT_HANDLE ),
					&csbi
				))
				return;

				*x =csbi.dwCursorPosition.X;
				*y =csbi.dwCursorPosition.Y;
		}

		void setCursor(SW32 x, SW32 y)
		{
			HANDLE                     hStdOut;
			hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
			COORD    homeCoords = { x, y };
			SetConsoleCursorPosition( hStdOut, homeCoords );
		}

		void cls()
		{
				HANDLE                     hStdOut;
				CONSOLE_SCREEN_BUFFER_INFO csbi;
				DWORD                      count;
				DWORD                      cellCount;
				COORD                      homeCoords = { 0, 0 };

				hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
				if (hStdOut == INVALID_HANDLE_VALUE) return;

				/* Get the number of cells in the current buffer */
				if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
				cellCount = csbi.dwSize.X *csbi.dwSize.Y;

				/* Fill the entire buffer with spaces */
				if (!FillConsoleOutputCharacter(
					hStdOut,
					(TCHAR) ' ',
					cellCount,
					homeCoords,
					&count
				)) return;

				/* Fill the entire buffer with the current colors and attributes */
				if (!FillConsoleOutputAttribute(
					hStdOut,
					csbi.wAttributes,
					cellCount,
					homeCoords,
					&count
				)) return;

				/* Move the cursor home */
				SetConsoleCursorPosition( hStdOut, homeCoords );
		}


		void setColor(SW32 colorFront_, SW32 colorBack_, SW32 light_)
		{
			HANDLE m_Screen   = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(m_Screen, 	((colorBack_<<4) | colorFront_) | (light_<< 3) );
		}


		void getCursor(int*, int*);
		
	}; 
// coisa parvas com ficheiros rápidas
	namespace file
	{

		// apanha nome dos ficheiros do diretório
		#ifdef VL_SYSTEM_UNIX
		inline bool getFilenames( std::string _path,  std::vector<std::string>& _files)
		{
			_files.clear();
			DIR *dp;
			struct dirent *dirp;

			if((dp  = opendir(_path.c_str())) == NULL)
				return false;

			while ((dirp = readdir(dp)) != NULL)
			{
				if (dirp->d_name[0]!='.')
					_files.push_back(std::string(dirp->d_name));
			}
			closedir(dp);
			return true;
		}
		#endif

	#if (VL_SYSTEM_SYS==VL_SYSTEM_SYS_WINDOWS)
		// option_- 0:todos  1:ficheiros 2: dirs
		inline bool getFilenames
				(std::string path_pattern_, 
				 std::vector<std::string>& filenames_, int option_=1)
		{
			#pragma comment(lib, "User32.lib")
			WIN32_FIND_DATA ffd;
			HANDLE hFind = INVALID_HANDLE_VALUE;
			DWORD dwError=0;

			size_t length_of_arg = path_pattern_.size();
			StringCchLength(path_pattern_.c_str(), MAX_PATH, &length_of_arg);

			if (length_of_arg > (MAX_PATH - 3))
			{
				return false;
			}

			hFind = FindFirstFile( path_pattern_.c_str(), &ffd);
			if (INVALID_HANDLE_VALUE == hFind) 
			{
				//precisa de return especializado
				return false;
			} 
			switch (option_)
			{
				case 0:
				{
					do
					{
						if (strcmp(ffd.cFileName,".")==0) continue;
						if (strcmp(ffd.cFileName,"..")==0) continue;
						filenames_.push_back(ffd.cFileName);
					}
					while (FindNextFile(hFind, &ffd) != 0);
				}break;
				case 1:
				{
					do
					{
						if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
							filenames_.push_back(ffd.cFileName);
					}
					while (FindNextFile(hFind, &ffd) != 0);
				}break;
				case 2:
				{
					do
					{
						if (strcmp(ffd.cFileName,".")==0) continue;
						if (strcmp(ffd.cFileName,"..")==0) continue;
						if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							filenames_.push_back(ffd.cFileName);
					}
					while (FindNextFile(hFind, &ffd) != 0);
				}break;
			}
			dwError = GetLastError();
			if (dwError != ERROR_NO_MORE_FILES) 
			{
				return false;
			}
			FindClose(hFind);
			return true;
		}
		#endif

		inline std::string getParentPathStr(std::string path_, char delim_)
		{
			if (path_.size()==0)
				return "";
			for (VL_SZ i=path_.size()-1; i>=-1; i--)
				if (path_[i]==delim_)
					return path_.substr(0,i);
			return "";
		}
		
		inline void getParentPathStr(std::string path_, std::string& parent_, std::string& curr_, char delim_)
		{
			if (path_.size()==0)
			{
				parent_ = "";
				curr_= "";
				return;
			}
			for (SW32 i=(SW32)path_.size()-1; i>=-1; i--)
			{
				if (path_[i]==delim_)
				{
					parent_ = path_.substr(0,i);
					curr_	= path_.substr(i,path_.length()-i);
					return ;
				}
			}
			parent_ = "";
			curr_= path_;
			return;
		}

		
		// desmancha fullpath em basepath e ficheiro
		void getBasePathAndFile(const std::string  path_,  
								std::string& basepath_, 
								std::string& filename_)
		{
			size_t pos = path_.find_last_of("\\/");
			basepath_ = (std::string::npos == pos) ? "" : path_.substr(0, pos + 1);
			filename_ = (std::string::npos == pos) ? "" : path_.substr(pos+1);
		}
		// retorna a fullpath e  extensao
		inline void getBasePathAndExt(const std::string a, std::string& fn, std::string& ext)
		{
			for (SW32 i=a.length()-1; i>-1; i--)
			{
				if  ( a[i] == '.') 
				{
					ext = a.substr(i+1,a.length()-i-1);
					fn = a.substr(0,i);
					return;
				}
			}
			fn = a;
			ext = "";
		}
		
		void getBasePathAndFileAndExt(const std::string  path_,  
										std::string& basepath_, 
										std::string& filename2_,
										std::string& ext_)
		{
			std::string filename;
			getBasePathAndFile(path_, basepath_, filename);
			getBasePathAndExt(filename, filename2_, ext_);
		}

		// desmancha fullpath em basepath e ficheiro
		std::string getBasePath(const std::string path)	
		{
			size_t pos = path.find_last_of("\\/");
			return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
		}

		int				loadText(const char*  filename,  char** buffer,  unsigned long* len);
		
		//c++
		unsigned long	getFileLength(std::ifstream& file);

		
		inline std::string getFilename(std::string path_)
		{
			std::string bp, fn;
			getBasePathAndFile(path_, bp, fn);
			return fn;
		}

	};

	
	class OneCallback
	{
	public:
		void *f,  *arg1, *arg2;
		bool async;
		int nargs;
		OneCallback()
		{
			f   = 0;
			nargs = 0;
			 
		}
		OneCallback(void(*f1_)(void*), void* arg1_, bool async_=false):
					 arg1(arg1_),async(async_)
		{
			nargs = 1;
			f   = (void*)f1_;
		}
		OneCallback(void(*f2_)(void*,void*), void* arg1_, void* arg2_, bool async_=false):
					 arg1(arg1_),arg2(arg2_),async(async_)
		{
			nargs = 2;
			f   = (void*)f2_;
		}
		inline void call()
		{
			#ifdef BOOST_THREAD_THREAD_HPP
			if (f && async)
			{
				if (async)
					{
						if (nargs==1)
							boost::thread th( (( void(*)(void*) )(f)),arg1);
						else
							boost::thread th((( void(*)(void*,void*) )(f)),arg1,arg2); 
					}
					return;
			} 
			#endif
			if (f)
			{
				if (nargs==1)
					(( void(*)(void*) )(f))(arg1);
				else
					(( void(*)(void*,void*) )(f))(arg1,arg2);
			}
		}
	};

};};



#endif