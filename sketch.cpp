#ifndef _Sketch_H_
#define _Sketch_H_
#include "MurMurHash.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <map>
#include <algorithm>
#include <cstdio>
#include "trace.h"
using namespace std;
#define bucketsnum 100

struct Estimator{
    struct Buckets{
        int id;
        int count;
    }buckets[bucketsnum];
    int total;
    void insert(int id){
        total++;
        for(int i=0;i<bucketsnum;++i){
            int p = rand() % total;
            if(p == 0){
                buckets[i].id = id;
                buckets[i].count = 0;
            }
            if(buckets[i].id == id){
                buckets[i].count++;
            }
        }
    }
    double query(){
        double sum = 0;
        for(int i=0;i<bucketsnum;++i){
            int c = buckets[i].count;
            // printf("c = %d\n",c);
            double x = (c * log(c) - (c - 1) * log(c - 1));
            if(c==1) x = 0;//printf("x = %lf\n",x);
            sum += x;
        }
        double entropy = log(total) - sum / bucketsnum;
        // printf("sum = %lf\n",sum);
        return entropy;
    }
    int querytotal(){
        return total;
    }
    double query2(int count){
        double prior_entropy = query();
        // printf("%lf",1.0*total/count);
        return prior_entropy * total / count;
    }
};

#define estimatornum 10024
#define singleD 3
struct Sketch{
    #define hash(id, i) MurmurHash64B(&id, sizeof(int), i)
    Estimator* estimator[estimatornum];
    Sketch(){
        for(int i=0;i<estimatornum;++i){
            estimator[i] = new Estimator();
        }
    }
    void insert(int IP,int id){
        for(int i=0;i<singleD;++i){
            int p = hash(IP, i) % estimatornum;
            estimator[p]->insert(id);
        }
    }
    double queryMean(int IP){
        double total = 0;
        for(int i=0;i<singleD;++i){
            int p = hash(IP, i) % estimatornum;
            total += estimator[p]->query();
        }
        return total / singleD;
    }
    double queryDirectMin(int IP){
        double Min = 0x3f3f3f3f;
        for(int i=0;i<singleD;++i){
            int p = hash(IP, i) % estimatornum;
            Min = min(Min, estimator[p]->query());
        }
        return Min;
    }
    int query_count(int IP){
        int count = 0x3f3f3f3f;
        for(int i=0;i<singleD;++i){
            int p = hash(IP, i) % estimatornum;
            count = min(count, estimator[p]->querytotal());
        }
        return count;
    }
    double queryMin(int IP, int count){
        double Min = 0x3f3f3f3f;
        for(int i=0;i<singleD;++i){
            int p = hash(IP, i) % estimatornum;
            Min = min(Min, estimator[p]->query2(count));
        }
        return Min;
    }

    #undef hash
};

// struct Elastic{
//     struct SingleBucket{
//         int IP;
//         int id;
//         int count;
//     };
//     struct Bucket{
//         SingleBucket buckets[4];
//         int vote;
//     }
//     #define HeavyBucketsNum 1000
//     Bucket buckets[HeavyBucketsNum];
//     Sketch* sketch;
//     Elastic(){
//         sketch = new Sketch();
//         for(int i=0;i<HeavyBucketsNum;++i){
//             buckets[i].vote = 0;
//             for(int j=0;j<4;++j){
//                 buckets[i].buckets[j].IP = 0;
//                 buckets[i].buckets[j].id = 0;
//                 buckets[i].buckets[j].count = 0;
//             }
//         }
//     }
//     void insert(int IP,int id){
//         int p = hash(IP, 10) % HeavyBucketsNum;
//         bool flag = true;
//         for(int i=0;i<4;++i){
//             if(buckets[p].buckets[i].IP == IP && buckets[p].buckets[i].id == id){
//                 buckets[p].buckets[i].count++;
//                 flag = false;
//             }
//         }
//         if(flag){
//             int mn = 0x3f3f3f3f;
//             int pos = -1;
//             for(int i=0;i<4;++i){
//                 if(buckets[p].buckets[i].count == 0){
//                     buckets[p].buckets[i].IP = IP;
//                     buckets[p].buckets[i].id = id;
//                     buckets[p].buckets[i].count = 1;
//                     break;
//                 }
//                 if(buckets[p].buckets[i].count < mn){
//                     mn = buckets[p].buckets[i].count;
//                     pos = i;
//                 }
//             }
//         }
//         if(flag){
//             //vote++
//             buckets[p].vote++;
//             int count = 1;
//             if (buckets[p].vote > mn * 7){
//                 //heavy
//                 buckets[p].vote = 0;
//                 swap(buckets[p].buckets[pos].IP , IP);
//                 swap(buckets[p].buckets[pos].id , id);
//                 swap(buckets[p].buckets[pos].count , count);
//             }
//             while(count--)
//                 sketch -> insert(IP,id);
//         }
//     }
//     double query(int IP){
//         int p = hash(IP, 10) % HeavyBucketsNum;
//         double total = 0;
//         for(int i=0;i<4;++i){
//             if(buckets[p].buckets[i].IP == IP){
//                 total += buckets[p].buckets[i].count;
//             }
//         }
//     }
// };


class GroundTruth{
    
public:
    map<int , map<int , int> > MP;
    void insert(int IP,int id){
        MP[IP][id]++;
    }
    double query_entropy(int IP){
        int total = 0;
        for(auto it = MP[IP].begin();it != MP[IP].end();++it){
            total += it->second;
        }
        double entropy = 0;
        for(auto it = MP[IP].begin();it != MP[IP].end();++it){
            double count = 1.0 * it->second;
            // printf("[%.0lf]",count);
            entropy += count * log(count);
        }
        entropy = log(total) - entropy / total;
        return entropy;
    }
    vector<int> query_topk(int k){
        vector<pair<int,int>> vec;
        for(auto it = MP.begin();it != MP.end();++it){
            int IP = it->first;
            int total = 0;
            for(auto it2 = MP[IP].begin();it2 != MP[IP].end();++it2){
                total += it2->second;
            }
            vec.push_back(make_pair(total,IP));
        }
        sort(vec.begin(),vec.end());
        reverse(vec.begin(),vec.end());
        vector<int> res;
        for(int i=0;i<k;++i){
            // printf("{%d}\n",`vec[i].second);
            res.push_back(vec[i].second);
        }
        return res;
    }
    int query_count(int IP){
        int total = 0;
        for(auto it = MP[IP].begin();it != MP[IP].end();++it){
            total += it->second;
        }
        return total;
    }
};


map<int,int> cnt;
int transfer_latency(uint32_t x){
    //log scale
    int p =(int)((log(x) * 1000));
    // printf("{%d}\n",p);
    cnt[p]++;
    // printf("{%d}\n",cnt.size());
    return p;
}

int main(){
    srand(time(NULL));
    Sketch *sketch = new Sketch();
    GroundTruth* groundtruth = new GroundTruth();
    vector<pair<uint32_t, uint32_t>> data = loadCAIDA("130000_time.dat",200000);
    puts("load data success");
    for(int i = 0; i < data.size(); i++){
        int IP = data[i].first, item = transfer_latency(data[i].second);
        sketch -> insert(IP , item);
        groundtruth -> insert(IP,item);
        // if (i % 100000 == 0) printf("i = %d\n", i);
    }

    vector<int> top200_id = groundtruth -> query_topk(2000);
    // get aae
    double sum_mean = 0;
    double sum_estE = 0;
    double sum_estR = 0;
    double sum_min = 0;

    int len = top200_id.size();
    for(int i=0;i<len;++i){
        int id = top200_id[i];
        int count_est = sketch -> query_count(id);
        int count_real = groundtruth -> query_count(id);
        // printf("[%d %d]\n",count_real,count_est);
        double entropy_mean = sketch -> queryMean(id); //求mean
        double entropy_estE = sketch -> queryMin(id,count_est); //给了一个期望意义上的上界?
        double entropy_estR = sketch -> queryMin(id,count_real); //给了一个期望意义上的上界?
        double entropy_min = sketch -> queryDirectMin(id); //给了一个期望意义上的上界?
        double entropy_real = groundtruth -> query_entropy(id);
        printf("[%lf %lf %lf %lf %lf]\n",entropy_mean,entropy_estE,entropy_estR,entropy_min,entropy_real);
        double aae_mean = abs(entropy_mean - entropy_real);
        double aae_estE = abs(entropy_estE - entropy_real);
        double aae_estR = abs(entropy_estR - entropy_real);
        double aae_min = abs(entropy_min - entropy_real);
        printf("%lf,%lf,%lf,%lf\n",aae_mean,aae_estE,aae_estR,aae_min);
        sum_mean += aae_mean;
        sum_estE += aae_estE;
        sum_estR += aae_estR;
        sum_min += aae_min;
    }
    printf("mean = %lf\n",sum_mean / len);
    printf("estR = %lf\n",sum_estR / len);
    printf("estE = %lf\n",sum_estE / len);
    printf("min = %lf\n",sum_min / len);

}
#endif