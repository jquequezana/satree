#pragma once

#include "XmlRpc.h"

#include <iostream>
#include <string>
#include <vector>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/progress.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

#include "arboretum/stDiskPageManager.h"
#include "arboretum/stSlimTree.h"
#include "arboretum/stResult.h"

#include "DataSetLoader.h"
#include "TimeSeries.h"

#include "RecordManager.h"
#include "stLinearSearch.h"
#include "SATreeAdapter.h"

#include "btree.h"
#include "ObjectT.h"


namespace fs = boost::filesystem;

#define  DEBUG_MODE

#define PAGE_SIZE 64000

class ImageSearchServer  : public XmlRpcServerMethod {
private:
	struct StringPath {
		char filename[128];
	};

	typedef tsss::TimeSeries tTimeSeries;			
	typedef tTimeSeries*  TimeSeriesPtr;

	typedef stDiskPageManager tstDiskPageManagerPtr;			
	typedef tstDiskPageManagerPtr* stDiskPageManagerPtr;

	typedef tsss::L2Distance tTimeSeriesDistanceEvaluator;			
	typedef tTimeSeriesDistanceEvaluator*  TimeSeriesDistanceEvaluatorPtr;

	//typedef stLinearSearch<tTimeSeries, tTimeSeriesDistanceEvaluator> DBMTree;
    typedef SATreeAdapter<tTimeSeries, tTimeSeriesDistanceEvaluator> DBMTree;

	typedef DBMTree tDBMTree;			
	typedef tDBMTree*  DBMTreePtr;

 
	typedef tDBMTree tMetricAccessMethod;
	typedef tDBMTree* MetricAccessMethodPtr;

    typedef ObjectT<std::string> String;
	typedef mds::BTree<String> Btree;

private:

	std::string  objectExtension;
	std::string dataExtension;

	std::string  sDatasetPath;
	std::vector< std::vector<std::string> > randomSetCollection;

	bool treeOpened;

	stDiskPageManagerPtr diskPageManager;
	MetricAccessMethodPtr tree;
    
	Btree* textIndex;
    bool TextTreeIsOpened;
    
	RecordManager<StringPath>* recordManager;


public:
 
	ImageSearchServer(XmlRpcServer* s)  ;

	~ImageSearchServer() ;

	void createAccessMethod( const std::string& name, bool &isOpen) ;

	void setLoader(std::string datasetName,std::string file);
	
	void setConfiguration(std::string datasetPath, std::string objectExtension);

	std::vector<std::string> getRandomSet(std::string datasetName);

	void insert( std::string image) ;

	std::vector<std::string> query( std::string query, int K) ;

	void execute(XmlRpcValue& params, XmlRpcValue& result);
};

ImageSearchServer::ImageSearchServer(XmlRpcServer* s)  
	: XmlRpcServerMethod("ImageSearch", s)
{
	tree = NULL;
	diskPageManager = NULL;
	recordManager = NULL;
    textIndex = NULL;
}


void ImageSearchServer::execute(XmlRpcValue& params, XmlRpcValue& result) {
	std::string operation = params[0];
	
	if(operation == "configuration") {
		std::string datasetPath = params[1];
		std::string dataExtension = params[2];
		this->setConfiguration(datasetPath, dataExtension);
	}
	if(operation == "getRandomSet") {
		std::string datasetPath = params[1];
		std::vector<std::string> randomSet = this->getRandomSet("/tmp/" + datasetPath);
		
		for (int i = 0; i < randomSet.size(); i++ ) {
			result[i] = randomSet[i];
		}
	}
	else if(operation == "query") {
		std::string queryObject = params[1];
		int kNearestNeighbors = params[2];
		std::vector< std::string > ret;
		ret = this->query(queryObject, kNearestNeighbors);

		for (int i = 0; i < ret.size(); i++ ) {
			result[i] = ret[i];
		}
	}

	else if(operation == "text_query") {
		std::string queryObject = params[1];
		int kNearestNeighbors = params[2];
		 
        String object = String(queryObject);
		Btree::iterator iter = textIndex->find(object);
		
		for (int i = 0; iter != textIndex->end() && i < kNearestNeighbors; i++ ) {
			result[i] = (*iter).m_pObj->m_Data;
            cout << result[i] << endl;
			iter++;
		}
	}
}


ImageSearchServer::~ImageSearchServer() {
	delete tree;
	delete diskPageManager;
}

void ImageSearchServer::createAccessMethod( const std::string& name, bool &isOpen) {
	
	if(tree == NULL) {
		tree = MetricAccessMethodPtr( new tMetricAccessMethod(diskPageManager) );
	}
    if (textIndex == NULL) {
        TextTreeIsOpened = false;
        DiskPageManager* pageManager = new DiskPageManager();
        if(!pageManager->open("/tmp/index.txt")) {
           TextTreeIsOpened = true;
           pageManager->create("/tmp/index.txt", 256);
        }
        textIndex = new Btree(pageManager, true);
    }
	recordManager = new RecordManager<StringPath>(name + ".objects");

	std::cout << "isOpen: " <<   treeOpened  << "\n";
	std::cout << "GetNumberOfObjects: " <<   tree->GetNumberOfObjects() << "\n";

	std::cout << "index_name: " <<   name << "\n";
	isOpen = this->treeOpened;
}


void ImageSearchServer::insert( std::string imageFile) {

	std::vector<double> featureVector;
	std::string line;
	std::string fullpath = this->sDatasetPath + "/" +  imageFile;

	int pos = fullpath.find(".");
	fullpath = fullpath.substr(0, pos) + this->dataExtension; // load feature vector 

	std::cerr << "insert-obj: " << fullpath << "\n";
	std::ifstream finput( fullpath.c_str() );

	std::vector<std::string> results;
	if(finput.is_open() == false) {
		std::cout << "fail open imageFile\n";	
		return;
	}

	while( std::getline(finput, line ) ) {
		std::stringstream scin(line);
		double value;
		int idx=0;
		while (scin >> value) {
			featureVector.push_back(value);
			idx++;
		}
	}
	int id = tree->GetNumberOfObjects();
	StringPath stringPath;
	strcpy(stringPath.filename, imageFile.c_str());
	recordManager->save(id,  stringPath);

	TimeSeriesPtr object = TimeSeriesPtr(new tTimeSeries(featureVector, id));
	tree->Add(object);
    String* obj = new String(imageFile);
    
    /* Already DataSet Textree */
    if (TextTreeIsOpened == false) {
        textIndex->insert(Btree::value_type(obj, 0));
    }
    
	std::cout << "insert: " <<   tree->GetNumberOfObjects() << "\n";
}

std::vector<std::string> ImageSearchServer::query( std::string query, int K)
{
	std::vector<std::string> results;
	
	std::vector<double> featureVector;
	std::string line;
	std::string fullpath = this->sDatasetPath + "/" +  query;
	
	int pos = fullpath.find(".");
	fullpath = fullpath.substr(0, pos) + this->dataExtension; // load feature vector 

	std::cerr << "query_path: " << fullpath << "\n";
	std::ifstream finput( fullpath.c_str() );

	if(finput.is_open() == false) {
		std::cerr << "open fail" << fullpath << "\n";
		return results;
	}

	while( std::getline(finput, line ) ) {
		std::stringstream scin(line);
		double value;
		int idx=0;
		while (scin >> value) {
			featureVector.push_back(value);
			idx++;
		}
	}


	std::cout << "fv_sz: " << featureVector.size() << std::endl;
	TimeSeriesPtr object = TimeSeriesPtr(new tTimeSeries(featureVector));
	stResult<tTimeSeries>* result = tree->NearestQuery(object, K);
	std::cout << "max_query_dist: " << result->GetMaximumDistance() << "\n";

	for (int i = 0; i < result->GetNumOfEntries(); i++)
	{
		int id = (*result)[i].GetObject()->GetID();
#ifdef  DEBUG_MODE
		std::cout << "id: " << id << std::endl;
#endif
		StringPath reg;
		recordManager->recover(id, reg);
		std::string rpath = reg.filename;
#ifdef  DEBUG_MODE
		std::cout << "rpath: " << rpath << std::endl;
#endif
		results.push_back(rpath);
	}
	delete result;

	return results;
}
 
void ImageSearchServer::setConfiguration(std::string datasetPath, std::string dataExtension){
	std::cout << "setConfiguration: " << "\n";
	
	if(this->sDatasetPath == datasetPath && this->dataExtension == dataExtension)  {
		if(tree != NULL)  {
			this->sDatasetPath = datasetPath;
			this->dataExtension = dataExtension;
			
			bool opened = false;
			std::string indexFileName = datasetPath + "." + dataExtension + "_index_lsh";
			DataSetLoader loader(datasetPath, this->dataExtension);
			this->objectExtension = loader.getImageExtension();

			std::cout << "datasetPath opened " << std::endl; 
			return;
		}
	}
	else {
		tree = NULL;
	}

	this->sDatasetPath = datasetPath;
	this->dataExtension = dataExtension;

	bool opened = false;
	 
	DataSetLoader loader(datasetPath, this->dataExtension);
	this->objectExtension = loader.getImageExtension();

	this->createAccessMethod(datasetPath + "_" + dataExtension + "_index_dbmtree", opened);

	if( tree->GetNumberOfObjects() == 0 ){
		loader.load();
		std::vector<fs::path> train = loader.getTrainDataSet();
		std::vector<fs::path> test = loader.getTestDataSet();
		
		for (int i = 0; i < train.size(); i++) {
			fs::path  fullPath  = train[i];
			std::string fvPath  = fullPath.string();
			std::string imgPath  = fullPath.filename().string();
			
			int pos = imgPath.find(".");
			imgPath = imgPath.substr(0, pos) + this->objectExtension;
			this->insert(imgPath);
		}
        tree->BulkLoad();
        //textIndex->insert(imageFile);
		//this->createAccessMethod(datasetPath + "_" + dataExtension + "_index_dbmtree", opened);
	}
}

std::vector<std::string> ImageSearchServer::getRandomSet(std::string datasetPath) {
	std::vector<std::string> resul;
	int index=rand()%40;

	std::string file= datasetPath + std::string("_random") + boost::lexical_cast<std::string>(index)+".txt";

	std::cout << "randomset filename : " << file << "\n";

	randomSetCollection.clear();
	setLoader(datasetPath, file);
	
	int pos = rand()  % randomSetCollection.size();;
	resul =  randomSetCollection [pos];
#ifdef  DEBUG_MODE

	for (int i = 0; i < resul.size(); i++)
	{
		std::cout << "random set: " << resul[i] << "\n";
	}
#endif DEBUG_MODE

	return resul;
}

void ImageSearchServer::setLoader(std::string datasetName,std::string file){

	fs::path path_name(file);
	if(fs::is_regular(path_name)){
		std::cout << "open randoms";
        std::ifstream reader(path_name.string().c_str());
		std::string buffer;
		std::vector<std::string> rset;
		while(std::getline(reader,buffer)){
			rset.push_back(buffer);
		}
		randomSetCollection.push_back(rset);
	}
	else {
		std::cout << "create randoms";
		DataSetLoader *loader = new DataSetLoader(sDatasetPath, objectExtension);

		loader->load();

		std::vector<std::string> rrr = loader->getFileNames();
		int tam=40;
		int i = 0;
		for (int i = 0; i < tam; i++)
		{
			std::string file = datasetName + std::string("_random") + boost::lexical_cast<std::string>(i) + ".txt";
			std::ofstream in(file.c_str());
			std::vector<std::string> rset = loader->getRandomSet(tam, in);
			randomSetCollection.push_back(rset);
			std::cout << "random_set : "  << file << "\n";
		}
	}
}

