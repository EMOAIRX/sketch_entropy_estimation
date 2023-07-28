#ifndef _Estimator_H_
#define _Estimator_H_

#include "MurMurHash.h"

#include <vector>
#include <iostream>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;
class Estimator{
    
    public:
    struct Buckets{
        int IP;
        int id;
        int count;
    };
    vector<Buckets> buckets;
    int total;
    int bucketsnum;
    map<int , map<int , int> > real;
    // init
    Estimator(int mem){
        total = 0;
        bucketsnum = mem / sizeof(Buckets);
        // printf("bucketsnum = %d\n", bucketsnum);
        buckets.resize(bucketsnum);
        for(int i = 0; i < bucketsnum; i++){
            buckets[i].IP = 0;
            buckets[i].id = 0;
            buckets[i].count = 0;
        }
    }

    void insert(int IP, int id){
        real[IP][id]++;
        // if(total < bucketsnum){
        //     buckets[total].IP = IP;
        //     buckets[total].id = id;
        //     buckets[total].count = 0;
        //     total++;
        // } else{
        total++;
        for(int i=0;i<bucketsnum;++i){
            int p = rand() % total;
            if(p == 0){
                buckets[i].IP = IP;
                buckets[i].id = id;
                buckets[i].count = 0;
            }
        }
            // int p = rand() % total;
            // if(p < bucketsnum){
            //     buckets[p].IP = IP;
            //     buckets[p].id = id;
            //     buckets[p].count = 0;
            // }
            // total++;
        // }
        for(int i = 0; i < bucketsnum; i++){
            if (buckets[i].IP == IP && buckets[i].id == id){
                if(i==0) printf("[[%d %d %d]]\n",IP,id,buckets[i].count);
                buckets[i].count++;
            }
        }
        
    }
   // X i,j := m ∗ (ci,j log ci,j − (ci,j − 1) log (ci,j − 1))
    double query_entropy(int IP, int m){
        vector<double> ans;
        for (int i = 0; i < bucketsnum; i++){
            if(buckets[i].IP == IP){
                // printf("{%d %d\n}",IP,buckets[i].IP);
                int c = buckets[i].count;
                // printf("[%d %d %d]\n",buckets[i].IP,buckets[i].count,c);
                if(c >= 2){
                    double x = m * (c * log(c) - (c - 1) * log(c - 1));
                    ans.push_back(x);
                } else ans.push_back(0);
            }
        }
        if(ans.size() == 0) return 0;
        //return mean ans
        // printf("{%d}\n",ans.size());
        // printf("[%d]\n",ans.size());
        // return  ans[(ans.size() - 1) / 2];
        if(ans.size() < 2){
            double sum = 0;
            for(int i = 0; i < ans.size(); i++){
                sum += ans[i];
            }
            return sum / ans.size();
        }
        printf("[%d]\n",ans.size());
        double ans0 = 0, ans1 = 0, ans2 = 0;
        for(int i=0;i<ans.size()/3;i++){
            ans0 += ans[i];
        } ans0 /= ans.size()/3;
        for(int i=ans.size()/3;i<ans.size()*2/3;i++){
            ans1 += ans[i];
        } ans1 /= (ans.size()*2/3 - ans.size()/3);
        for(int i=ans.size()*2/3;i<ans.size();i++){
            ans2 += ans[i];
        } ans2 /= (ans.size() - ans.size()*2/3);
        // return the median
        return (ans1+ans2+ans0)/3;
        // if (ans0 < ans1){
        //     if (ans1 < ans2) return ans1;
        //     else if (ans0 < ans2) return ans2;
        //     else return ans0;
        // } else{
        //     if (ans0 < ans2) return ans0;
        //     else if (ans1 < ans2) return ans2;
        //     else return ans1;
        // }
    }
};



class CMsketch{
    
    public:
    int w, d;
    vector<vector<int>> C;
    map<int,int> real;
    CMsketch(int mem){
        this->w = 3;
        this->d = mem / w / sizeof(int);
        printf("w = %d, d = %d\n", w, d);
        C.resize(w);
        for(int i = 0; i < w; i++){
            C[i].resize(d);
        }
        for(int i = 0; i < w; i++){
            for(int j = 0; j < d; j++){
                C[i][j] = 0;
            }
        }
    }
    #define hash(id, i) MurmurHash64B(&id, sizeof(int), i)
    void insert(int id){
        real[id]++;
        for(int i = 0; i < w; i++){
            int hashvalue = hash(id, i) % d;
            C[i][hashvalue]++;
        }
    }
    int query(int id){
        int min = 0x7fffffff;
        for(int i = 0; i < w; i++){
            int hashvalue = hash(id, i) % d;
            if(C[i][hashvalue] < min){
                min = C[i][hashvalue];
            }
        }
        return min;
    }
    int query_real(int id){
        return real[id];
    }
    vector<int> query_topk(int K){
        vector<pair<int, int>> topk;
        for(auto it = real.begin(); it != real.end(); it++){
            topk.push_back(make_pair(it->second, it->first));
        }
        printf("topk.size = %d\n", topk.size());
        sort(topk.begin(), topk.end());
        reverse(topk.begin(), topk.end());
        vector<int> ans;
        for(int i = 0; i < K; i++){
            ans.push_back(topk[i].second);
        }
        return ans;
    }
};


#endif