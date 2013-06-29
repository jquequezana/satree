//
//  SATreeRaw.h
//  ImageSearchServer
//
//  Created by Jan Quequezana on 29/06/13.
//  Copyright (c) 2013 Jan Quequezana. All rights reserved.
//

#ifndef ImageSearchServer_SATreeRaw_h
#define ImageSearchServer_SATreeRaw_h

#include <cstdlib>
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>
#include <ctime>
#include <limits>
#include <cmath>
#include <cassert>
#include <queue>
using namespace std;


template<typename T,typename DistFinder>
class SATreeAdaptee
{
    struct Node
    {
        T* ptr_data;
        float covering_radius;
        std::vector<Node*> neighbours;
        Node(const T& data):covering_radius(0.0f),ptr_data(&data)
        {}
        Node():ptr_data(0),covering_radius(0.0f){}
    };
    struct AElement
    {
        Node* node;
        float distance;
        AElement()
        {}
        AElement(Node* n,const float& d):node(n),distance(d)
        {
            
            
        }
        struct Sorter
        {
            bool operator()(const AElement& a,const AElement& b)
            {
                return a.distance <= b.distance;
                }
                };
                };
                struct QElement
                {
                    Node* node;
                    float lbound,dmin;
                    QElement(){}
                    QElement(Node* n,const float& lb,const float& d):node(n),lbound(lb),dmin(d)
                    {
                        
                    }
                    struct Sorter
                    {
                        bool operator()(const QElement& a,const QElement& b)
                        {
                            return a.lbound > b.lbound;
                        }
                    };
                };

                typename std::map<std::pair<Node*,Node*>,float> distances; //fromNode to Nodei
                friend struct PtrNodesComparer;
                struct PtrNodesComparer
                {
                    SATreeAdaptee* ptr_tree;
                    Node* node2Compare;
                    PtrNodesComparer(SATreeAdaptee* ptree,Node* tocomp_node):ptr_tree(ptree),node2Compare(tocomp_node){
                    }
                    bool operator()(Node* a,Node* b)
                    {
                        float dist_a,dist_b;
                        if(!ptr_tree->getDistance(node2Compare,a,dist_a))
                            dist_a = ptr_tree->calculateDistance(node2Compare,a);
                        if(!ptr_tree->getDistance(node2Compare,b,dist_b))
                            dist_b = ptr_tree->calculateDistance(node2Compare,b);
                        return dist_a < dist_b;
                    }
                };
            public:
                Node* root;
                Node* allNodes;
                
                SATreeAdaptee()
                {
                    
                }
                ~SATreeAdaptee()
                {
                    delete[] allNodes;
                }
                float calculateDistance(Node* fromNode,Node* toNode)
                {
                    DistFinder calculator;
                    return (distances[std::pair<Node*,Node*>(fromNode,toNode)] = calculator(*(fromNode->ptr_data),*(toNode->ptr_data)));
                }
                void calculateDistances(Node* fromNode,const std::vector<Node*>& rest_nodes)
                {
                    DistFinder calculator;
                    for(typename std::vector<Node*>::const_iterator iter = rest_nodes.begin();iter != rest_nodes.end();++iter)
                        distances[std::pair<Node*,Node*>(fromNode,(*iter))] = calculator(*(fromNode->ptr_data),*((*iter)->ptr_data));
                }
                bool getDistance(Node* fromNode,Node* toNode,float& distance) /*false if distance not calculated*/
                {
                    typename std::map<std::pair<Node*,Node*>,float>::const_iterator res_it;
                    if(((res_it = distances.find(std::pair<Node*,Node*>(fromNode,toNode))) != distances.end()) || ((res_it = distances.find(std::pair<Node*,Node*>(toNode,fromNode))) != distances.end()))
                    {
                        distance = res_it->second;
                        return true;
                    }
                    return false;
                }
                
                void getNodesIncresingDist(Node* fromNode,const std::vector<Node*>& rest_nodes,std::vector<Node*>& result)
                {
                    result.assign(rest_nodes.begin(),rest_nodes.end());
                    PtrNodesComparer comparer(this,fromNode);
                    std::sort(result.begin(),result.end(),comparer);
                }
                
                void insert(Node* a,std::vector<Node*>& node_set)
                {
                    std::sort(node_set.begin(), node_set.end());
                    bool condition = true;
                    calculateDistances(a,node_set);
                    std::vector<Node*> nodesInIncrDistance;
                    getNodesIncresingDist(a,node_set,nodesInIncrDistance);
                    float dist_b;
                    float min;
                    Node* min_node;
                    for(typename std::vector<Node*>::iterator it = nodesInIncrDistance.begin();it != nodesInIncrDistance.end();++it)
                    {
                        a->covering_radius = std::max(a->covering_radius,distances[std::pair<Node*,Node*>(a,(*it))]);
                        condition = true;
                        for(typename std::vector<Node*>::const_iterator niterator = a->neighbours.begin(); niterator != a->neighbours.end() ;++niterator)
                        {
                            if(!getDistance(*it,*niterator,dist_b))
                                dist_b = calculateDistance(*it,*niterator);
                            if(distances[std::pair<Node*,Node*>(a,*it)] >= dist_b)
                            {
                                condition = false;
                                break;
                            }
                        }
                        if(condition)
                            a->neighbours.push_back(*it);
                        
                    }
                    
                    std::sort(a->neighbours.begin(),a->neighbours.end());
                    std::list<Node*> difference;
                    std::set_difference(node_set.begin(),node_set.end(),a->neighbours.begin(),a->neighbours.end(),std::insert_iterator<std::list<Node*> >(difference, difference.end()));
                    for(typename std::list<Node*>::const_iterator ite = difference.begin();ite != difference.end();++ite)
                    {
                        min = std::numeric_limits<float>::max();
                        min_node = 0;
                        for(typename std::vector<Node*>::const_iterator niterator = a->neighbours.begin(); niterator != a->neighbours.end() ;++niterator)
                        {
                            if(!getDistance(*ite,*niterator,dist_b))
                                dist_b = calculateDistance(*ite,*niterator);
                            if(dist_b < min)
                            {
                                min = dist_b;
                                min_node = *niterator;
                                break;
                            }
                        }
                        assert(min_node);
                        min_node->neighbours.push_back(*ite);
                    }
                    
                    for(typename std::vector<Node*>::iterator niterator = a->neighbours.begin(); niterator != a->neighbours.end() ;++niterator)
                    {
                        if((*niterator)->neighbours.size() > 0)
                            insert(*niterator,(*niterator)->neighbours);
                    }
                }
                void insert(const std::vector<T*>& ds)
                {
                    size_t i = 0, u = 0;
                    allNodes = new Node[ds.size()];
                    for(typename std::vector<T*>::const_iterator ite = ds.begin();ite != ds.end();++ite,++i)
                        allNodes[i].ptr_data = *ite;
                    size_t rand_idx = rand() % ds.size();
                    root = &(allNodes[rand_idx]);
                    std::vector<Node*> initialSet(ds.size() - 1);
                    for(i = 0;i<ds.size();++i)
                        if(i != rand_idx)
                            initialSet[u++] = &(allNodes[i]);
                    insert(root,initialSet);
                    
                    
                }
                void rangeQuery(Node* a,const T& query_obj,const float& r,float dmin,std::map<Node*,float>& map_helper,std::list<T*>& resultSet)
                {
                    static DistFinder finder;
                    static typename std::map<Node*,float>::iterator miter;
                    float daq;
                    if((miter = map_helper.find(a)) != map_helper.end())
                        daq = miter->second;
                    else
                        daq = map_helper[a] = finder(query_obj,*(a->ptr_data));
                    
                    float d_elmq;
                    if(daq <= a->covering_radius + r)
                    {
                        if(daq <= r)
                            resultSet.push_back(a->ptr_data);
                        for(typename std::vector<Node*>::const_iterator iter = a->neighbours.begin();iter != a->neighbours.end();++iter)
                        {
                            if((miter = map_helper.find(*iter)) != map_helper.end())
                                d_elmq = miter->second;
                            else
                                d_elmq = map_helper[*iter] = finder(query_obj,*((*iter)->ptr_data));
                            if(d_elmq < dmin)
                                dmin = d_elmq;
                        }
                        for(typename std::vector<Node*>::const_iterator iter = a->neighbours.begin();iter != a->neighbours.end();++iter)
                        {
                            if((miter = map_helper.find(*iter)) != map_helper.end())
                                d_elmq = miter->second;
                            else
                                d_elmq = map_helper[*iter] = finder(query_obj,*((*iter)->ptr_data));
                            if(d_elmq <= dmin + 2 * r)
                                rangeQuery(*iter,query_obj,r,dmin,map_helper,resultSet);
                        }
                        
                    }
                }
                void rangeQuery(const T& query_obj,const float& r,std::list<T*>& resultSet)
                {
                    DistFinder calc;
                    std::map<Node*,float> temp_map;
                    float dmin = temp_map[root] = calc(query_obj,*(root->ptr_data));
                    rangeQuery(root,query_obj,r,dmin,temp_map,resultSet);
                }
                void kNNQuery(const T& query_obj,const size_t& k,std::list<T*>& resultSet)
                {
                    std::priority_queue<QElement,std::vector<QElement>,typename QElement::Sorter> Q;
                    std::priority_queue<AElement,std::vector<AElement>,typename AElement::Sorter> A;
                    std::map<Node*,float> mdistances_q;
                    DistFinder finder;
                    mdistances_q[root] = finder(*(root->ptr_data),query_obj);
                    typename std::map<Node*,float>::iterator ite;
                    Q.push(QElement(root,std::max(0.0f,mdistances_q[root] - root->covering_radius),mdistances_q[root]));
                    float r = std::numeric_limits<float>::max(),dmin,lbound,d_qb;
                    Node* a;
                    while(!Q.empty())
                    {
                        dmin = Q.top().dmin;
                        a = Q.top().node;
                        lbound = Q.top().lbound;
                        Q.pop();
                        
                        if(lbound > r)
                            break;
                        
                        if((ite = mdistances_q.find(a)) != mdistances_q.end())
                            A.push(AElement(a,ite->second));
                        else
                            A.push(AElement(a,(mdistances_q[a] = finder(*(a->ptr_data),query_obj))));
                        
                        
                        if(A.size() > k)
                            A.pop();
                        else if(A.size() == k)
                            r = A.top().distance;
                        
                        for(typename std::vector<Node*>::const_iterator iter = a->neighbours.begin();iter != a->neighbours.end();++iter)
                        {
                            if((ite = mdistances_q.find(*iter)) != mdistances_q.end())
                                d_qb = ite->second;
                            else
                                d_qb = mdistances_q[*iter] = finder(*((*iter)->ptr_data),query_obj);
                            if(d_qb < dmin)
                                dmin = d_qb;
                        }
                        for(typename std::vector<Node*>::const_iterator iter = a->neighbours.begin();iter != a->neighbours.end();++iter)
                        {
                            if((ite = mdistances_q.find(*iter)) != mdistances_q.end())
                                d_qb = ite->second;
                            else
                                d_qb = mdistances_q[*iter] = finder(*((*iter)->ptr_data),query_obj);
                            Q.push(QElement(*iter,std::max((d_qb - dmin)/2.0f,d_qb - (*iter)->covering_radius),dmin));
                        }
                    }
                    
                    while(!A.empty())
                    {
                        resultSet.push_front(A.top().node->ptr_data);
                        A.pop();
                    }
                }
                
                };
#endif
