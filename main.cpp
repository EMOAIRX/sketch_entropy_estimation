#include "Estimator.h"
#include "trace.h"
#include<map>
using namespace std;
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
        puts("");
        double entropy = 0;
        for(auto it = MP[IP].begin();it != MP[IP].end();++it){
            double count = 1.0 * it->second;
            printf("[%.0lf]",count);
            entropy += count * log(count);
        }puts("");
        return entropy;
    }
};

GroundTruth* groundtruth[8];


int transfer_latency(double x){
    //log scale
    int p =(int)((log(x) * 100));
    printf("{%d}\n",p);
    return p;
}

int main(){
    srand(time(NULL));
    int mem = 80000;
    // here set 8 estimator and 8 cmsketch with 1000 buckets(as CMSketch(1000))
    Estimator* estimator[32];
    CMsketch* cmsketch[32];
    for (int i = 0; i < 32; i++){
        estimator[i] = new Estimator(20000);
        cmsketch[i] = new CMsketch(mem-10000);
        groundtruth[i] = new GroundTruth();
    }
    vector<pair<uint32_t, double>> data = loadCAIDA("130000_time.dat",20000);
    puts("load data success");
    for(int i = 0; i < data.size(); i++){
        int IP = data[i].first, item = transfer_latency(data[i].second);
        for(int t = 26;t < 27;t ++){
            int uip = IP >> t;
            // printf("uip = %d\n",uip);
            // puts("A");
            estimator[t] -> insert(uip , item);
            // puts("B");
            groundtruth[t] -> insert(uip,item);
            // puts("C");
            cmsketch[t] -> insert(uip);
            // puts("D");
        }
        if (i % 10000 == 0) printf("i = %d\n", i);
    }

    int level = 26;
    vector<int> top200_id;
    top200_id = cmsketch[level] -> query_topk(20);
    // get aae
    int len = top200_id.size();
    for(int i=0;i<len;++i){
        int id = top200_id[i];
        // printf("id = %d\n",id);
        int count_real = cmsketch[level] -> query_real(id);
        // printf("count_real = %d\n",count_real);
        int count_est = cmsketch[level] -> query(id);
        // printf("count_est = %d\n",count_est);
        // printf("[%d %d]\n",count_real,count_est);
        double entropy0 = estimator[level] -> query_entropy(id , count_real);
        // printf("entropy0 = %lf\n",entropy0);
        double entropy1 = estimator[level] -> query_entropy(id , count_est);
        // printf("entropy1 = %lf\n",entropy1);
        double entropy_real = groundtruth[level] -> query_entropy(id);
        printf("entropy = {%lf %lf %lf}\n",entropy0,entropy1,entropy_real);
        double aae0 = fabs(entropy0 - entropy_real) / entropy_real;
        double aae1 = fabs(entropy1 - entropy_real) / entropy_real;
        printf("%lf,%lf\n",aae0,aae1);
    }
    return 0;
}  