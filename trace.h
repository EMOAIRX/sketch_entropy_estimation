#ifndef _TRACE_H_
#define _TRACE_H_

#include <cinttypes>
#include <vector>
#include <iostream>
#include <map>
using namespace std;

vector<pair<uint32_t, double>> loadCAIDA(const char *filename = "./CAIDA.dat",int length=10000000) {
  printf("Open %s \n", filename);
  FILE *pf = fopen(filename, "rb");
  if (!pf) {
    printf("%s not found!\n", filename);
    exit(-1);
  }
  map<uint32_t, double> last_come;
  last_come.clear();

  vector<pair<uint32_t, double>> vec;
  vec.clear();
  
  char trace[30];
  while (fread(trace, 1, 21, pf)) {
    uint32_t tkey = *(uint32_t *)(trace);
    double ttime = *(double *)(trace + 13);
    if (last_come.count(tkey))
      vec.push_back(pair<uint32_t, double>(tkey, ttime - last_come[tkey]));
    last_come[tkey] = ttime;
    if (vec.size() == length)
      break;
  }
  fclose(pf);
  return vec;
}

#endif // _TRACE_H_
