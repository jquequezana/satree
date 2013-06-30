// HelloServer.cpp : Simple XMLRPC server example. Usage: HelloServer serverPort
//
#include "XmlRpc.h"

using namespace XmlRpc;

#include <iostream>
#include <stdlib.h>

#include "ImageSearchServer.h"


#define  DEBUG_MODE

// The server
XmlRpcServer s;
 
 

int main(int argc, char* argv[])
{
	std::vector<std::string> lst;

	ImageSearchServer imageSearch(&s);

	//std::string datasetPath = "/Users/jquequezana/workspace/files/flickr";
	//std::string dataExtension  = ".rgb";

	std::string datasetPath = "/Users/jquequezana/workspace/files/facedb2";
	std::string dataExtension  = ".sfi.pca";
	
    imageSearch.setConfiguration(datasetPath, dataExtension);
	//imageSearch.getRandomSet(datasetPath);
	imageSearch.query("01.jpg", 20 );
/*
	datasetPath = "C:/xampp/htdocs/cbir/Caltech-256";
	dataExtension  =   ".jpg.oRGBHistograms";

	imageSearch.setConfiguration(datasetPath, dataExtension);
	//imageSearch.getRandomSet(datasetPath);
	imageSearch.query("024_0056.jpg", 10 );*/



/*	datasetPath = "C:/xampp/htdocs/cbir/PIBAP";
	dataExtension  =   ".jpg.eGeometricFeatures2";

	imageSearch.setConfiguration(datasetPath, dataExtension);
	imageSearch.getRandomSet(datasetPath);
	imageSearch.query("1_2_1.jpg", 10 );*/

	int port = 9090;

	XmlRpc::setVerbosity(5);

	// Create the server socket on the specified port
	s.bindAndListen(port);

	// Enable introspection
	s.enableIntrospection(true);

	// Wait for requests indefinitely
	s.work(-1.0);

	return 0;
}

