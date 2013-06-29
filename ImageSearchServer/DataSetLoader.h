#pragma once

#ifndef cbir_dataset_loader_h
#define cbir_dataset_loader_h



// stl container and algorithm
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <ctime>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <set>

// boost utilities
#include <boost/lexical_cast.hpp> //para conversiones

#include "boost/filesystem/operations.hpp" 
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/fstream.hpp"    // ditto
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>


namespace fs=boost::filesystem;   


class DataSetLoader
{
public:

	DataSetLoader(std::string m_trainPath,std::string m_objExt)
		:	trainPath(m_trainPath),objExt(m_objExt)
	{

	}

	void load();

	std::vector<fs::path> getTrainDataSet(){return m_filesDta;}
	std::vector<fs::path> getTestDataSet(){ return m_querys;}

	std::vector<std::string> getFileNames(){
		std::vector<std::string> ret ;
		for (int i = 0; i < m_filesDta.size(); i++)
		{
			ret.push_back( m_filesDta[i].filename().string() );
		}
		return ret;
	}


	std::vector<std::string>  getRandomSet(int numberCh,std::ofstream & File){
		int i=0;
		std::set<int> ar;

		std::vector<std::string> ret;
		do{

			int index;
			index = rand()%m_filesDta.size();
			if (ar.find(index)== ar.end())
			{
				ret.push_back(m_filesDta[index].filename().string());
				ar.insert(index);
				File << m_filesDta[index].filename().string();
				File << "\n";
				i++;
			}
		}while( i < numberCh );
		File.close();

		return ret;
	}


	std::string getImageExtension();

protected:
	bool isFile(std::string filename);
	bool is_figure(std::string & filename);

	fs::path LoadDatabase(fs::path & sub_path);


private:

	std::string  imageExtension;

	std::string trainPath;
	std::string objExt;

	std::vector<fs::path> m_filesDta;
	std::vector<fs::path> m_querys;
};

void DataSetLoader::load(){

	srand((unsigned)time(0));

	fs::path directory(trainPath.c_str());

	//////////////////////////////////////////////////////////////////////////
	LoadDatabase(directory);
	/////////////////////////////
	/////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	fs::directory_iterator end_itr;
	for (fs::directory_iterator itr(directory);itr!=end_itr;++itr)
	{
		if (fs::is_directory(*itr))
		{
			fs::path sub(itr->path().string());
			fs::path selected = LoadDatabase(sub);
			this->m_querys.push_back(selected);
		}
	}
	//////////////////////////////////////////////////////////////////////////
}

bool DataSetLoader::isFile(std::string filename){
	return boost::iends_with(filename, objExt);

}
bool DataSetLoader::is_figure(std::string & filename){
	
	if ( boost::iends_with(filename,".jpg") ) {
		imageExtension = ".jpg";
		return true;
	} 
	else if  ( boost::iends_with(filename,".png") ) {
		imageExtension = ".png";
		return true;
	}
	return false;
}

std::string DataSetLoader::getImageExtension() {

	fs::path sub_path(trainPath.c_str());

	fs::directory_iterator end_itr;
	for (fs::directory_iterator itr(sub_path);itr!=end_itr;++itr)
	{
		if (fs::is_regular(*itr))
		{ 
			std::string linkdir=itr->path().string();

			if (is_figure(linkdir))
			{
				return imageExtension;
			}
		}
	}
	return "null";
}


fs::path DataSetLoader::LoadDatabase(fs::path & sub_path){

	std::vector<fs::path> querys;

	fs::directory_iterator end_itr;
	for (fs::directory_iterator itr(sub_path);itr!=end_itr;++itr)
	{
		if (fs::is_regular(*itr))
		{ 
			std::string linkdir=itr->path().string();

			if (isFile(linkdir))
			{
				m_filesDta.push_back(*itr);
				querys.push_back(*itr);
			}
		}
	}

	int index=rand()%querys.size();
	return querys[index];
}


#endif 