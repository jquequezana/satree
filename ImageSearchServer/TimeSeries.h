//---------------------------------------------------------------------------
#ifndef TIME_SERIES_H_
#define TIME_SERIES_H_


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
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <ctime>

// boost utilities
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp> //para conversiones

// arboretum utilities
#include <arboretum/stMetricAccessMethod.h>
#include <arboretum/stTypes.h>
#include <arboretum/stUserLayerUtil.h>
#include <arboretum/stTypes.h>
#include <arboretum/stUtil.h>

namespace tsss {
	//---------------------------------------------------------------------------
	// Class TimeSeries
	//---------------------------------------------------------------------------
	class TimeSeries{
	public:
		TimeSeries(){
			// Invalidate Serialized buffer.
			Serialized = NULL;
			ID = -1;
		}//end TimeSeries
		TimeSeries(double values[], int dim, int id = 1){
			Serialized = NULL;
			ID = id;
			for(int i = 0; i < dim; i++)
				Values.push_back ( values[i] );
			// Invalidate Serialized buffer.
		}//end TimeSeries

		TimeSeries(std::vector<double> values, int id = 1){
			Serialized = NULL;
			ID = id;
			for(int i = 0; i < values.size(); i++)
				Values.push_back ( values[i] );
			// Invalidate Serialized buffer.
		}//end TimeSeries


		~TimeSeries(){
			// Does Serialized exist ?
			if (Serialized != NULL){
				// Yes! Dispose it!
				delete [] Serialized;
			}//end if
		}//end TimeSeries

		std::vector<double> &GetValues() {
			return Values;
		}

		double& operator [] (int i){
			return Values[i];
		}
		const double operator [] (int i) const{
			return Values[i];
		}

		double& get(int i){
			return Values[i];
		}
		const double get(int i) const{
			return Values[i];
		}

		int GetDimension() const{
			return Values.size();
		}
		// The following methods are required by the stObject interface.
		/**
		* Creates a perfect clone of this object. This method is required by
		* stObject interface.
		*
		* @return A new instance of TimeSeries wich is a perfect clone of the original
		* instance.
		*/
		TimeSeries * Clone(){
			return new TimeSeries(Values, this->ID);
		}//end Clone

		/**
		* Checks to see if this object is equal to other. This method is required
		* by  stObject interface.
		*
		* @param obj Another instance of TimeSeries.
		* @return True if they are equal or false otherwise.
		*/
		bool IsEqual(TimeSeries *obj){
			if(obj->GetDimension() != this->GetDimension())
				return false;

			int mindim =  obj->GetDimension()  < this->GetDimension() ? obj->GetDimension() : this->GetDimension();
			for(int i = 0; i < mindim ; i++) {
				if( Values[i] != obj->Values[i])
					return false;
			}
			return true;
		}//end IsEqual

		/**
		* Returns the size of the serialized version of this object in bytes.
		* This method is required  by  stObject interface.
		*/
		stSize GetSerializedSize(){
			return GetDimension()* sizeof(double) + sizeof(int) + sizeof(int);
		}//end GetSerializedSize

		const stByte * Serialize(){
			double * d;

			// Is there a seralized version ?
			if (Serialized == NULL){
				// No! Lets build the serialized version.

				// The first thing we need to do is to allocate resources...
				Serialized = new stByte[GetSerializedSize()];
				int *idxInt = (int *) Serialized;
				*idxInt = this->ID;

				idxInt = (int *) (Serialized + sizeof(int)); // If you ar not familiar with pointers, this
				*idxInt = GetDimension();

				d = (double *) (Serialized + sizeof(int) + sizeof(int)); // If you ar not familiar with pointers, this
				// action may be tricky! Be careful!

				for(int i=0; i < GetDimension(); i++)
					d[i] = Values[i];

			}//end if

			return Serialized;
		}//end TimeSeries::Serialize


		void Unserialize(const stByte *data, stSize datasize){

			// Now we know the size, lets get it from the serialized version.
			double *d;
			int * idxInt = (int *) data;  
			this->ID = *idxInt;

			idxInt = (int *) (data + sizeof(int)); // If you ar not familiar with pointers, this
			int new_size = *idxInt;

			d = (double *) (data + sizeof(int) + sizeof(int));  // If you ar not familiar with pointers, this
			// action may be tricky! Be careful!

			Values.clear();
			for(int i = 0; i < new_size; i++)
				Values.push_back(d[i]);

			if (Serialized != NULL){
				delete [] Serialized;
				Serialized = NULL;
			}//end if
		}//end TimeSeries::Unserialize


		void SetID(int id ){ ID = id;}

		int GetID() const {return ID;}

	private:

		//DISALLOW_COPY_AND_ASSIGN(TimeSeries);

		std::vector<double>	Values;
		int					ID;
		stByte *			Serialized;
	};//end TMapPoint


	/////////////////////////////////////////////////////////////////////////
#define SUPERIOR  1
#define IZQUIERDA 2
#define DIAGONAL  3

	using std::vector;

	class TimeSeriesDistanceEvaluator : public stMetricEvaluatorStatistics{
	public:

		typedef TimeSeries					  tTimeSeries; 
		typedef boost::shared_ptr<TimeSeries> TimeSeriesPtr; 

		stDistance GetDistance(TimeSeries* obj1, TimeSeries* obj2){
			return DTW(obj1->GetValues(), obj2->GetValues());
		}

		stDistance GetDistance(TimeSeriesPtr obj1, TimeSeriesPtr obj2){
			return DTW(obj1->GetValues(), obj2->GetValues());
		}

		stDistance computeDotProduct(TimeSeriesPtr obj1, std::vector<double> v) {

			TimeSeriesPtr obj2 = TimeSeriesPtr( new tTimeSeries(v) );
			GetDistance(obj1, obj2);

			double timeSeriesValue1, timeSeriesValue2;
			double sum = 0.0;

			for(int i = 0; i < path.size(); i++) {
				timeSeriesValue1 = obj1->get(path[i].first); 
				timeSeriesValue2 = obj2->get(path[i].second); 
				sum += timeSeriesValue1 * timeSeriesValue2;
			}
			//return (sqrt( sum )/ P2[obj1->GetDimension()][obj2->GetDimension()]);//ponderado
			return sum;
		}


		double DTW(std::vector<double>& pfVx,std::vector<double>& pfVy) {
			path.clear();
			matrix_path2.clear();

			int sizex = pfVx.size();
			int sizey = pfVy.size();
			double min;
			int i,j;
			int dir;
			vector<vector<double> > D((sizex+1),vector<double>(sizey+1,0.0));
			vector<vector<double> > P((sizex+1),vector<double>(sizey+1,0.0));
			vector<vector<int> > matrix_path((sizex+1),vector<int>(sizey+1,0));

			D[1][1]=pow((pfVx[0]-pfVy[0]),2);
			P[1][1]=SUPERIOR;
			matrix_path[1][1]=SUPERIOR;

			for(i=2;i<=sizex;i++)
			{
				D[i][1]=pow((pfVx[i-1]-pfVy[0]),2)+ D[i-1][1];
				P[i][1]=i;
			}
			for(i=2;i<=sizey;i++)
			{
				D[1][i]=pow((pfVx[0]-pfVy[i-1]),2)+ D[1][i-1];
				P[1][i]=i;
			}

			for (i=2;i<=sizex;i++){
				for(j=2;j<=sizey;j++)
				{
					min= D[i-1][j-1];
					dir=DIAGONAL;

					if (D[i-1][j]<min)
					{
						min= D[i-1][j];
						dir=SUPERIOR;
						if (D[i][j-1]<min)
						{
							min= D[i][j-1];
							dir=IZQUIERDA;
						}
					}
					else
						if (D[i][j-1]<min)
						{
							min= D[i][j-1];
							dir=IZQUIERDA;
						}
						D[i][j]= pow( (pfVx[i-1]-pfVy[j-1]),2) +(float) min;
						P[i][j]=P[i-(dir & 1)][j-((dir&2)/2)]+1;
						matrix_path[i][j]=dir;
				}
			}
			matrix_path2=matrix_path;

			//trackPath(sizex-1, sizey-1);

			//for(i=0;i<path.size();i++)
			//	std::cout<<"("<<path[i].first<<"-"<<path[i].second<<"),";
			P2 = P;
			return (sqrt(D[sizex][sizey])/ P[sizex][sizey]);//ponderado
			//return D[sizex][sizey];
		}
		void trackPath(int pos_x, int pos_y)
		{
			if(pos_x==0 && pos_y==0) {
				path.push_back(std::pair<int,int>(pos_x, pos_y));
				return;
			}
			//std::cout<<"("<<pos_x<<"-"<<pos_y<<")"<<endl;
			switch(matrix_path2[pos_x][pos_y]) {
	case SUPERIOR:
		trackPath(pos_x-1, pos_y/*, matrix_path2/*, p*/);
		break;
	case DIAGONAL:
		trackPath(pos_x-1, pos_y-1/*, matrix_path2/*, p*/);
		break;
	case IZQUIERDA:
		trackPath(pos_x, pos_y-1/*, matrix_path2/*, p*/);
		break;
			}
			path.push_back(std::pair<int,int>(pos_x, pos_y));
		}

	private:
		std::vector<std::pair<int,int> > path;//Vector wich contains the path
		std::vector<std::vector<int> > matrix_path2;//Temporary Matrix
		std::vector<std::vector<double> > P2;

	};//end TVectorDistanceEvaluator



	class L2Distance : public stMetricEvaluatorStatistics {
	public:
		typedef TimeSeries					  tTimeSeries; 
		typedef boost::shared_ptr<TimeSeries> TimeSeriesPtr; 

		stDistance computeDotProduct(TimeSeriesPtr obj1, std::vector<double> v) {
			return 0;
		}

		stDistance GetDistance(TimeSeriesPtr obj1, TimeSeriesPtr obj2){
			return GetDistance(obj1.get(), obj2.get());
		}

		double GetDistance (TimeSeries* obj1, TimeSeries* obj2) {
			stDistance distance = 0.0f;
			for(int i = 0; i < obj1->GetDimension(); i++ ) 
			{
				distance += pow((*obj2)[i] - (*obj1)[i], 2);
			}
			this->UpdateDistanceCount();
			return sqrt(distance);
		}
	};

	//---------------------------------------------------------------------------
	// Output operator
	//---------------------------------------------------------------------------
	/**
	* This operator will write a string representation of a city to an outputstream.
	*/
	std::ostream & operator << (std::ostream & out, const TimeSeries & v){
		std::cout << "idObj: " << v.GetID() << " => \t ";
		for( int k = 0 ; k < v.GetDimension(); k++)
			out << v[k] << ", ";
		return out;
	}//end operator <<

}


#endif //end 
