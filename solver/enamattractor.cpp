#include <bits/stdc++.h>
#include "../TdZdd/include/tdzdd/DdSpec.hpp"
#include "../TdZdd/include/tdzdd/DdStructure.hpp"
#include "../TdZdd/include/tdzdd/DdSpecOp.hpp"
using namespace std;

class EnumAttractor:  public tdzdd::PodArrayDdSpec <EnumAttractor, bool, 2>{
  vector<vector<bool> > coverTable;
  string T;
  int numCover;

public:
  EnumAttractor(vector<vector<bool> > coverTable, string T) : coverTable(coverTable), T(T) {
    numCover = coverTable.size();
    setArraySize(numCover);
  }

  int getRoot(bool* state) const {
    for(int i=0 ; i<numCover ; i++)
      state[i] = false;

    return T.length();
  }


  int getChild(bool* state, int level, int value) const {
    // value = 1
    if(value == 1){
      int pos = T.length() - level;
      for(int i=0 ; i<numCover ; i++){
	if(coverTable[i][pos] == true){
	  state[i] = true;
	}
      }
    }
    
    if(level == 1){
      for(int i=0 ; i<numCover ; i++){
	if(state[i] == false) return 0;
      }
      return -1;
    }

    return level - 1;
  }
};

//最小文字列アトラクタを求める
class FindMinAttractor: public tdzdd::DdEval<FindMinAttractor, int>{
  int const numCovers;

public:
  FindMinAttractor(int numCovers): numCovers(numCovers){ }

  void evalTerminal(int& size, bool one) const {
    size = one ? 0 : numCovers;
  }

  void evalNode(int& size, int level, tdzdd::DdValues<int, 2> const& values) const {
    size = min(values.get(0), values.get(1) + 1);
  }
};


class SizeRestriction:  public tdzdd::DdSpec <SizeRestriction, int, 2>{
  int const size;
  int const numItems;

public:
  SizeRestriction(int numItems, int size): numItems(numItems), size(size){ };

  int getRoot(int& state) const {
    state = 0;
    return numItems;
  }

  int getChild(int& state, int level, int value) const {
    state += value;
    if(state > size) return 0;

    if(level == 1) return -1;
    return level-1;
  }
};


vector<vector<bool> > makeCoverTable(string T){
  vector<vector<bool> > coverTable;
  for(int i = 0 ; i < T.length(); i++){
    for(int j = 1 ; j < T.length() - i; j++){ // j は文字列長
      vector<bool> cover(T.length(), false);
      string sub = T.substr(i, j);
      //cout << sub << endl;
      int pos = 0;
      while((pos = T.find(sub, pos)) != SIZE_MAX){
	for(int k = pos ; k < pos+j ; k++) cover[k] = true;
	pos++;
      }

      coverTable.push_back(cover);

      /*
      for(auto it = cover.begin() ; it != cover.end() ; it++){
	cout << *it;
      }
      cout << endl;
      */
    }
  }
    
  return coverTable;
}


int main(int argc, char** argv){
  if(argc < 2){
    cout << "Input error" << endl;
    exit(-1);
  }
  int N = atoi(argv[1]);
  string T;
  for(int i=0 ; i<N ; i++){
    char c;
    cin >> c;
    T.push_back(c);
  }
  // cin >> T;
  // T.erase(T.begin() + N, T.end());

  cout << "Input strings" << endl << "------------------------" << endl;
  cout << T << endl;
  cout << "------------------------" << endl << "End" << endl;


  vector<vector<bool> > coverTable = makeCoverTable(T);

  EnumAttractor attr(coverTable, T);
  tdzdd::DdStructure<2> dd(attr);
  dd.zddReduce();
  cout << "#Attractors: "<< dd.zddCardinality() << endl;

  std::ofstream out("test.dot");
  dd.dumpDot(out);
  out.close();


  
  for(auto it = dd.begin() ; it != dd.end() ; ++it){
    for(auto itr = (*it).begin() ; itr != (*it).end() ; ++itr){
      std::cout << T.length() - *itr + 1 << " ";
    }
    std::cout << std::endl;
  }
  

  int min_size = dd.evaluate(FindMinAttractor(coverTable.size()));
  cout << "Size of minimum attractor: " << min_size << endl;
  
  SizeRestriction sr(coverTable.size(), min_size);
  dd.zddSubset(sr);
  dd.zddReduce();
  cout << "#Minimum Attractors: "<< dd.zddCardinality() << endl;

  for(auto i : dd){
    for(auto j : i){
      std::cout << T.length() - j + 1 << " ";
    }
    std::cout << endl;
  }
  
  // for(auto it = dd.begin() ; it != dd.end() ; ++it){
  //   for(auto itr = (*it).begin() ; itr != (*it).end() ; ++itr){
  //     std::cout << T.length() - *itr + 1 << " ";
  //   }
  //   std::cout << std::endl;
  // }

  return 0;
}


