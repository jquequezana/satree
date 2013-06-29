

#ifndef FS_DATAFILE_H_
#define FS_DATAFILE_H_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>


template<class Register>
class RecordManager :public std::fstream{
public:
	RecordManager(std::string file_name) 
		: std::fstream(file_name.data(), std::ios::in | std::ios::out | std::ios::binary ) 
	{
		fileName = file_name;
		if(is_open() == false) 
		{
			open(file_name.data(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary );
		}
	}
	//~RecordManager() {
	//	close();
	//}
	void save(Register &reg);
	void save(const long &n ,Register &reg);
	bool recover(const long &n ,Register &reg);
	void erase(const long &n) ;

private:
	string fileName;
	void package();

};
//---------------------------------------------------------------------

//---------------------------------------------------------------------
template<class Register>
void RecordManager<Register>::save(const long &n ,Register &reg) 
{
	clear();
	seekp(n*sizeof(Register), std::ios::beg);
	write(reinterpret_cast<const char *> (&reg), sizeof(reg));
}

template<class Register>
void RecordManager<Register>::save(Register &reg) 
{
	clear();
	seekp(0, std::ios::end);
	write(reinterpret_cast<const char *> (&reg), sizeof(reg));
}

template<class Register>
bool RecordManager<Register>::recover(const long &n, Register &reg) 
{
	clear();
	seekg(n*sizeof(Register), std::ios::beg);
	read(reinterpret_cast< char *> (&reg), sizeof(reg));
	return gcount() > 0;
}

// Marca el registro como borrado:
template<class Register>
void RecordManager<Register>::erase(const long &n) 
{
	char mark;
	clear();
	mark = 'N';
	seekg(n * sizeof(Register), std::ios::beg);
	write(&mark, 1); 
}

#endif
//---------------------------------------------------------------------
