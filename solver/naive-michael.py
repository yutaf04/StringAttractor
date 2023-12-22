from itertools import product, combinations

def blocks_ranges(w):
    blocks = dict()

    for i in range(len(w)):
        for j in range(i+1, len(w)+1):
            wij=w[i:j]
            if wij in blocks:
                blocks[wij].append(set(range(i, j)))
            else:
                blocks[wij]=[set(range(i,j))]
    
    return blocks


def isAttractor(S, w):
    br=blocks_ranges(w)
    for b in br:
        for i in range (len(br[b])):
            if S & br[b][i]:
                break
        else:
            return False
    
    return True


def lsa(w): #length of smallest attractor of w
    for r in range(1, len(w)+1):
        for s in combinations(range(len(w)), r):
            if isAttractor(set(s), w):
                return r

def a(n): #only search strings starting with 0 by symmetry
    return max(lsa("0"+"".join(u)) for u in product("01", repeat=n-1))


print([a(n) for n in range(1, 15)]) 


