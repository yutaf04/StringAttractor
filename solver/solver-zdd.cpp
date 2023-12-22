#include <bits/stdc++.h>
#include "../tdzdd/DdSpec.hpp"
#include "../tdzdd/DdStructure.hpp"
#include "../tdzdd/DdSpecOp.hpp"
using namespace std;
vector<int> perm;//variable-order for ZDD
int branch = 0; 
map<string,int> mp, frontier, miniStritr;
vector<string> miniStr; //minimalsubstrings

//construct attractor zdd
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
        for(int i = 0; i < numCover; i++) state[i] = false;
        return T.length();
    }

    int getChild(bool* state, int level, int value) const {
        // value = 1
        if(value == 1){
            int pos = perm[T.length() - level];
            for(int i = 0; i < numCover; i++){
                if(coverTable[i][pos] == true){
                    state[i] = true;
                }
            }
        }

        //frontier branch cut
        for(int i = 0; i < numCover; i++){
            if(T.length() - level == frontier[miniStr[i]] + 1 && state[i] == false){
                branch++;
                return 0;
            }
        }
        
        if(level == 1){
            for(int i = 0; i < numCover; i++){
                if(state[i] == false) return 0;
            }
            return -1;
        }
        
        return level - 1;
    }
};

//find minimum attractor 
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

//size restriction (construct find minimum size zdd)
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
        return level - 1;
    }
};

//find index order by bread first search
vector<int> findBfsOrder(string T, vector<vector<bool> > &coverTable, vector<set<string> > &idxcover, map<string,set<int> > &strcover){
    int n = T.length();
    set<int> idxS;
    for(int i = 0; i < n; i++) idxS.insert(i);

    vector<bool> used(n, false);
    vector<int> order;
    queue<int> q;
    while(order.size() != n){
        if(q.empty()){
            int mn = n, itr = -1;
            int mnres = n;
            for(auto i: idxS){
                if(idxcover[i].size() < mn){
                    mn = idxcover[i].size();
                    itr = i;

                    int imaxsize = 0;
                    for(auto [str, S]: strcover){
                        if(S.find(i) != S.end() && S.size() > imaxsize){
                            imaxsize = S.size(); 
                        }
                    }
                    mnres = imaxsize;
                    
                }else if(idxcover[i].size() == mn){
                    int imaxsize = 0;
                    for(auto [str, S]: strcover){
                        if(S.find(i) != S.end() && S.size() > imaxsize){
                            imaxsize = S.size(); 
                        }
                    }

                    if(imaxsize < mnres){
                        mnres = imaxsize;
                        itr = i;
                    }
                }
            }

            idxS.erase(itr);
            q.push(itr);
        }else{
            auto idx = q.front(); q.pop();
            if(used[idx]) continue;
            used[idx] = true;
            order.push_back(idx);

            vector<pair<int,string> > next;
            for(auto str: idxcover[idx]) next.emplace_back((int)strcover[str].size(), str);
            if(next.empty()) continue;
            sort(next.begin(), next.end());
            for(auto [_, str]: next){
                for(auto id: strcover[str]){
                    //idの数が少ないstrのidを優先して追加していく方が良さそう(idの数が少ないstrはフロンティアから早く抜けそう)  
                    if(!used[id]){
                        q.push(id);
                    }
                }
            }
        }
    }

    return order;
}

//calculate covering substrings by index
vector<set<string> > calc_idx_cover_str(string T, vector<vector<bool> > &SubstrTable, vector<string> &ministr){
    int n = T.size(); 
    vector<set<string> > res(n+1);
    for(int i = 0; i < (int)SubstrTable.size(); i++){
        for(int j = 0; j < (int)SubstrTable[i].size(); j++){
            if(SubstrTable[i][j]){
                res[j].insert(ministr[i]);
            }
        }
    }
    return res;
}

//calcurate including idx by minimal substring
map<string, vector<int> > calc_string_including_idx(string T, vector<vector<bool> > &SubstrTable, vector<string> &ministr){
    int n = T.size();
    map<string, vector<int> > res;
    for(int i = 0; i < (int)SubstrTable.size(); i++){
        for(int j = 0; j < (int)SubstrTable[i].size(); j++){
            if(SubstrTable[i][j]){
                res[ministr[i]].push_back(j);
            }
        }
    }

    return res;
}


map<string, set<int> > calc_string_including_idx2(string T, vector<vector<bool> > &SubstrTable, vector<string> &ministr){
    int n = T.size();
    map<string, set<int> > res;
    for(int i = 0; i < (int)SubstrTable.size(); i++){
        for(int j = 0; j < (int)SubstrTable[i].size(); j++){
            if(SubstrTable[i][j]){
                res[ministr[i]].insert(j);
            }
        }
    }

    return res;
}

//sort all substring(size -> lexicographical order)
vector<pair<int,string> > sortallsubstr(string T){
    int n = T.length();
    vector<pair<int,string> > res;//{size, str}
    set<string> used;
    for(int i = 0; i < n; i++){
        for(int j = 1; j <= n - i; j++){
            string t = T.substr(i, j);
            if(used.find(t) == used.end()){
                res.push_back({j, t});
                used.insert(t);
            }
        }
    }

    sort(res.begin(), res.end());
    return res;
}

//check T is minimalsubstring or not
bool isMinimalSubstr(string T, map<string,int> &occ){
    int n = T.length();
    int num = occ[T];
    for(int i = 0; i < n; i++){
        for(int j = 1; j <= n - i; j++){
            if(i == 0 && j == n) continue;
            string t = T.substr(i, j);
            if(occ.find(t) != occ.end() && occ[t] <= num){
                return false;
            }
        }
    }

    return true;
}

//make minimul substring table
vector<vector<bool> > makeMiniStrTable(string T, map<string,int> &occ, vector<string>&miniStr){
    int n = T.size();
    vector<vector<bool> > substrTable;
    vector<pair<int,string> > strmap = sortallsubstr(T);
    for(auto [len, sub]: strmap){
        int pos = 0;//出現位置
        int poscnt = 0;//出現回数

        vector<bool> cover(n, false);
        while((pos = T.find(sub, pos)) != SIZE_MAX){
            poscnt++;
            for(int i = pos; i < pos + len; i++) cover[i] = true;
            pos++;
        }

        occ[sub] = poscnt;
        if(isMinimalSubstr(sub, occ)){
            substrTable.push_back(cover);
            miniStr.push_back(sub);
        }
    }

    return substrTable;
}

void printMiniStr(vector<string> miniStr, vector<vector<bool> > MiniStrTable, map<string,int> &miniStrItr){
    cout << "minimal substr: ";
    for(auto s: miniStr) cout << s << " ";
    cout << endl;

    int k = 0;
    for(auto cover: MiniStrTable){
        cout << miniStr[k] << " ";
        miniStrItr[miniStr[k]] = k;
        for(int j = 0; j < (int)cover.size(); j++){
            if(cover[j]){
                cout << j+1 << " ";
            }
        }
        cout << endl;
        k++;
    }
}

void printOrder(vector<int> &order){
    cout << "order: ";
    for(auto i: order) cout << i+1 << " ";
    cout << endl; 
}

int main(int argc, char** argv){
    int n;
    cin >> n;
    string T; //Input String
    for(int i = 0; i < n; i++){
        char c; cin >> c;
        T.push_back(c);
    }

    map<string,int> occ;
    vector<vector<bool> > miniStrTable = makeMiniStrTable(T, occ, miniStr);
    cout << n << endl;
    cout << "miniStrsize: " << miniStr.size() << endl;
    printMiniStr(miniStr, miniStrTable, miniStritr);
    // cout << "miniStritr" << endl;
    // for(auto [k, v]: miniStritr){
    //     cout << k << " " << v << endl;
    // }

    vector<set<string> > idxcover = calc_idx_cover_str(T, miniStrTable, miniStr);
    map<string, vector<int> > strcover = calc_string_including_idx(T, miniStrTable, miniStr);
    map<string, set<int> > strcover2 = calc_string_including_idx2(T, miniStrTable, miniStr);
    
    vector<int> order = findBfsOrder(T, miniStrTable, idxcover, strcover2);
    
    perm.resize((int)T.size());
    //for(int i = 0; i < (int)perm.size(); i++) perm[i] = i;
    for(int i = 0; i < (int)perm.size(); i++) perm[i] = order[i];
    printOrder(order);

    //calultate frontier in advance
    for(int i = 0; i < (int)perm.size(); i++){
        for(auto str: idxcover[perm[i]]){
            frontier[str] = i;
        }
    }


    EnumAttractor attr(miniStrTable, T);
    tdzdd::DdStructure<2> dd(attr);
    dd.zddReduce();
    //cout << "Num of Attractors: "<< dd.zddCardinality() << endl;
    cout << "Num of zdd node: " << dd.size() << endl;

    int min_size = dd.evaluate(FindMinAttractor(miniStrTable.size()));
    cout << "Size of minimum attractor: " << min_size << endl;

    //cout << "Size of covertable: " << substrTable.size() << endl;
    cout << "Num of all Attractors: "<< dd.zddCardinality() << endl;

    SizeRestriction sr(T.size(), min_size);
    dd.zddSubset(sr);
    dd.zddReduce();
    cout << "Num of minimum Attractors: "<< dd.zddCardinality() << endl;
    cout << "Num of zdd node: " << dd.size() << endl;

    std::ofstream out("result.dot");
    dd.dumpDot(out);
    out.close();

    // for(auto i : dd){
    //     for(auto j : i){
    //         std::cout << perm[T.length() - j] + 1 << " ";
    //     }
    //     std::cout << endl;
    // }       

    //cout << "num of branch cut =" << branch << endl;

}