#ifndef ImageSearchServer_SATree_h
#define ImageSearchServer_SATree_h

template <class ObjectType, class EvaluatorType>
class stLinearSearch : public stMetricTree<ObjectType, EvaluatorType> {
                
public:
    
    typedef ObjectType tObject;
    typedef EvaluatorType tMetricEvaluator;
    typedef stResult <ObjectType> tResult;
    typedef std::vector<tObject*> tDataset;
    
    
    stLinearSearch(stPageManager * pageman) : stMetricTree<ObjectType, EvaluatorType> (pageman)
    {
        
    }
    
    virtual bool Add(tObject * obj) {
        this->dataset.push_back(obj);
        return true;
    }
    
    virtual void BulkLoad() {
    }
    
    virtual long GetNumberOfObjects(){
        return dataset.size();
    }
    
    
    virtual tResult * RangeQuery(tObject * sample, stDistance range) {
        tResult *result = new tResult();
        result->SetQueryInfo(sample->Clone(), tResult::RANGEQUERY, -1, range, false);
        
        for (int i = 0; i < dataset.size (); i++) {
            tObject* obj = dataset[i];
            double dist = metric.GetDistance(sample, obj);
            if (dist < range) {
                result->AddPair(obj->Clone(), dist);
            }
        }
        return result;
    }
    
    virtual tResult * NearestQuery(tObject * sample, stCount k, bool tie = true) {
        tResult *result = new tResult();
        result->SetQueryInfo(sample->Clone(), tResult::KNEARESTQUERY, k, -1.0, tie);
        
        for (int i = 0; i < dataset.size (); i++) {
            tObject* obj = dataset[i];
            double dist = metric.GetDistance(sample, obj);
            result->AddPair(obj->Clone() , dist);
            
        }
        result->Cut(k);
        return result;
    }
    
private:
	std::vector<tObject*> dataset;
	tMetricEvaluator metric;
    
};


#endif