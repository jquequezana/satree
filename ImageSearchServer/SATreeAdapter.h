//
//  SATreeAdapter.h
//  ImageSearchServer
//
//  Created by Jan Quequezana on 29/06/13.
//  Copyright (c) 2013 Jan Quequezana. All rights reserved.
//

#ifndef ImageSearchServer_SATreeAdapter_h
#define ImageSearchServer_SATreeAdapter_h

#include "SATreeAdaptee.h"
#include "stLinearSearch.h"

template <class ObjectType, class EvaluatorType>
class SATreeAdapter : public stLinearSearch<ObjectType, EvaluatorType> {
    
    struct DistanceAdapter {
        float operator() (ObjectType first, ObjectType second) {
            EvaluatorType comparator;
            return comparator.GetDistance(&first, &second);
        };
    };
    
  public:
    typedef ObjectType tObject;
    typedef EvaluatorType tMetricEvaluator;
    typedef stResult <ObjectType> tResult;
    typedef std::vector<tObject*> tDataset;
    
    SATreeAdapter(stPageManager * pageman) : stLinearSearch<ObjectType, EvaluatorType>(pageman) {
    
    }
    
    bool Add(tObject * obj) {
        this->dataset.push_back(obj);
        return true;
    }
    
    void BulkLoad() {
        tree_.insert(this->dataset);
    }
    
    long GetNumberOfObjects(){
        return this->dataset.size();
    }
    
    
    tResult * RangeQuery(tObject * sample, stDistance range) {
        tResult *result = new tResult();
        std::list<ObjectType*> results;

        tree_.rangeQuery(*sample, range, results);
        
        for (typename std::list<ObjectType*>::iterator it = results.begin(); it != results.end(); it++) {
            result->AddPair((*it)->Clone(), range);
        }
        
        return result;
    }
    
    tResult * NearestQuery(tObject * sample, stCount k, bool tie = true) {
        tResult *result = new tResult();
        std::list<ObjectType*> results;
        
        tree_.kNNQuery(*sample, k, results);
        
        for (typename std::list<ObjectType*>::iterator it = results.begin(); it != results.end(); it++) {
            result->AddPair((*it)->Clone(), 1);
        }
        
        return result;
    }
    
  private:
    tDataset dataset;
    SATreeAdaptee<ObjectType, DistanceAdapter> tree_;
    
};

#endif
